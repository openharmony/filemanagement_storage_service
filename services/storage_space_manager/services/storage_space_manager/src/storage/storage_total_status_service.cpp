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

#include <cmath>
#include <sys/statvfs.h>

#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"

namespace OHOS {
namespace StorageSpaceManager {

namespace {
    constexpr int64_t ONE_GB = 1024 * 1024 * 1024;   // 1GB in bytes
    constexpr int64_t UNIT = 1024;               // Binary unit
    constexpr int64_t THRESHOLD = 100;           // Threshold for rounding
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
    struct statvfs diskInfo;
    int ret = statvfs(path, &diskInfo);
    if (ret != 0) {
        LOGE("Failed to statvfs for path: %{public}s, errno: %{public}d", path, errno);
        return E_STATVFS_FAILED;
    }

    std::string typeStr;
    if (type == static_cast<int32_t>(StorageStatType::TOTAL)) {
        size = static_cast<int64_t>(diskInfo.f_bsize) * static_cast<int64_t>(diskInfo.f_blocks);
        typeStr = "total space";
    } else if (type == static_cast<int32_t>(StorageStatType::FREE)) {
        size = static_cast<int64_t>(diskInfo.f_frsize) * static_cast<int64_t>(diskInfo.f_bavail);
        typeStr = "free space";
    } else {
        size = static_cast<int64_t>(diskInfo.f_bsize) *
               (static_cast<int64_t>(diskInfo.f_blocks) - static_cast<int64_t>(diskInfo.f_bfree));
        typeStr = "used space";
    }

    LOGI("GetSizeOfPath: path=%{public}s, type=%{public}s, size=%{public}lld",
         path, typeStr.c_str(), static_cast<long long>(size));
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

    totalSize = GetRoundSize(dataSize + rootSize);
    LOGI("GetTotalSize: totalSize=%{public}lld (/data=%{public}lld, /=%{public}lld)",
         static_cast<long long>(totalSize),
         static_cast<long long>(dataSize),
         static_cast<long long>(rootSize));
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

    systemSize = roundSize - dataSize;
    LOGI("GetSystemSize: roundSize=%{public}lld, dataSize=%{public}lld, systemSize=%{public}lld",
         static_cast<long long>(roundSize),
         static_cast<long long>(dataSize),
         static_cast<long long>(systemSize));
    return E_OK;
}

int32_t StorageTotalStatusService::GetInodeOfPath(const char *path, int32_t type, int64_t &inodeCnt)
{
    struct statvfs diskInfo;
    int ret = statvfs(path, &diskInfo);
    if (ret != 0) {
        LOGE("Failed to statvfs for path: %{public}s, errno: %{public}d", path, errno);
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
        inodeCnt = static_cast<int64_t>(diskInfo.f_files) - static_cast<int64_t>(diskInfo.f_ffree);
        typeStr = "used inodes";
    }

    LOGI("GetInodeOfPath: path=%{public}s, type=%{public}s, inodeCnt=%{public}lld",
         path, typeStr.c_str(), static_cast<long long>(inodeCnt));
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
