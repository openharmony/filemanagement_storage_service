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

#include "disk/disk_notification.h"

#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "int_wrapper.h"
#include "storage_service_log.h"
#include "string_wrapper.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace StorageManager {
DiskNotification::DiskNotification() {}
DiskNotification::~DiskNotification() {}

void DiskNotification::NotifyDiskChange(StorageDaemon::DiskInfo::DiskState notifyCode, std::shared_ptr<Disk> &disk)
{
    AAFwk::Want want;
    AAFwk::WantParams wantParams;
    if (disk == nullptr) {
        LOGE("DiskManagerService::NotifyDiskChange volume is nullptr");
        return;
    }
    wantParams.SetParam("diskId", AAFwk::String::Box(disk->GetDiskId()));
    wantParams.SetParam("sizeBytes", AAFwk::Integer::Box(disk->GetSizeBytes()));
    wantParams.SetParam("sysPath", AAFwk::String::Box(disk->GetSysPath()));
    wantParams.SetParam("vendor", AAFwk::String::Box(disk->GetVendor()));
    wantParams.SetParam("flag", AAFwk::Integer::Box(disk->GetFlag()));
    switch (notifyCode) {
        case StorageDaemon::DiskInfo::DiskState::REMOVED:
            LOGI("notifycode: REMOVED");
            wantParams.SetParam("diskId", AAFwk::String::Box(disk->GetDiskId()));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_REMOVED);
            break;
        case StorageDaemon::DiskInfo::DiskState::MOUNTED:
            LOGI("notifycode: MOUNTED");
            wantParams.SetParam("diskState", AAFwk::Integer::Box(StorageDaemon::DiskInfo::DiskState::MOUNTED));
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_DISK_MOUNTED);
            break;
        default:
            break;
    }
    want.SetParams(wantParams);
    EventFwk::CommonEventData commonData { want };
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}
}
} // namespace OHOS
