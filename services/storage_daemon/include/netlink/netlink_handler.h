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

#ifndef OHOS_STORAGE_DAEMON_NETLINK_HANDLER_H
#define OHOS_STORAGE_DAEMON_NETLINK_HANDLER_H

#include "netlink_listener.h"

namespace OHOS {
namespace StorageDaemon {
class NetlinkHandler : public NetlinkListener {
public:
    explicit NetlinkHandler(int32_t listenerSocket);
    virtual ~NetlinkHandler() = default;
    int32_t Start();
    int32_t Stop();

protected:
    virtual void OnEvent(char *msg);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_NETLINK_HANDLER_H
