/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <memory>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/netlink.h>

#include "gtest/gtest.h"
#include "netlink_listener_mock.h"
#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
int32_t StartSocket(int32_t& socketFd);
class NetlinkListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: NetlinkListenerTest_StartListener_001
 * @tc.desc: Verify the StartListener function when socket not start.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkListenerTest, NetlinkListenerTest_StartListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkListenerTest_StartListener_001 start";
    int32_t socket = -1;
    std::shared_ptr<NetlinkListenerMock> mock = std::make_shared<NetlinkListenerMock>(socket);
    auto execFun = [](std::shared_ptr<NetlinkListenerMock> netlinkListerner) {
        int32_t ret = netlinkListerner->StartListener();
        EXPECT_TRUE(ret == E_ERR);
    };
    std::thread execThread(execFun, mock);
    sleep(1);
    execThread.join();
    GTEST_LOG_(INFO) << "NetlinkListenerTest_StartListener_001 end";
}

/**
 * @tc.name: NetlinkListenerTest_StartListener_StopListener_001
 * @tc.desc: Verify the StartListener/StopListener function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(NetlinkListenerTest, NetlinkListenerTest_StartListener_StopListener_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NetlinkListenerTest_StartListener_StopListener_001 start";
    int32_t socket = 0;
    int32_t ret = StartSocket(socket);
    ASSERT_TRUE(ret == 0 && socket >= 0);

    std::shared_ptr<NetlinkListenerMock> mock = std::make_shared<NetlinkListenerMock>(socket);
    auto execFun = [](std::shared_ptr<NetlinkListenerMock> netlinkListerner) {
        int32_t ret = netlinkListerner->StartListener();
        EXPECT_TRUE(ret == E_OK);
    };
    std::thread execThread(execFun, mock);
    sleep(1);
    ret = mock->StopListener();
    EXPECT_TRUE(ret == E_OK);
    execThread.join();

    (void)close(socket);
    GTEST_LOG_(INFO) << "NetlinkListenerTest_StartListener_StopListener_001 end";
}

int32_t StartSocket(int32_t& socketFd)
{
    struct sockaddr_nl addr;
    int32_t sz = 256 * 1024;
    int32_t on = 1;

    (void)memset_s(&addr, sizeof(addr), 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = 0xffffffff;

    socketFd = socket(PF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_KOBJECT_UEVENT);
    if (socketFd < 0) {
        LOGE("Create netlink socket failed, errno %{public}d", errno);
        return -1;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_RCVBUFFORCE, &sz, sizeof(sz)) != 0) {
        LOGE("Set SO_RCVBUFFORCE failed, errno %{public}d", errno);
        (void)close(socketFd);
        return -1;
    }

    if (setsockopt(socketFd, SOL_SOCKET, SO_PASSCRED, &on, sizeof(on)) != 0) {
        LOGE("Set SO_PASSCRED failed, errno %{public}d", errno);
        (void)close(socketFd);
        return -1;
    }

    if (bind(socketFd, (struct sockaddr *) &addr, sizeof(addr)) != 0) {
        LOGE("Socket bind failed, errno %{public}d", errno);
        (void)close(socketFd);
        return -1;
    }
    return 0;
}
} // STORAGE_DAEMON
} // OHOS
