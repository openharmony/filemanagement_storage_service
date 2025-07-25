/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "storage/volume_storage_status_service.h"

#include <sys/statvfs.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_utils.h"
#ifdef EXTERNAL_STORAGE_MANAGER
#include "volume/volume_manager_service.h"
#endif

namespace OHOS {
namespace StorageManager {
VolumeStorageStatusService::VolumeStorageStatusService() {}
VolumeStorageStatusService::~VolumeStorageStatusService() {}


std::string VolumeStorageStatusService::GetVolumePath(std::string volumeUuid)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    auto volumePtr = VolumeManagerService::GetInstance().GetVolumeByUuid(volumeUuid);
    if (volumePtr == nullptr) {
        LOGE("VolumeStorageStatusService::GetVolumePath fail.");
        return "";
    }
    return volumePtr->GetPath();
#else
    return "";
#endif
}

int32_t VolumeStorageStatusService::GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize)
{
    std::string path = GetVolumePath(volumeUuid);
    LOGI("VolumeStorageStatusService::GetFreeSizeOfVolume path is %{public}s", GetAnonyString(path).c_str());
    if (path == "") {
        return E_NON_EXIST;
    }
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
        return E_STATVFS;
    }
    freeSize = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    return E_OK;
}

int32_t VolumeStorageStatusService::GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize)
{
    std::string path = GetVolumePath(volumeUuid);
    if (path == "") {
        return E_NON_EXIST;
    }
    struct statvfs diskInfo;
    int ret = statvfs(path.c_str(), &diskInfo);
    if (ret != E_OK) {
        return E_STATVFS;
    }
    totalSize =  (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    return E_OK;
}
} // StorageManager
} // OHOS
