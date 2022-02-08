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
#include "want.h"
#include "want_params.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "string_wrapper.h"
#include "storage_service_log.h"
namespace OHOS {
namespace StorageManager {
Notification::Notification() {}
Notification::~Notification() {}

void Notification::NotifyVolumeChange(int32_t notifyCode, std::string id, std::string diskId,
    std::string fsUuid, std::string path)
{
    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    wantParams.SetParam("id", AAFwk::String::Box(id));
    wantParams.SetParam("diskId", AAFwk::String::Box(diskId));
    switch (notifyCode) {
        case VOLUME_REMOVED:
            LOGI("notifycode: VOLUME_REMOVED");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_REMOVED);
            break;
        case VOLUME_UNMOUNTED:
            LOGI("notifycode: VOLUME_UNMOUNTED");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_UNMOUNTED);
            break;
        case VOLUME_MOUNTED:
            LOGI("notifycode: VOLUME_MOUNTED");
            wantParams.SetParam("fsUuid", AAFwk::String::Box(fsUuid));
            wantParams.SetParam("path", AAFwk::String::Box(path));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_MOUNTED);
            break;
        case VOLUME_BAD_REMOVAL:
            LOGI("notifycode: VOLUME_BAD_REMOVAL");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_BAD_REMOVAL);
            break;
        case VOLUME_EJECT:
            LOGI("notifycode: VOLUME_EJECT");
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_EJECT);
            break;
        default: {
            break;
        }
    }
    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}
}
} // namespace OHOS
