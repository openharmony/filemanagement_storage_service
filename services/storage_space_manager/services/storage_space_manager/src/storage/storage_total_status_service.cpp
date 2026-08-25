/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "storage/storage_total_status_service.h"

#include <cstdint>
#include <cstdlib>
#include "securec.h"
#include <sys/statvfs.h>

#include "file_ex.h"
#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"

namespace OHOS {
namespace StorageSpaceManager {

namespace {
    constexpr int64_t ONE_GB = 1000000000;           // 1GB in bytes
    constexpr int64_t ONE_TB = 1024LL * ONE_GB;      // 1024GB in bytes
    constexpr int64_t UNIT = 1000;                // Decimal unit
    constexpr int64_t THRESHOLD = 100;           // Threshold for rounding
    constexpr int64_t SECTOR_SIZE = 512;
    constexpr int32_t ROUND_HALF = 2;
    constexpr const char *BOOT_DEVICE_SIZE_PATH = "/proc/bootdevice/size";

    std::string AnonymizePath(const char *path)
    {
        if (path == nullptr) {
            return "null";
        }
        std::string pathStr(path);
        if (pathStr.empty()) {
            return "empty";
        }
        size_t pos = pathStr.rfind('/');
        if (pos == std::string::npos) {
            return "****/" + pathStr;
        }
        if (pos == 0) {
            return pathStr;
        }
        return "****" + pathStr.substr(pos);
    }

    int32_t CalcBlockProduct(int64_t blockSize, int64_t blockCount, int64_t &result, const char *tag)
    {
        if (blockSize <= 0 || blockCount < 0) {
            LOGE("%{public}s: invalid block values", tag);
            return E_IO_ERROR;
        }
        if (blockCount > 0 && blockSize > INT64_MAX / blockCount) {
            LOGE("%{public}s: overflow detected", tag);
            return E_IO_ERROR;
        }
        result = blockSize * blockCount;
        return E_OK;
    }
}

int64_t StorageTotalStatusService::GetRoundSize(int64_t size)
{
    if (size < 0) {
        return 0;
    }

    int64_t val = 1;
    int64_t multiple = UNIT;
    while (val * multiple < size) {
        uint64_t tmpVal = static_cast<uint64_t>(val);
        tmpVal <<= 1;
        val = static_cast<int64_t>(tmpVal);
        if (val > THRESHOLD && multiple < ONE_GB) {
            val = 1;
            multiple *= UNIT;
        }
    }
    return val * multiple;
}

int32_t StorageTotalStatusService::GetSizeOfPath(const char *path, int32_t type, int64_t &size)
{
    if (path == nullptr) {
        LOGE("GetSizeOfPath: path is nullptr");
        return E_INVALID_ARGUMENT;
    }
    std::string safePath = AnonymizePath(path);

    struct statvfs diskInfo;
    if (memset_s(&diskInfo, sizeof(diskInfo), 0, sizeof(diskInfo)) != EOK) {
        LOGE("GetSizeOfPath: memset_s failed for path: %{public}s", safePath.c_str());
        return E_IO_ERROR;
    }
    int ret = statvfs(path, &diskInfo);
    if (ret != 0) {
        LOGE("Failed to statvfs for path: %{public}s, errno: %{public}d", safePath.c_str(), errno);
        return E_STATVFS_FAILED;
    }

    std::string typeStr;
    int32_t err;
    if (type == static_cast<int32_t>(StorageStatType::TOTAL)) {
        err = CalcBlockProduct(static_cast<int64_t>(diskInfo.f_bsize),
            static_cast<int64_t>(diskInfo.f_blocks), size, "GetSizeOfPath: total space");
        typeStr = "total space";
    } else if (type == static_cast<int32_t>(StorageStatType::FREE)) {
        err = CalcBlockProduct(static_cast<int64_t>(diskInfo.f_frsize),
            static_cast<int64_t>(diskInfo.f_bavail), size, "GetSizeOfPath: free space");
        typeStr = "free space";
    } else {
        int64_t usedBlocks = static_cast<int64_t>(diskInfo.f_blocks) -
                             static_cast<int64_t>(diskInfo.f_bfree);
        err = CalcBlockProduct(static_cast<int64_t>(diskInfo.f_bsize),
            usedBlocks, size, "GetSizeOfPath: used space");
        typeStr = "used space";
    }
    if (err != E_OK) {
        return err;
    }

    LOGI("GetSizeOfPath: path=%{public}s, type=%{public}s, size=%{public}lld",
         safePath.c_str(), typeStr.c_str(), static_cast<long long>(size));
    return E_OK;
}

int32_t StorageTotalStatusService::GetTotalSize(int64_t &totalSize)
{
    int64_t dataSize = 0;
    int32_t ret = GetSizeOfPath(PATH_DATA, static_cast<int32_t>(StorageStatType::TOTAL), dataSize);
    if (ret != E_OK) {
        LOGE("Failed to get data partition size");
        return ret;
    }

    int64_t rootSize = 0;
    ret = GetSizeOfPath(PATH_ROOT, static_cast<int32_t>(StorageStatType::TOTAL), rootSize);
    if (ret != E_OK) {
        LOGE("Failed to get root partition size");
        return ret;
    }

    int64_t rawSize = dataSize + rootSize;
    if (rawSize > ONE_TB) {
        std::string content;
        if (OHOS::LoadStringFromFile(BOOT_DEVICE_SIZE_PATH, content) && !content.empty()) {
            int64_t sectorCount = atoll(content.c_str());
            if (sectorCount <= 0) {
                LOGE("GetTotalSize: atoll failed for %{public}s", BOOT_DEVICE_SIZE_PATH);
                totalSize = GetRoundSize(rawSize);
            } else if (sectorCount <= INT64_MAX / SECTOR_SIZE) {
                int64_t deviceSize = sectorCount * SECTOR_SIZE;
                totalSize = (deviceSize + ONE_GB / ROUND_HALF) / ONE_GB * ONE_GB;
            } else {
                totalSize = GetRoundSize(rawSize);
            }
        } else {
            LOGE("GetTotalSize: read boot device size failed for %{public}s", BOOT_DEVICE_SIZE_PATH);
            totalSize = GetRoundSize(rawSize);
        }
    } else {
        totalSize = GetRoundSize(rawSize);
    }
    LOGI("GetTotalSize: totalSize=%{public}lld, rawSize=%{public}lld, "
         "(/data=%{public}lld, /=%{public}lld)",
         static_cast<long long>(totalSize), static_cast<long long>(rawSize),
         static_cast<long long>(dataSize), static_cast<long long>(rootSize));
    return E_OK;
}

int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    int32_t ret = GetSizeOfPath(PATH_DATA, static_cast<int32_t>(StorageStatType::FREE), freeSize);
    if (ret != E_OK) {
        LOGE("Failed to get free size");
        return ret;
    }

