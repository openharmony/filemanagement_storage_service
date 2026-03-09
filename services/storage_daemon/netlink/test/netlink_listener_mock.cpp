/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "netlink_listener_real_mock.h"
#include "netlink/netlink_listener.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;

NetlinkListener::NetlinkListener(int32_t socket)
{
    socketFd_ = socket;
}

int32_t NetlinkListener::StartListener()
{
    return INetlinkListenerMock::netlinkListenerMoc->StartListener();
}

int32_t NetlinkListener::StopListener()
{
    return INetlinkListenerMock::netlinkListenerMoc->StopListener();
}
}
}