/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "volume/notification.h"

#include "storage_service_errno.h"
#include "storage/volume_storage_status_service.h"

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "storage_service_log.h"
#include "string_wrapper.h"
#include "volume_core.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace StorageManager {
Notification::Notification() {}
Notification::~Notification() {}

namespace {
void SetMountedEventParams(AAFwk::WantParams &wantParams, std::shared_ptr<VolumeExternal> volume)
{
    if (volume == nullptr) {
        LOGE("SetMountedEventParams: volume is nullptr");
        return;
    }
    wantParams.SetParam("path", AAFwk::String::Box(volume->GetPath()));
    wantParams.SetParam("fsType", AAFwk::Integer::Box(volume->GetFsType()));

    // Get free size using VolumeStorageStatusService
    int64_t freeSize = 0;
    auto &statusService = VolumeStorageStatusService::GetInstance();
    int32_t ret = statusService.GetFreeSizeOfVolume(volume->GetUuid(), freeSize);
    if (ret == E_OK) {
        wantParams.SetParam("freeSize", AAFwk::Long::Box(freeSize));
        LOGI("Volume mounted: id=%{public}s, freeSize=%{public}lld",
            volume->GetId().c_str(), (long long)freeSize);
    } else {
        LOGW("Volume mounted: id=%{public}s, failed to get freeSize, ret=%{public}d",
            volume->GetId().c_str(), ret);
    }
}

void SetUnmountedEventParams(AAFwk::WantParams &wantParams, std::shared_ptr<VolumeExternal> volume)
{
    if (volume == nullptr) {
        LOGE("SetUnmountedEventParams: volume is nullptr");
        return;
    }
    wantParams.SetParam("fsType", AAFwk::Integer::Box(volume->GetFsType()));

    // MTP/PTP devices do not support statvfs, so freeSize is not applicable
    if (volume->GetFsType() == FsType::MTP || volume->GetFsType() == FsType::PTP) {
        LOGI("Volume unmounted: id=%{public}s, fsType=%{public}d (MTP/PTP device, freeSize not set)",
            volume->GetId().c_str(), volume->GetFsType());
    } else {
        // Use the freeSize saved before unmount for normal block devices
        int64_t freeSize = volume->GetFreeSize();
        wantParams.SetParam("freeSize", AAFwk::Long::Box(freeSize));
        LOGI("Volume unmounted: id=%{public}s, fsType=%{public}d, freeSize=%{public}lld",
            volume->GetId().c_str(), volume->GetFsType(), (long long)freeSize);
    }
}

struct VolumeStateInfo {
    VolumeState state;
    const char* logMessage;
    std::string eventAction;
};

const VolumeStateInfo STATE_INFOS[] = {
    {VolumeState::REMOVED, "VOLUME_REMOVED", EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_REMOVED},
    {VolumeState::UNMOUNTED, "VOLUME_UNMOUNTED", EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_UNMOUNTED},
    {VolumeState::MOUNTED, "VOLUME_MOUNTED", EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_MOUNTED},
    {VolumeState::BAD_REMOVAL, "VOLUME_BAD_REMOVAL", EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_BAD_REMOVAL},
    {VolumeState::EJECTING, "VOLUME_EJECT", EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_EJECT},
    {VolumeState::DAMAGED, "DeskDamaged", EventFwk::CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE},
    {VolumeState::DAMAGED_MOUNTED, "DeskDamagedMounted", EventFwk::CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE},
    {VolumeState::ENCRYPTING, "DeskEncrypting", EventFwk::CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL},
    {VolumeState::ENCRYPTED_AND_LOCKED, "DeskEncryptedAndLocked",
        EventFwk::CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL},
    {VolumeState::ENCRYPTED_AND_UNLOCKED, "DeskEncryptedAndUnLocked",
        EventFwk::CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL},
    {VolumeState::DECRYPTING, "DeskDecrypting", EventFwk::CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL}
};
} // anonymous namespace

void Notification::NotifyVolumeChange(VolumeState notifyCode, std::shared_ptr<VolumeExternal> volume)
{
    if (volume == nullptr) {
        LOGE("Notification::NotifyVolumeChange volume is nullptr");
        return;
    }

    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    wantParams.SetParam("id", AAFwk::String::Box(volume->GetId()));
    wantParams.SetParam("diskId", AAFwk::String::Box(volume->GetDiskId()));
    wantParams.SetParam("fsUuid", AAFwk::String::Box(volume->GetUuid()));
    wantParams.SetParam("flags", AAFwk::Integer::Box(volume->GetFlags()));
    wantParams.SetParam("volumeState", AAFwk::Integer::Box(notifyCode));

    for (const auto& info : STATE_INFOS) {
        if (info.state == notifyCode) {
            LOGI("notifycode: %{public}s, id:%{public}s", info.logMessage, volume->GetId().c_str());
            want.SetAction(info.eventAction);
            break;
        }
    }

    if (notifyCode == VolumeState::MOUNTED) {
        SetMountedEventParams(wantParams, volume);
    }

    if (notifyCode == VolumeState::UNMOUNTED) {
        SetUnmountedEventParams(wantParams, volume);
    }

    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}
}
} // namespace OHOS