    LOGI("GetFreeSize: freeSize=%{public}lld", static_cast<long long>(freeSize));
    return E_OK;
}

int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    int64_t roundSize = 0;
    int32_t ret = GetTotalSize(roundSize);
    if (ret != E_OK) {
        LOGE("GetTotalSize failed for system size calculation");
        return ret;
    }

    int64_t dataSize = 0;
    ret = GetSizeOfPath(PATH_DATA, static_cast<int32_t>(StorageStatType::TOTAL), dataSize);
    if (ret != E_OK) {
        LOGE("Failed to get data partition size");
        return ret;
    }

    if (dataSize > roundSize) {
        LOGE("GetSystemSize: dataSize=%{public}lld > roundSize=%{public}lld",
             static_cast<long long>(dataSize), static_cast<long long>(roundSize));
        systemSize = 0;
        return E_OK;
    }
    systemSize = roundSize - dataSize;
    LOGI("GetSystemSize: roundSize=%{public}lld, dataSize=%{public}lld, systemSize=%{public}lld",
         static_cast<long long>(roundSize),
         static_cast<long long>(dataSize),
         static_cast<long long>(systemSize));
    return E_OK;
}

int32_t StorageTotalStatusService::GetInodeOfPath(const char *path, int32_t type, int64_t &inodeCnt)
{
    if (path == nullptr) {
        LOGE("GetInodeOfPath: path is nullptr");
        return E_INVALID_ARGUMENT;
    }
    std::string safePath = AnonymizePath(path);

    struct statvfs diskInfo;
    if (memset_s(&diskInfo, sizeof(diskInfo), 0, sizeof(diskInfo)) != EOK) {
        LOGE("GetInodeOfPath: memset_s failed for path: %{public}s", safePath.c_str());
        return E_IO_ERROR;
    }
    int ret = statvfs(path, &diskInfo);
    if (ret != 0) {
        LOGE("Failed to statvfs for path: %{public}s, errno: %{public}d", safePath.c_str(), errno);
        return E_STATVFS_FAILED;
    }

    std::string typeStr;
    if (type == static_cast<int32_t>(StorageStatType::TOTAL)) {
        inodeCnt = static_cast<int64_t>(diskInfo.f_files);
        typeStr = "total inodes";
    } else if (type == static_cast<int32_t>(StorageStatType::FREE)) {
        inodeCnt = static_cast<int64_t>(diskInfo.f_ffree);
        typeStr = "free inodes";
    } else {
        int64_t totalFiles = static_cast<int64_t>(diskInfo.f_files);
        int64_t freeFiles = static_cast<int64_t>(diskInfo.f_ffree);
        if (totalFiles < 0 || freeFiles < 0 || totalFiles < freeFiles) {
            LOGE("GetInodeOfPath: invalid inode values");
            return E_IO_ERROR;
        }
        inodeCnt = totalFiles - freeFiles;
        typeStr = "used inodes";
    }

    LOGI("GetInodeOfPath: path=%{public}s, type=%{public}s, inodeCnt=%{public}lld",
         safePath.c_str(), typeStr.c_str(), static_cast<long long>(inodeCnt));
    return E_OK;
}

int32_t StorageTotalStatusService::GetTotalInodes(int64_t &totalInodes)
{
    int32_t ret = GetInodeOfPath(PATH_DATA, static_cast<int32_t>(StorageStatType::TOTAL), totalInodes);
    if (ret != E_OK) {
        LOGE("Failed to get total inodes");
        return ret;
    }

    LOGI("GetTotalInodes: totalInodes=%{public}lld", static_cast<long long>(totalInodes));
    return E_OK;
}

int32_t StorageTotalStatusService::GetFreeInodes(int64_t &freeInodes)
{
    int32_t ret = GetInodeOfPath(PATH_DATA, static_cast<int32_t>(StorageStatType::FREE), freeInodes);
    if (ret != E_OK) {
        LOGE("Failed to get free inodes");
        return ret;
    }

    LOGI("GetFreeInodes: freeInodes=%{public}lld", static_cast<long long>(freeInodes));
    return E_OK;
}

} // namespace StorageSpaceManager
} // namespace OHOS
