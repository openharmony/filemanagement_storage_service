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

#include "netlink/netlink_listener.h"

#include <cerrno>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/netlink.h>

#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

constexpr int POLL_IDLE_TIME = 1000;
constexpr int UEVENT_MSG_LEN = 1024;

namespace OHOS {
namespace StorageDaemon {
ssize_t UeventKernelMulticastRecv(int32_t socket, char *buffer, size_t length)
{
    struct iovec iov = { buffer, length };
    struct sockaddr_nl addr;
    char control[CMSG_SPACE(sizeof(struct ucred))];
    struct msghdr hdr = {
        .msg_name = &addr,
        .msg_namelen = sizeof(addr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
        .msg_control = control,
        .msg_controllen = sizeof(control),
        .msg_flags = 0,
    };
    struct cmsghdr *cmsg;

    ssize_t n = recvmsg(socket, &hdr, 0);
    if (n <= 0) {
        LOGE("Recvmsg failed, errno %{public}d", errno);
        return n;
    }

    if (addr.nl_groups == 0 || addr.nl_pid != 0) {
        LOGE("Groups or pid check failed");
        return E_ERR;
    }

    cmsg = CMSG_FIRSTHDR(&hdr);
    if (cmsg == nullptr || cmsg->cmsg_type != SCM_CREDENTIALS) {
        LOGE("SCM_CREDENTIALS check failed");
        return E_ERR;
    }

    struct ucred cred;
    if (memcpy_s(&cred, sizeof(cred), CMSG_DATA(cmsg), sizeof(struct ucred)) != EOK || cred.uid != 0) {
        LOGE("Uid check failed");
        return E_ERR;
    }

    return n;
}

void NetlinkListener::RecvUeventMsg()
{
    auto msg = std::make_unique<char[]>(UEVENT_MSG_LEN + 1);
    ssize_t count;

    while (1) {
        count = UeventKernelMulticastRecv(socketFd_, msg.get(), UEVENT_MSG_LEN);
        if (count <= 0) {
            (void)memset_s(msg.get(), UEVENT_MSG_LEN + 1, 0, UEVENT_MSG_LEN + 1);
            break;
        }
        if (count >= UEVENT_MSG_LEN) {
            continue;
        }

        msg.get()[count] = '\0';
        OnEvent(msg.get());
    }
}

int32_t NetlinkListener::ReadMsg(int32_t fd_count, struct pollfd ufds[2])
{
    int32_t i;
    for (i = 0; i < fd_count; i++) {
        if (ufds[i].revents == 0) {
            continue;
        }

        if (ufds[i].fd == socketPipe_[0]) {
            int32_t msg = 0;
            if (read(socketPipe_[0], &msg, 1) < 0) {
                LOGE("Read socket pipe failed");
                return E_ERR;
            }
            if (msg == 0) {
                LOGI("Stop listener");
                return E_ERR;
            }
        } else if (ufds[i].fd == socketFd_) {
            if ((static_cast<uint32_t>(ufds[i].revents) & POLLIN)) {
                RecvUeventMsg();
                continue;
            }
            if ((static_cast<uint32_t>(ufds[i].revents)) & (POLLERR | POLLHUP)) {
                LOGE("POLLERR | POLLHUP");
                return E_ERR;
            }
        }
    }
    return E_OK;
}

void NetlinkListener::RunListener()
{
    struct pollfd ufds[2];
    int32_t idle_time = POLL_IDLE_TIME;

    while (1) {
        int32_t fd_count = 0;

        ufds[fd_count].fd = socketPipe_[0];
        ufds[fd_count].events = POLLIN;
        ufds[fd_count].revents = 0;
        fd_count++;

        if (socketFd_ > -1) {
            ufds[fd_count].fd = socketFd_;
            ufds[fd_count].events = POLLIN;
            ufds[fd_count].revents = 0;
            fd_count++;
        }

        int32_t n = poll(ufds, fd_count, idle_time);
        if (n < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            }
            break;
        } else if (!n) {
            continue;
        }

        if (ReadMsg(fd_count, ufds) != 0) {
            return;
        }
    }
}

void NetlinkListener::EventProcess(void *object)
{
    if (object == nullptr) {
        return;
    }

    NetlinkListener* me = reinterpret_cast<NetlinkListener *>(object);
    me->RunListener();
}

int32_t NetlinkListener::StartListener()
{
    if (socketFd_ < 0) {
        return E_ERR;
    }

    if (pipe(socketPipe_) == -1) {
        LOGE("Pipe error");
        return E_ERR;
    }
    socketThread_ = std::make_unique<std::thread>(&NetlinkListener::EventProcess, this);
    if (socketThread_ == nullptr) {
        (void)close(socketPipe_[0]);
        (void)close(socketPipe_[1]);
        socketPipe_[0] = socketPipe_[1] = -1;
        return E_ERR;
    }

    return E_OK;
}

int32_t NetlinkListener::StopListener()
{
    int32_t msg = 0;
    write(socketPipe_[1], &msg, 1);

    if (socketThread_ != nullptr && socketThread_->joinable()) {
        socketThread_->join();
    }

    (void)close(socketPipe_[0]);
    (void)close(socketPipe_[1]);
    socketPipe_[0] = socketPipe_[1] = -1;

    return E_OK;
}

NetlinkListener::NetlinkListener(int32_t socket)
{
    socketFd_ = socket;
}
} // StorageDaemon
} // OHOS
