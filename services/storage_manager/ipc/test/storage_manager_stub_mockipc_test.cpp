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

#include "ipc_skeleton_mock.h"
#include "storage_manager_ipc_interface_code.h"
#include "storage_manager_proxy.h"
#include "ipc/storage_manager_stub.h"
#include "storage_manager_stub_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
using namespace std;
using namespace testing::ext;
constexpr pid_t BACKUP_SA_UID = 1089;
constexpr pid_t DFS_UID = 1009;

class StorageManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    static inline shared_ptr<IPCSkeletonMoc> ipcSkeletonMoc_ = nullptr;
};

void StorageManagerStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    ipcSkeletonMoc_ = make_shared<IPCSkeletonMoc>();
    IPCSkeletonMoc::ipcSkeletonMoc = ipcSkeletonMoc_;
}

void StorageManagerStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    IPCSkeletonMoc::ipcSkeletonMoc = nullptr;
    ipcSkeletonMoc_ = nullptr;
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: IB3B3O
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 start";

    StorageManagerStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    int32_t code = static_cast<int32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATS_INCREASE);

    EXPECT_CALL(*ipcSkeletonMoc_, GetCallingUid()).WillOnce(testing::Return(BACKUP_SA_UID));
    EXPECT_CALL(mock, GetBundleStatsForIncrease(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    int32_t ret = mock.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: IB3B3O
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
    int32_t code = static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS_BY_TYPE);

    EXPECT_CALL(*ipcSkeletonMoc_, GetCallingUid()).WillOnce(testing::Return(BACKUP_SA_UID));
    EXPECT_CALL(mock, GetUserStorageStatsByType(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    int32_t ret = mock.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_002 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: IB3B3O
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003 start";

    StorageManagerStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    int32_t code = static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_MEM_PARA);

    EXPECT_CALL(*ipcSkeletonMoc_, GetCallingUid()).WillOnce(testing::Return(BACKUP_SA_UID));
    EXPECT_CALL(mock, UpdateMemoryPara(testing::_, testing::_)).WillOnce(testing::Return(E_OK));
    int32_t ret = mock.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_003 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: IB3B3O
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004 start";

    StorageManagerStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    int32_t code = static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT_DFS_DOCS);

    EXPECT_CALL(*ipcSkeletonMoc_, GetCallingUid()).WillOnce(testing::Return(DFS_UID));
    EXPECT_CALL(mock, MountDfsDocs(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    int32_t ret = mock.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_004 end";
}

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: IB3B3O
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005 start";

    StorageManagerStubMock mock;

    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
    EXPECT_TRUE(bRet) << "write token error";
    int32_t code = static_cast<int32_t>(StorageManagerInterfaceCode::UMOUNT_DFS_DOCS);

    EXPECT_CALL(*ipcSkeletonMoc_, GetCallingUid()).WillOnce(testing::Return(DFS_UID));
    EXPECT_CALL(mock, UMountDfsDocs(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(E_OK));
    int32_t ret = mock.OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_005 end";
}
} // STORAGE_MANAGER
} // OHOS
