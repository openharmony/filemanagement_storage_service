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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon_proxy.h"
#include "ipc/storage_daemon_stub.h"
#include "storage_daemon_stub_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
    const int ERROR_CODE = 99999;
}

class StorageDaemonStubTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function with error descriptor.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(u"error descriptor");
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(IStorageDaemon::START_USER, data, reply, option);
    EXPECT_TRUE(ret == E_PERMISSION_DENIED) << "descriptor error";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_002
 * @tc.desc: Verify the OnRemoteRequest function with error code.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_002 start";

    StorageDaemonStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    bool bRet = data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(ERROR_CODE, data, reply, option);
    EXPECT_TRUE(ret != E_OK) << "request code error";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_003
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_003 start";

    int code[] = {
        IStorageDaemon::SHUTDOWN,
        IStorageDaemon::MOUNT,
        IStorageDaemon::UMOUNT,
        IStorageDaemon::CHECK,
        IStorageDaemon::FORMAT,
        IStorageDaemon::PREPARE_USER_DIRS,
        IStorageDaemon::DESTROY_USER_DIRS,
        IStorageDaemon::START_USER,
        IStorageDaemon::STOP_USER
    };

    StorageDaemonStubMock mock;

    EXPECT_CALL(mock, Shutdown()).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, PrepareUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DestroyUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, StartUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, StopUser(testing::_)).WillOnce(testing::Return(E_OK));

    for (auto c : code) {
        if (c >= IStorageDaemon::MOUNT && c <= IStorageDaemon::FORMAT) {
            continue;
        }
        MessageParcel data;
        MessageParcel reply;
        MessageOption option(MessageOption::TF_SYNC);
        bool bRet = data.WriteInterfaceToken(StorageDaemonProxy::GetDescriptor());
        EXPECT_TRUE(bRet) << "write token error";
        int32_t ret = mock.OnRemoteRequest(c, data, reply, option);
        EXPECT_TRUE(ret == E_OK);
        EXPECT_TRUE(E_OK == reply.ReadInt32());
    }

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonStubTest_OnRemoteRequest_003 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandlePrepareUserDirs_001
 * @tc.desc: Verify the HandlePrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandlePrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandlePrepareUserDirs_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, PrepareUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandlePrepareUserDirs(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, PrepareUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandlePrepareUserDirs(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandlePrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleDestroyUserDirs_001
 * @tc.desc: Verify the HandleDestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleDestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDestroyUserDirs_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, DestroyUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleDestroyUserDirs(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, DestroyUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleDestroyUserDirs(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDestroyUserDirs_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleStartUser_001
 * @tc.desc: Verify the HandleStartUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleStartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleStartUser_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, StartUser(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleStartUser(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, StartUser(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleStartUser(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleStartUser_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleStopUser_001
 * @tc.desc: Verify the HandleStopUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleStopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleStopUser_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, StopUser(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleStopUser(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, StopUser(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleStopUser(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleStopUser_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleSetVolDesc_001
 * @tc.desc: Verify the HandleSetVolDesc function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleSetVolDesc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleSetVolDesc_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, SetVolumeDescription(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleSetVolDesc(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, SetVolumeDescription(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleSetVolDesc(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleSetVolDesc_001 end";
}
} // STORAGE_DAEMON
} // OHOS
