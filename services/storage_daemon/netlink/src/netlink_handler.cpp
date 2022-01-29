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
#include "netlink/netlink_handler.h"

#include "netlink/netlink_data.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
void HandleDiskEventStub(NetlinkData *nlData)
{
    LOGI("GetAction %{public}d", nlData->GetAction());
    LOGI("GetDevpath %{public}s", nlData->GetDevpath().c_str());
    LOGI("GetSyspath %{public}s", nlData->GetSyspath().c_str());
    LOGI("GetSubsystem %{public}s", nlData->GetSubsystem().c_str());
    LOGI("GetParam MAJOR %{public}s", nlData->GetParam("MAJOR").c_str());
    LOGI("GetParam MINOR %{public}s", nlData->GetParam("MINOR").c_str());
    LOGI("GetParam DEVNAME %{public}s", nlData->GetParam("DEVNAME").c_str());
    LOGI("GetParam DEVTYPE %{public}s", nlData->GetParam("DEVTYPE").c_str());
    LOGI("GetParam SEQNUM %{public}s", nlData->GetParam("SEQNUM").c_str());

    return;
}

NetlinkHandler::NetlinkHandler(int32_t socket) : NetlinkListener(socket) {}

int32_t NetlinkHandler::Start()
{
    return this->StartListener();
}

int32_t NetlinkHandler::Stop()
{
    return this->StopListener();
}

void NetlinkHandler::OnEvent(char *msg)
{
    auto nlData = std::make_unique<NetlinkData>();

    nlData->Decode(msg);
    if (strcmp(nlData->GetSubsystem().c_str(), "block") == 0) {
        HandleDiskEventStub(nlData.get());
    }
}
} // StorageDaemon
} // OHOS
