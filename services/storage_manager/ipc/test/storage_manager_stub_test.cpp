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

#include "accesstoken_kit.h"
#include "storage_manager_ipc_interface_code.h"
#include "storage_manager_proxy.h"
#include "ipc/storage_manager_stub.h"
#include "ipc_skeleton.h"
#include "storage_manager_stub_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return Security::AccessToken::TOKEN_NATIVE;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
} // namespace OHOS::Security::AccessToken

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    pid_t callingUid = 5523;
    return callingUid;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    uint32_t callingTokenID = 100;
    return callingTokenID;
}
} // namespace OHOS

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

namespace {
    const int ERROR_CODE = 99999;
    int32_t g_code[] = {
        static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::REMOVE_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_START_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::STOP_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::COMPLETE_ADD_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_CREATED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_MOUNTED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_STATE_CHANGED),
        static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT),
        static_cast<int32_t>(StorageManagerInterfaceCode::UNMOUNT),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_CREATED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_DESTROYED),
        static_cast<int32_t>(StorageManagerInterfaceCode::PARTITION),
        static_cast<int32_t>(StorageManagerInterfaceCode::CREATE_USER_KEYS),
        static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_USER_KEYS),
        static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH),
        static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::ACTIVE_USER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::INACTIVE_USER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_KEY_CONTEXT),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_UUID),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_ID),
        static_cast<int32_t>(StorageManagerInterfaceCode::SET_VOL_DESC),
        static_cast<int32_t>(StorageManagerInterfaceCode::FORMAT),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_DISK_BY_ID),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_SYSTEM_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATUS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_CURR_BUNDLE_STATS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_CURR_USER_STATS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_VOLUMES),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_DISKS),
        static_cast<int32_t>(StorageManagerInterfaceCode::UNLOCK_USER_SCREEN),
        static_cast<int32_t>(StorageManagerInterfaceCode::LOCK_SCREEN_STATUS),
        static_cast<int32_t>(StorageManagerInterfaceCode::SET_BUNDLE_QUOTA),
        static_cast<int32_t>(StorageManagerInterfaceCode::GENERATE_APP_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_APP_KEY),
        static_cast<uint32_t>(StorageManagerInterfaceCode::CREATE_RECOVER_KEY),
        static_cast<uint32_t>(StorageManagerInterfaceCode::SET_RECOVER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_MOUNT),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_MTP_UNMOUNT),
    };
}

class StorageManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function with error descriptor.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 start";

    StorageManagerStubMock mock;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(u"error descriptor");
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER), data,
        reply, option);
    EXPECT_TRUE(ret == E_PERMISSION_DENIED) << "descriptor error";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002
 * @tc.desc: Verify the OnRemoteRequest function with error code.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002 start";

    StorageManagerStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";

    int32_t ret = mock.OnRemoteRequest(ERROR_CODE, data, reply, option);
    EXPECT_TRUE(ret != E_OK) << "request code error";

    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003 start";

    StorageManagerStubMock mock;
    EXPECT_CALL(mock, PrepareAddUser(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, RemoveUser(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, PrepareStartUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, StopUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, CompleteAddUser(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyVolumeCreated(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyVolumeMounted(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyVolumeStateChanged(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Mount(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Unmount(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyDiskCreated(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyDiskDestroyed(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Partition(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetVolumeByUuid(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetVolumeById(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, SetVolumeDescription(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, Format(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetDiskById(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GenerateUserKeys(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DeleteUserKeys(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UpdateUserAuth(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, ActiveUserKey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, InactiveUserKey(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UpdateKeyContext(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetFreeSizeOfVolume(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetTotalSizeOfVolume(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetSystemSize(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetTotalSize(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetFreeSize(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetBundleStats(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetCurrentBundleStats(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetUserStorageStats(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetUserStorageStats(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetAllVolumes(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetAllDisks(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UnlockUserScreen(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GetLockScreenStatus(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, SetBundleQuota(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, GenerateAppkey(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, DeleteAppkey(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, UpdateUseAuthWithRecoveryKey(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, CreateRecoverKey(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, SetRecoverKey(testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyMtpMounted(testing::_, testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    EXPECT_CALL(mock, NotifyMtpUnmounted(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));

    for (auto c : g_code) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option(MessageOption::TF_SYNC);
        bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
        EXPECT_TRUE(bRet) << "write token error";
        int32_t ret = mock.OnRemoteRequest(c, data, reply, option);
        EXPECT_TRUE(ret == E_OK);
        EXPECT_TRUE(reply.ReadInt32() == E_OK);
    }

    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003 start";
    int32_t code[] = {
        static_cast<int32_t>(StorageManagerInterfaceCode::LOCK_USER_SCREEN),
#ifdef STORAGE_SERVICE_MEDIA_FUSE
        static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT_MEDIA_FUSE),
        static_cast<int32_t>(StorageManagerInterfaceCode::UMOUNT_MEDIA_FUSE),
#endif
    };
    StorageManagerStubMock mock;
    for (auto c : code) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option(MessageOption::TF_SYNC);
        bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
        EXPECT_TRUE(bRet) << "write token error";
        int32_t ret = mock.OnRemoteRequest(c, data, reply, option);
        EXPECT_EQ(ret, E_BUNDLEMGR_ERROR);
    }

    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005 start";
    StorageManagerStubMock mock;
    EXPECT_CALL(mock, GetFileEncryptStatus(testing::_, testing::_, testing::_)).WillOnce(testing::Return(E_OK));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    int32_t ret = mock.OnRemoteRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_FILE_ENCRYPT_STATUS), data,
        reply, option);
    EXPECT_TRUE(ret == E_OK);
    reply.ReadBool();
    EXPECT_TRUE(reply.ReadInt32() == E_OK);

    EXPECT_CALL(mock, GetUserNeedActiveStatus(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    ret = mock.OnRemoteRequest(static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_NEED_ACTIVE_STATUS), data,
        reply, option);
    EXPECT_TRUE(ret == E_OK);
    reply.ReadBool();
    EXPECT_TRUE(reply.ReadInt32() == E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005 end";
}
} // STORAGE_MANAGER
} // OHOS
