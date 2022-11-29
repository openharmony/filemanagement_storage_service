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

#include <gtest/gtest.h>

#include "storage_daemon_client.h"
#include "ipc/istorage_daemon.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "help_utils.h"
namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class StorageDaemonClientTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageDaemonClient* storageDaemonClient_;
};

void StorageDaemonClientTest::SetUp()
{
    storageDaemonClient_ = new StorageDaemonClient();
}

void StorageDaemonClientTest::TearDown(void)
{
    StorageTest::StorageTestUtils::ClearTestResource();
    if (storageDaemonClient_ != nullptr) {
        delete storageDaemonClient_;
        storageDaemonClient_ = nullptr;
    }
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 103;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->DestroyUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_StartUser_001
 * @tc.desc: check the StartUser function when args are normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 104;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK) << "PrepareUserDirs error";
    ret = storageDaemonClient_->StartUser(userid);
    EXPECT_TRUE(ret == E_OK) << "StartUser error";

    ret = storageDaemonClient_->StopUser(userid);
    EXPECT_TRUE(ret == E_OK) << "check StopUser error";

    storageDaemonClient_->DestroyUserDirs(userid, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001
 * @tc.desc: Verify the PrepareUserSpace function when args are normal.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 105;
    std::string volId = "vol-1-1";
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserSpace(userid, volId, flags);
    EXPECT_TRUE(ret == E_OK);

    storageDaemonClient_->DestroyUserSpace(userid, volId, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ActiveUserKey_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 108;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->GenerateUserKeys(userid, flags);
    ASSERT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->ActiveUserKey(userid, {}, {});
    EXPECT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->InactiveUserKey(userid);
    EXPECT_TRUE(ret == E_OK);

    storageDaemonClient_->DeleteUserKeys(userid);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_ActiveUserKey_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 109;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->PrepareUserDirs(userid, flags);
    ret = storageDaemonClient_->StartUser(userid);
    EXPECT_TRUE(ret == E_OK) << "StartUser error";

    ret = storageDaemonClient_->UpdateUserAuth(userid, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    ret = storageDaemonClient_->StopUser(userid);
    storageDaemonClient_->DestroyUserDirs(userid, flags);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H0F7I
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001 start";

    ASSERT_TRUE(storageDaemonClient_ != nullptr);

    int32_t userid = 110;
    int32_t flags = IStorageDaemon::CRYPTO_FLAG_EL2;
    int32_t ret = storageDaemonClient_->GenerateUserKeys(userid, flags);
    ASSERT_TRUE(ret == E_OK);

    ret = storageDaemonClient_->UpdateUserAuth(userid, {}, {}, {});
    EXPECT_TRUE(ret == E_OK) << "UpdateUserAuth error";

    ret = storageDaemonClient_->UpdateKeyContext(userid);
    EXPECT_TRUE(ret == E_OK);

    storageDaemonClient_->DeleteUserKeys(userid);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateKeyContext_001 end";
}
}
}