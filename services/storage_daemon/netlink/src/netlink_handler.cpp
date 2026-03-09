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

#include "netlink/netlink_handler.h"

#include "disk/disk_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
NetlinkHandler::NetlinkHandler(int32_t socket) : NetlinkListener(socket) {}

int32_t NetlinkHandler::Start()
{
    LOGI("[L1:NetlinkHandler] Start: >>> ENTER <<<");

    int32_t ret = this->StartListener();
    if (ret == E_OK) {
        LOGI("[L1:NetlinkHandler] Start: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:NetlinkHandler] Start: <<< EXIT FAILED <<< StartListener failed, ret=%{public}d", ret);
    }
    return ret;
}

int32_t NetlinkHandler::Stop()
{
    LOGI("[L1:NetlinkHandler] Stop: >>> ENTER <<<");

    int32_t ret = this->StopListener();
    if (ret == E_OK) {
        LOGI("[L1:NetlinkHandler] Stop: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L1:NetlinkHandler] Stop: <<< EXIT FAILED <<< StopListener failed, ret=%{public}d", ret);
    }
    return ret;
}

void NetlinkHandler::OnEvent(char *msg)
{
    LOGD("[L1:NetlinkHandler] OnEvent: >>> ENTER <<<");

    auto nlData = std::make_unique<NetlinkData>();
    nlData->Decode(msg);
    if (strcmp(nlData->GetSubsystem().c_str(), "block") == 0) {
        LOGD("[L1:NetlinkHandler] OnEvent: block device event, sysPath=%{public}s, devPath=%{public}s,"
             "subsystem=%{public}s, action=%{public}d",
            nlData->GetSyspath().c_str(), nlData->GetDevpath().c_str(),
            nlData->GetSubsystem().c_str(), nlData->GetAction());
        DiskManager::Instance().HandleDiskEvent(nlData.get());
        LOGD("[L1:NetlinkHandler] OnEvent: <<< EXIT SUCCESS <<< block event handled");
    } else {
        LOGD("[L1:NetlinkHandler] OnEvent: <<< EXIT SUCCESS <<< non-block subsystem=%{public}s, skipped",
             nlData->GetSubsystem().c_str());
    }
}
} // StorageDaemon
} // OHOS

