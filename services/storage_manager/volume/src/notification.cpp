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

#include "volume/notification.h"

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "int_wrapper.h"
#include "storage_service_log.h"
#include "string_wrapper.h"
#include "volume_core.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace StorageManager {
Notification::Notification() {}
Notification::~Notification() {}

void Notification::NotifyVolumeChange(VolumeState notifyCode, std::shared_ptr<VolumeExternal> volume)
{
    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    if (volume == nullptr) {
        LOGE("Notification::NotifyVolumeChange volume is nullptr");
        return;
    }
    wantParams.SetParam("id", AAFwk::String::Box(volume->GetId()));
    wantParams.SetParam("diskId", AAFwk::String::Box(volume->GetDiskId()));
    wantParams.SetParam("fsUuid", AAFwk::String::Box(volume->GetUuid()));
    wantParams.SetParam("flags", AAFwk::Integer::Box(volume->GetFlags()));
    switch (notifyCode) {
        case VolumeState::REMOVED:
            LOGI("notifycode: VOLUME_REMOVED");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_REMOVED);
            break;
        case VolumeState::UNMOUNTED:
            LOGI("notifycode: VOLUME_UNMOUNTED");
            wantParams.SetParam("volumeState", AAFwk::Integer::Box(UNMOUNTED));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_UNMOUNTED);
            break;
        case VolumeState::MOUNTED:
            LOGI("notifycode: VOLUME_MOUNTED");
            wantParams.SetParam("volumeState", AAFwk::Integer::Box(MOUNTED));
            wantParams.SetParam("path", AAFwk::String::Box(volume->GetPath()));
            wantParams.SetParam("fsType", AAFwk::Integer::Box(volume->GetFsType()));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_MOUNTED);
            break;
        case VolumeState::BAD_REMOVAL:
            LOGI("notifycode: VOLUME_BAD_REMOVAL");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_BAD_REMOVAL);
            break;
        case VolumeState::EJECTING:
            LOGI("notifycode: VOLUME_EJECT");
            wantParams.SetParam("volumeState", AAFwk::Integer::Box(EJECTING));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_VOLUME_EJECT);
            break;
        case VolumeState::DAMAGED:
        case VolumeState::DAMAGED_MOUNTED:
            LOGI("notifycode: DeskDamaged, id:%{public}s", volume->GetId().c_str());
            wantParams.SetParam("volumeState", AAFwk::Integer::Box(DAMAGED));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTABLE);
            break;
        default:
            return;
    }
    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}
}
} // namespace OHOS
