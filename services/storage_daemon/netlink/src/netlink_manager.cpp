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

#include "netlink/netlink_manager.h"

#include <cerrno>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/netlink.h>

#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
NetlinkManager* NetlinkManager::instance_ = nullptr;

NetlinkManager* NetlinkManager::Instance()
{
    if (instance_ == nullptr) {
        instance_ = new NetlinkManager();
    }

    return instance_;
}

int32_t NetlinkManager::Start()
{
    struct sockaddr_nl addr;
    int32_t sz = 256*1024;
    int32_t on = 1;

    (void)memset_s(&addr, sizeof(addr), 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = static_cast<uint32_t>(getpid());
    addr.nl_groups = 0xffffffff;

    socketFd_ = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT);
    if (socketFd_ < 0) {
        LOGE("Create netlink socket failed, errno %{public}d", errno);
        return E_ERR;
    }

    if (setsockopt(socketFd_, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) != 0) {
        LOGE("Set SO_RCVBUFFORCE failed, errno %{public}d", errno);
        (void)close(socketFd_);
        return E_ERR;
    }

    if (setsockopt(socketFd_, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) != 0) {
        LOGE("Set SO_PASSCRED failed, errno %{public}d", errno);
        (void)close(socketFd_);
        return E_ERR;
    }

    if (bind(socketFd_, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        LOGE("Socket bind failed, errno %{public}d", errno);
        (void)close(socketFd_);
        return E_ERR;
    }

    nlHandler_ = new NetlinkHandler(socketFd_);
    if (nlHandler_->Start()) {
        (void)close(socketFd_);
        return E_ERR;
    }
    return E_OK;
}

int32_t NetlinkManager::Stop()
{
    int32_t ret = 0;

    if (nlHandler_->Stop()) {
        ret = E_ERR;
    }
    delete nlHandler_;
    nlHandler_ = nullptr;
    (void)close(socketFd_);
    socketFd_ = -1;

    return ret;
}
} // StorageDaemon
} // OHOS
