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

#include <gtest/gtest.h>

#include "ipc/istorage_daemon.h"
#include "ipc/storage_daemon_proxy.h"
#include "storage_daemon_service_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

namespace {
    constexpr int32_t USER_ID1 = 100;
}

class StorageDaemonProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown() {};

    std::shared_ptr<StorageDaemonProxy> proxy_ = nullptr;
    sptr<StorageDaemonServiceMock> mock_ = nullptr;
};

void StorageDaemonProxyTest::SetUp()
{
    mock_ = new StorageDaemonServiceMock();
    proxy_ = std::make_shared<StorageDaemonProxy>(mock_);
}

/**
 * @tc.name: StorageDaemonProxyTest_Shutdown_001
 * @tc.desc: Verify the Shutdown function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_Shutdown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Shutdown_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->Shutdown();
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(IStorageDaemon::SHUTDOWN == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_Shutdown_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_PrepareUserDirs_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->PrepareUserDirs(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(IStorageDaemon::PREPARE_USER_DIRS == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DestroyUserDirs_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->DestroyUserDirs(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(IStorageDaemon::DESTROY_USER_DIRS == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_StartUser_001
 * @tc.desc: Verify the StartUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StartUser_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->StartUser(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(IStorageDaemon::START_USER == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StartUser_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_StopUser_001
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StopUser_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));

    int32_t ret = proxy_->StopUser(USER_ID1);
    ASSERT_TRUE(ret == E_OK);
    ASSERT_TRUE(IStorageDaemon::STOP_USER == mock_->code_);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_StopUser_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_SetVolumeDescription_001
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetVolumeDescription_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    string volId = "vol-0-1";
    string description = "description-1";
    int32_t ret = proxy_->SetVolumeDescription(volId, description);
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_SetVolumeDescription_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InitGlobalKey_001
 * @tc.desc: Verify the InitGlobalKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InitGlobalKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalKey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InitGlobalKey();
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalKey_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InitGlobalUserKeys_001
 * @tc.desc: Verify the InitGlobalUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InitGlobalUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalUserKeys_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InitGlobalUserKeys();
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InitGlobalUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_GenerateUserKeys_001
 * @tc.desc: Verify the GenerateUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_GenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateUserKeys_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->GenerateUserKeys(USER_ID1, IStorageDaemon::CRYPTO_FLAG_EL1);
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_GenerateUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProxyTest_InactiveUserKey_001
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonProxyTest, StorageDaemonProxyTest_InactiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InactiveUserKey_001 start";

    EXPECT_CALL(*mock_, SendRequest(testing::_, testing::_, testing::_, testing::_))
        .Times(1)
        .WillOnce(testing::Invoke(mock_.GetRefPtr(), &StorageDaemonServiceMock::InvokeSendRequest));
    int32_t ret = proxy_->InactiveUserKey(USER_ID1);
    ASSERT_TRUE(ret == E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProxyTest_InactiveUserKey_001 end";
}
} // STORAGE_DAEMON
} // OHOS