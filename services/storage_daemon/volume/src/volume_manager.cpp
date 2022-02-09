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

#include "volume/volume_manager.h"
#include <cstdlib>
#include <sys/sysmacros.h>
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/string_utils.h"
#include "volume/external_volume_info.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
VolumeManager* VolumeManager::instance_ = nullptr;

VolumeManager* VolumeManager::Instance()
{
    if (instance_ == nullptr) {
        instance_ = new VolumeManager();
    }

    return instance_;
}

std::shared_ptr<VolumeInfo> VolumeManager::GetVolume(const std::string volId)
{
    for (std::list<std::shared_ptr<VolumeInfo>>::iterator it = volumes_.begin();
         it != volumes_.end(); ++it) {
        if ((*it)->GetVolumeId() == volId) {
            return (*it);
        }
    }
    return nullptr;
}

std::string VolumeManager::CreateVolume(const std::string diskId, dev_t device)
{
    std::string volId = StringPrintf("vol-%u-%u", major(device), minor(device));

    LOGI("create volume %{public}s.", volId.c_str());

    std::shared_ptr<VolumeInfo> tmp = GetVolume(volId);
    if (tmp != nullptr) {
        LOGE("volume %{public}s exist.", volId.c_str());
        return "";
    }

    auto info = std::make_shared<ExternalVolumeInfo>();
    int32_t ret = info->Create(volId, diskId, device);
    if (ret) {
        return "";
    }

    volumes_.push_back(info);
    return volId;
}

int32_t VolumeManager::DestroyVolume(const std::string volId)
{
    LOGI("destroy volume %{public}s.", volId.c_str());

    std::shared_ptr<VolumeInfo> destroyNode = GetVolume(volId);

    if (destroyNode == nullptr) {
        LOGE("the volume %{public}s does not exist", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t ret = destroyNode->Destroy();
    if (ret)
        return ret;
    volumes_.remove(destroyNode);
    destroyNode.reset();
    return E_OK;
}

int32_t VolumeManager::Check(const std::string volId)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("the volume %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Check();
    if (err != E_OK) {
        LOGE("the volume %{public}s check failed.", volId.c_str());
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::Mount(const std::string volId, uint32_t flags)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("the volume %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Mount(flags);
    if (err != E_OK) {
        LOGE("the volume %{public}s mount failed.", volId.c_str());
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::UMount(const std::string volId)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("the volume %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->UMount();
    if (err != E_OK) {
        LOGE("the volume %{public}s mount failed.", volId.c_str());
        return err;
    }
    return E_OK;
}

int32_t VolumeManager::Format(const std::string volId, const std::string fsType)
{
    std::shared_ptr<VolumeInfo> info = GetVolume(volId);
    if (info == nullptr) {
        LOGE("the volume %{public}s does not exist.", volId.c_str());
        return E_NON_EXIST;
    }

    int32_t err = info->Format(fsType);
    if (err != E_OK) {
        LOGE("the volume %{public}s format failed.", volId.c_str());
        return err;
    }
    return E_OK;
}
} // StorageDaemon
} // OHOS
