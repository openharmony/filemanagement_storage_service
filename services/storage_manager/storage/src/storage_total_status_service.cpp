/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <sys/statvfs.h>
#include <singleton.h>
#include "volume/volume_manager_service.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage/storage_total_status_service.h"

using namespace std;

namespace OHOS {
namespace StorageManager {
StorageTotalStatusService::StorageTotalStatusService() {}
StorageTotalStatusService::~StorageTotalStatusService() {}

std::string StorageTotalStatusService::GetVolumePath(std::string volumeUuid)
{
    auto volumePtr = DelayedSingleton<VolumeManagerService>::GetInstance()->GetVolumeByUuid(volumeUuid);
    if (volumePtr == nullptr) {
        LOGI("StorageTotalStatusService::GetVolumePath fail.");
        return "";
    }
    return volumePtr->GetPath();
}

int64_t StorageTotalStatusService::GetFreeSizeOfVolume(string volumeUuid)
{
    string path = GetVolumePath(volumeUuid);
    LOGI("StorageTotalStatusService::GetFreeSizeOfVolume path is %{public}s", path.c_str());
    if (path == "") {
        return E_ERR;
    }
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
            return E_ERR;
    }
    int64_t freeSize = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    return freeSize;
}

int64_t StorageTotalStatusService::GetTotalSizeOfVolume(string volumeUuid)
{
    string path = GetVolumePath(volumeUuid);
    if (path == "") {
        return E_ERR;
    }
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
            return E_ERR;
    }
    int64_t totalSize =  (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    return totalSize;
}

int64_t StorageTotalStatusService::GetSystemSize()
{
    return E_OK;
}

int64_t StorageTotalStatusService::GetTotalSize()
{
    string path = "/data";
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    int64_t totalSize =  (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    return totalSize;
}

int64_t StorageTotalStatusService::GetFreeSize()
{
    string path = "/data";
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    int64_t freeSize = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    return freeSize;
}
} // StorageManager
} // OHOS
