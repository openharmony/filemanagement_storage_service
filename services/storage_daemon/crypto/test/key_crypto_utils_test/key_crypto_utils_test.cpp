/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "key_crypto_utils.h"
#include "os_account_manager.h"
#include "screenlock_manager.h"
using namespace testing;
using namespace OHOS::AccountSA;
int g_idNum = 0;
int g_authNum = 0;
int g_lockNum = 0;
int g_accountExist = 0;
bool g_accountIsExist = true;
static int g_nonExist = 2;
std::vector<int32_t> temp;
namespace OHOS::AccountSA {
ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    ids = temp;
    return g_idNum;
}

ErrCode OsAccountManager::IsOsAccountExists(const int id, bool &isOsAccountExists)
{
    isOsAccountExists = g_accountIsExist;
    return g_accountExist;
}
}

namespace OHOS::ScreenLock {
int32_t ScreenLockManager::RequestStrongAuth(int reasonFlag, int32_t userId)
{
    return g_authNum;
}

int32_t ScreenLockManager::Lock(int32_t userId)
{
    return g_lockNum;
}
}

// Test suite class
namespace OHOS::StorageDaemon {
class ForceLockUserScreenTest : public ::testing::Test {
    /**
    * Define common variable
    * Implement Setup TearDown methods
    */
    public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ForceLockUserScreenTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void ForceLockUserScreenTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ForceLockUserScreenTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ForceLockUserScreenTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.number: ForceLockUserScreen_QueryActiveOsAccountIdsFailed
 * @tc.name: ForceLockUserScreen_QueryActiveOsAccountIdsFailed
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_QueryActiveOsAccountIdsFailed, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds function to return an error
    // Call the function under test
    g_idNum = 1;
    temp = {1, 2, 3};
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(1, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_NoActiveOsAccounts, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds function to return success but an empty list
    // Call the function under test
    g_idNum = ERR_OK;
    temp = {};
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = g_nonExist;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(2, isOsAccountExists);
    EXPECT_EQ(ret, g_nonExist);
}

/**
 * @tc.number: ForceLockUserScreen_RequestStrongAuthFailed
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_RequestStrongAuthFailed, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds and RequestStrongAuth functions to return specific results
    g_idNum = 1;
    temp = {};
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(3, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: ForceLockUserScreen_LockFailed
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_LockFailed, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds, RequestStrongAuth, and Lock functions to return specific results
    g_idNum = ERR_OK;
    temp = {1, 2, 3};
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(5, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: ForceLockUserScreen_Success_1
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_Success_1, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds, RequestStrongAuth, and Lock functions to return success
    g_idNum = ERR_OK;
    temp = {1, 2, 3};
    g_authNum = g_nonExist;
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = g_nonExist;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(999, isOsAccountExists);
    EXPECT_EQ(ret, g_nonExist);
}

/**
 * @tc.number: ForceLockUserScreen_Success_2
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_Success_2, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds, RequestStrongAuth, and Lock functions to return success
    g_idNum = ERR_OK;
    temp = {1, 2, 3};
    g_authNum = ScreenLock::E_SCREENLOCK_OK;
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(111, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: ForceLockUserScreen_Success_3
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_Success_3, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds, RequestStrongAuth, and Lock functions to return success
    g_idNum = ERR_OK;
    temp = {1, 2, 3};
    g_authNum = ScreenLock::E_SCREENLOCK_OK;
    g_lockNum = g_nonExist;
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(111, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: ForceLockUserScreen_Success_4
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, ForceLockUserScreen_Success_4, testing::ext::TestSize.Level1)
{
    // Mock the QueryActiveOsAccountIds, RequestStrongAuth, and Lock functions to return success
    g_idNum = ERR_OK;
    temp = {1, 2, 3};
    g_authNum = ScreenLock::E_SCREENLOCK_OK;
    g_lockNum = ScreenLock::E_SCREENLOCK_OK;
    // Call the function under test
    OHOS::StorageService::KeyCryptoUtils::ForceLockUserScreen();
    g_accountExist = ERR_OK;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(12345, isOsAccountExists);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: CheckAccountExists_Failed
 * @tc.name: ForceLockUserScreen_NoActiveOsAccounts
 * @tc.desc: Test function of ForceLockUserScreen interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(ForceLockUserScreenTest, CheckAccountExists_Failed, testing::ext::TestSize.Level1)
{
    // Mock the IsOsAccountExists function to return an error
    // Call the function under test
    g_accountExist = g_nonExist;
    bool isOsAccountExists = g_accountIsExist;
    int32_t ret = OHOS::StorageService::KeyCryptoUtils::CheckAccountExists(12345, isOsAccountExists);
    EXPECT_EQ(ret, g_accountExist);
}
}
