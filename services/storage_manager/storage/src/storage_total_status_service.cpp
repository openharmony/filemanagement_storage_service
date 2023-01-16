/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include <cstdlib>
#include <cstring>
#include <mntent.h>
#include <singleton.h>
#include <sys/statvfs.h>
#include <unordered_set>

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
constexpr int32_t DEV_BLOCK_DIR_LEN = 10;
constexpr int32_t MNT_DIR_LEN = 4;
constexpr int32_t DATA_DIR_LEN = 5;

StorageTotalStatusService::StorageTotalStatusService() {}
StorageTotalStatusService::~StorageTotalStatusService() {}

int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    struct mntent *mntent = nullptr;
    std::unordered_set<std::string> mntSet;
    std::unique_ptr<FILE, decltype(&endmntent)> mntFile(setmntent("/proc/mounts", "re"), endmntent);
    if (mntFile.get() == nullptr) {
        LOGE("enter setmntent is error");
        return E_ERR;
    }
    while ((mntent = getmntent(mntFile.get())) != nullptr) {
        if (!mntent) {
            LOGE("enter getmntent is error");
            break;
        }
        char *strName = mntent->mnt_fsname;
        char *strDir = mntent->mnt_dir;
        if (!mntSet.insert(strName).second) {
            continue;
        }
        if (strncmp(strName, PATH_DEV_BLOCK, DEV_BLOCK_DIR_LEN) == 0 && strncmp(strDir, PATH_MNT, MNT_DIR_LEN) != 0 &&
            strncmp(strDir, PATH_DATA, DATA_DIR_LEN) != 0) {
            LOGI("mntent::strName is %{public}s, mntent::strDir is %{public}s", strName, strDir);
            int64_t blkSize;
            int32_t err = GetSizeOfPath(strDir, SizeType::USED, blkSize);
            if (err != E_OK) {
                LOGE("get path %s size err", strDir);
            }
            systemSize += blkSize;
        }
    }
    return E_OK;
}

int32_t StorageTotalStatusService::GetTotalSize(int64_t &totalSize)
{
    return GetSizeOfPath(PATH_DATA, SizeType::TOTAL, totalSize);
}

int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    return GetSizeOfPath(PATH_DATA, SizeType::FREE, freeSize);
}

int32_t StorageTotalStatusService::GetSizeOfPath(const char *path, int32_t type, int64_t &size)
{
    struct statvfs diskInfo;
    int ret = statvfs(path, &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    if (type == SizeType::TOTAL) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    } else if (type == SizeType::FREE) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    } else {
        size = (int64_t)diskInfo.f_bsize * ((int64_t)diskInfo.f_blocks - (int64_t)diskInfo.f_bfree);
    }
    return E_OK;
}
} // StorageManager
} // OHOS
