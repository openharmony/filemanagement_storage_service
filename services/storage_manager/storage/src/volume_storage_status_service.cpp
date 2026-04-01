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
#include "storage_daemon_communication/storage_daemon_communication.h"
#include <cinttypes>
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

bool VolumeStorageStatusService::IsOddDevice(const std::string& volumeUuid)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    auto volExternalInfo = VolumeManagerService::GetInstance().GetVolumeByUuid(volumeUuid);
    if (volExternalInfo == nullptr) {
        LOGE("IsOddDevice: volExternalInfo is null");
        return false;
    }
    std::string fsType = volExternalInfo->GetFsTypeString();
    if (fsType == "udf" || fsType == "iso9660") {
        return true;
    }
    return false;
#else
    return false;
#endif
}

int32_t VolumeStorageStatusService::GetOddSize(const std::string& volumeUuid, int64_t &totalSize, int64_t &freeSize)
{
#ifdef EXTERNAL_STORAGE_MANAGER
    auto volExternalInfo = VolumeManagerService::GetInstance().GetVolumeByUuid(volumeUuid);
    if (volExternalInfo == nullptr) {
        LOGE("GetOddSize: volExternalInfo is null");
        return E_ERR;
    }
    auto volumeId = volExternalInfo->GetId();
    LOGI("get odd size : volumeid is %{public}s", volumeId.c_str());
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        return E_ERR;
    }
    int ret = sdCommunication->GetOddCapacity(volumeId, totalSize, freeSize);
    return ret;
#else
    return E_ERR;
#endif
}

int32_t VolumeStorageStatusService::GetFreeSizeOfVolume(const std::string& volumeUuid, int64_t &freeSize)
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
    if (IsOddDevice(volumeUuid)) {
        int64_t totalSize = 0;
        int64_t startTotalSize =  (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
        int64_t startFreeSize = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
        ret = GetOddSize(volumeUuid, totalSize, freeSize);
        LOGI("totalSize is %{public}" PRIu64 " freeSize is %{public}" PRIu64 ", ret val is %{public}d",
            totalSize, freeSize, ret);
        if (freeSize != 0) {
            return ret;
        }
        if (startFreeSize == 0) {
            freeSize = totalSize - startTotalSize;
            return E_OK;
        }
    }
    freeSize = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    return E_OK;
}

int32_t VolumeStorageStatusService::GetTotalSizeOfVolume(const std::string& volumeUuid, int64_t &totalSize)
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
    if (IsOddDevice(volumeUuid)) {
        int64_t freeSize = 0;
        GetOddSize(volumeUuid, totalSize, freeSize);
        return E_OK;
    }
    totalSize =  (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    return E_OK;
}
} // StorageManager
} // OHOS
