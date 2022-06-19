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

#include "volume/volume_info.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
int32_t VolumeInfo::Create(const std::string volId, const std::string diskId, dev_t device)
{
    id_ = volId;
    diskId_ = diskId;
    type_ = EXTERNAL;
    mountState_ = UNMOUNTED;
    mountFlags_ = 0;
    userIdOwner_ = 0;

    int32_t err = DoCreate(device);
    if (err) {
        return err;
    }
    return E_OK;
}

std::string VolumeInfo::GetVolumeId()
{
    return id_;
}

int32_t VolumeInfo::GetVolumeType()
{
    return type_;
}

std::string VolumeInfo::GetDiskId()
{
    return diskId_;
}

int32_t VolumeInfo::GetState()
{
    return mountState_;
}

int32_t VolumeInfo::Destroy()
{
    VolumeState state = REMOVED;
    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        return E_OK;
    }
    if (mountState_ != UNMOUNTED) {
        // force umount
        UMount(true);
        state = BADREMOVABLE;
    }

    int32_t err = DoDestroy();
    if (err) {
        return err;
    }
    mountState_ = state;
    return E_OK;
}

int32_t VolumeInfo::Mount(uint32_t flags)
{
    int32_t err = 0;

    if (mountState_ == MOUNTED) {
        return E_OK;
    }
    if (mountState_ != CHECKING) {
        LOGE("please check volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    mountFlags_ = flags;
    err = DoMount(mountFlags_);
    if (err) {
        mountState_ = UNMOUNTED;
        return err;
    }

    mountState_ = MOUNTED;
    return E_OK;
}

int32_t VolumeInfo::UMount(bool force)
{
    int32_t err = 0;

    if (mountState_ == REMOVED || mountState_ == BADREMOVABLE) {
        LOGE("the volume %{public}s is in REMOVED state", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    if (mountState_ == UNMOUNTED) {
        return E_OK;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
        return E_OK;
    }

    if (mountState_ == EJECTING && !force) {
        return E_WAIT;
    }

    mountState_ = EJECTING;

    err = DoUMount(force);
    if (!force && err) {
        mountState_ = MOUNTED;
        return err;
    }

    mountState_ = UNMOUNTED;
    return E_OK;
}

int32_t VolumeInfo::Check()
{
    if (mountState_ != UNMOUNTED) {
        LOGE("the volume %{public}s is not in UNMOUNT state", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    if (mountState_ == CHECKING) {
        mountState_ = UNMOUNTED;
    }

    int32_t err = DoCheck();
    if (err) {
        return err;
    }
    mountState_ = CHECKING;
    return E_OK;
}

int32_t VolumeInfo::Format(std::string type)
{
    if (mountState_ != UNMOUNTED) {
        LOGE("Please unmount the volume %{public}s first", GetVolumeId().c_str());
        return E_VOL_STATE;
    }

    int32_t err = DoFormat(type);
    if (err) {
        return err;
    }
    return E_OK;
}
} // StorageDaemon
} // OHOS
