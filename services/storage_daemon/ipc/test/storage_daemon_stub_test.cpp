/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon_ipc_interface_code.h"
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
    int32_t g_code[] = {
        static_cast<int32_t>(StorageDaemonInterfaceCode::SHUTDOWN),
        static_cast<int32_t>(StorageDaemonInterfaceCode::MOUNT),
        static_cast<int32_t>(StorageDaemonInterfaceCode::UMOUNT),
        static_cast<int32_t>(StorageDaemonInterfaceCode::CHECK),
        static_cast<int32_t>(StorageDaemonInterfaceCode::FORMAT),
        static_cast<int32_t>(StorageDaemonInterfaceCode::PARTITION),
        static_cast<int32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC),
        static_cast<int32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::START_USER),
        static_cast<int32_t>(StorageDaemonInterfaceCode::STOP_USER),
        static_cast<int32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY),
        static_cast<int32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH),
        static_cast<int32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY),
        static_cast<int32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY),
        static_cast<int32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT),
        static_cast<int32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN),
        static_cast<int32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN),
        static_cast<int32_t>(StorageDaemonInterfaceCode::LOCK_SCREEN_STATUS),
        static_cast<int32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY),
        static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY),
    };
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

    int32_t ret = mock.OnRemoteRequest(static_cast<int32_t>(StorageDaemonInterfaceCode::START_USER), data, reply,
        option);
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

    StorageDaemonStubMock mock;

    EXPECT_CALL(mock, Shutdown()).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Mount(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UMount(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Check(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Format(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Partition(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, SetVolumeDescription(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, PrepareUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DestroyUserDirs(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, StartUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, StopUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, InitGlobalKey()).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, InitGlobalUserKeys()).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GenerateUserKeys(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DeleteUserKeys(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UpdateUserAuth(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, ActiveUserKey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, InactiveUserKey(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UpdateKeyContext(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, LockUserScreen(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UnlockUserScreen(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, MountDfsDocs(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetLockScreenStatus(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GenerateAppkey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DeleteAppkey(testing::_, testing::_)).WillOnce(testing::Return(E_OK));

    for (auto c : g_code) {
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

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleMount_001
 * @tc.desc: Verify the HandleMount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleMount_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, Mount(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleMount(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, Mount(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleMount(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleMount_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleUMount_001
 * @tc.desc: Verify the HandleUMount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleUMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUMount_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, UMount(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleUMount(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, UMount(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleUMount(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUMount_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleCheck_001
 * @tc.desc: Verify the HandleCheck function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleCheck_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleCheck_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, Check(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleCheck(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, Check(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleCheck(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleCheck_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandlePartition_001
 * @tc.desc: Verify the HandlePartition function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandlePartition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandlePartition_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, Partition(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandlePartition(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, Partition(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandlePartition(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandlePartition_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleFormat_001
 * @tc.desc: Verify the HandleFormat function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleFormat_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleFormat_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, Format(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleFormat(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, Format(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleFormat(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleFormat_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleInitGlobalKey_001
 * @tc.desc: Verify the HandleInitGlobalKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleInitGlobalKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInitGlobalKey_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, InitGlobalKey()).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleInitGlobalKey(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, InitGlobalKey()).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleInitGlobalKey(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInitGlobalKey_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleInitGlobalUserKeys_001
 * @tc.desc: Verify the HandleInitGlobalUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleInitGlobalUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInitGlobalUserKeys_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, InitGlobalUserKeys()).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleInitGlobalUserKeys(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, InitGlobalUserKeys()).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleInitGlobalUserKeys(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInitGlobalUserKeys_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleGenerateUserKeys_001
 * @tc.desc: Verify the HandleGenerateUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleGenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGenerateUserKeys_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, GenerateUserKeys(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleGenerateUserKeys(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, GenerateUserKeys(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleGenerateUserKeys(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGenerateUserKeys_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleDeleteUserKeys_001
 * @tc.desc: Verify the HandleDeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleDeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDeleteUserKeys_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, DeleteUserKeys(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleDeleteUserKeys(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, DeleteUserKeys(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleDeleteUserKeys(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDeleteUserKeys_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleUpdateUserAuth_001
 * @tc.desc: Verify the HandleUpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleUpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUpdateUserAuth_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, UpdateUserAuth(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleUpdateUserAuth(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, UpdateUserAuth(testing::_, testing::_, testing::_, testing::_,
        testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleUpdateUserAuth(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUpdateUserAuth_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleActiveUserKey_001
 * @tc.desc: Verify the HandleActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleActiveUserKey_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, ActiveUserKey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleActiveUserKey(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, ActiveUserKey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleActiveUserKey(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleActiveUserKey_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleInactiveUserKey_001
 * @tc.desc: Verify the HandleInactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleInactiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInactiveUserKey_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, InactiveUserKey(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleInactiveUserKey(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, InactiveUserKey(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleInactiveUserKey(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleInactiveUserKey_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleGetLockScreenStatus_001
 * @tc.desc: Verify the HandleGetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleGetLockScreenStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGetLockScreenStatus_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, GetLockScreenStatus(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleGetLockScreenStatus(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, GetLockScreenStatus(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleGetLockScreenStatus(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGetLockScreenStatus_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleGenerateAppkey_001
 * @tc.desc: Verify the HandleGenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleGenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGenerateAppkey_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, GenerateAppkey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleGenerateAppkey(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, GenerateAppkey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleGenerateAppkey(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    string keyId = reply2.ReadString();
    EXPECT_TRUE(keyId == "");
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleGenerateAppkey_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleDeleteAppkey_001
 * @tc.desc: Verify the HandleDeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleDeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDeleteAppkey_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, DeleteAppkey(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleDeleteAppkey(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, DeleteAppkey(testing::_, testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleDeleteAppkey(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleDeleteAppkey_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonTest_HandleUpdateKeyContext_001
 * @tc.desc: Verify the HandleUpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonStubTest, Storage_Manager_StorageDaemonTest_HandleUpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUpdateKeyContext_001 start";

    StorageDaemonStubMock mock;

    MessageParcel data1;
    MessageParcel reply1;
    EXPECT_CALL(mock, UpdateKeyContext(testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.HandleUpdateKeyContext(data1, reply1);
    EXPECT_TRUE(ret == E_OK);
    int32_t err = reply1.ReadInt32();
    EXPECT_TRUE(err == E_OK);

    MessageParcel data2;
    MessageParcel reply2;
    EXPECT_CALL(mock, UpdateKeyContext(testing::_)).WillOnce(testing::Return(E_ERR));
    ret = mock.HandleUpdateKeyContext(data2, reply2);
    EXPECT_TRUE(ret == E_OK);
    err = reply2.ReadInt32();
    EXPECT_TRUE(err == E_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_StorageDaemonTest_HandleUpdateKeyContext_001 end";
}
} // STORAGE_DAEMON
} // OHOS
