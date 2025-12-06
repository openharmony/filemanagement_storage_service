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

#include "storage_daemon_test.h"

#include <filesystem>
#include <vector>
#include <fstream>
#include <gmock/gmock.h>
#include <string>

#include "file_ex.h"
#include "file_sharing/file_sharing.h"
#include "userdata_dir_info.h"

namespace {
#ifdef USER_CRYPTO_MIGRATE_KEY
constexpr const char *DATA_SERVICE_EL0_STORAGE_DAEMON_SD = "/data/service/el0/storage_daemon/sd";
constexpr const char *NEED_RESTORE_SUFFIX = "/latest/need_restore";
#endif
}

namespace OHOS {
bool SaveStringToFile(const std::string& filePath, const std::string& content, bool truncated /*= true*/)
{
    return StorageDaemon::Test::StorageDaemonTest::g_saveStringToFile;
}

namespace StorageDaemon {
int SetupFileSharingDir()
{
    return Test::StorageDaemonTest::g_setupFileSharingDir;
}

bool SaveStringToFileSync(const std::string &path, const std::string &data, std::string &errMsg)
{
    return Test::StorageDaemonTest::g_saveStringToFileSync;
}

namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std::filesystem;
static const std::vector<string> USER_DIRS = {
    USER_EL1_DIR,
    USER_EL2_DIR,
    USER_EL3_DIR,
    USER_EL4_DIR,
    USER_EL5_DIR,
};

static const std::vector<string> NATO_USER_DIRS = {
    NATO_EL2_DIR,
    NATO_EL3_DIR,
    NATO_EL4_DIR,
};

std::string GetKeyDirByUserAndType(unsigned int user, KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL1_KEY:
            keyDir = std::string(USER_EL1_DIR) + "/" + std::to_string(user);
            break;
        case EL2_KEY:
            keyDir = std::string(USER_EL2_DIR) + "/" + std::to_string(user);
            break;
        case EL3_KEY:
            keyDir = std::string(USER_EL3_DIR) + "/" + std::to_string(user);
            break;
        case EL4_KEY:
            keyDir = std::string(USER_EL4_DIR) + "/" + std::to_string(user);
            break;
        case EL5_KEY:
            keyDir = std::string(USER_EL5_DIR) + "/" + std::to_string(user);
            break;
        default:
            break;
    }
    return keyDir;
}

std::string GetNatoNeedRestorePath(uint32_t userId, KeyType type)
{
    std::string keyDir = "";
    switch (type) {
        case EL2_KEY:
            keyDir = std::string(NATO_EL2_DIR) + "/" + std::to_string(userId);
            break;
        case EL3_KEY:
            keyDir = std::string(NATO_EL3_DIR) + "/" + std::to_string(userId);
            break;
        case EL4_KEY:
            keyDir = std::string(NATO_EL4_DIR) + "/" + std::to_string(userId);
            break;
        default:
            break;
    }
    return keyDir;
}

void StorageDaemonTest::CreateNeedRestoreFile(int32_t userId, KeyType type)
{
    auto path = GetUserRootPath(type);
    std::string userPath = path + "/" + std::to_string(userId);
    std::error_code errCode;
    if (std::filesystem::exists(userPath, errCode)) {
        std::filesystem::rename(userPath, userPath + "_bak");
    }
    std::filesystem::create_directories(userPath + "/latest", errCode);
    std::ofstream file(storageDaemon_->GetNeedRestoreFilePathByType(userId, type));
    file.close();
}

void StorageDaemonTest::DeleteNeedRestoreFile(int32_t userId, KeyType type)
{
    auto path = GetUserRootPath(type);
    std::string userPath = path + "/" + std::to_string(userId);
    std::error_code errCode;
    std::filesystem::remove_all(userPath, errCode);
    if (std::filesystem::exists(userPath + "_bak", errCode)) {
        std::filesystem::rename(userPath + "_bak", userPath, errCode);
    }
}

std::string StorageDaemonTest::GetUserRootPath(KeyType type)
{
    static std::map<KeyType, std::string> UserRootPaths = {
        {EL1_KEY, USER_EL1_DIR},
        {EL2_KEY, USER_EL2_DIR},
        {EL3_KEY, USER_EL3_DIR},
        {EL4_KEY, USER_EL4_DIR},
        {EL5_KEY, USER_EL5_DIR},
    };
    auto it = UserRootPaths.find(type);
    if (it == UserRootPaths.end()) {
        return "";
    }
    return it->second;
}

void StorageDaemonTest::PrepareConfigDir()
{
    for (auto const &dir : USER_DIRS) {
        std::string userPath = dir + "/" + std::to_string(userId_);
        std::error_code errCode;
        if (std::filesystem::exists(userPath, errCode)) {
            std::filesystem::rename(userPath, userPath + "_bak", errCode);
        }
        std::filesystem::create_directories(userPath + "/latest", errCode);
        std::ofstream file(userPath + "/latest/need_restore");
        file.close();
    }

    for (auto const &dir : NATO_USER_DIRS) {
        std::error_code errCode;
        if (std::filesystem::exists(dir, errCode)) {
            std::filesystem::rename(dir, dir + "_bak", errCode);
        }
        std::string natoUserPath = dir + "/" + std::to_string(userId_);
        std::filesystem::create_directories(natoUserPath + "/latest", errCode);
        std::ofstream restoreFile(natoUserPath + "/latest/need_restore");
        std::ofstream fscryptFile(natoUserPath + "/fscrypt_version");
        restoreFile.close();
        fscryptFile.close();
    }
}

void StorageDaemonTest::RefreshConfigDir()
{
    for (auto const &dir : USER_DIRS) {
        std::string userPath = dir + "/" + std::to_string(userId_);
        std::error_code errCode;
        if (!std::filesystem::exists(userPath + "/latest/need_restore", errCode)) {
            std::filesystem::create_directories(userPath + "/latest", errCode);
            std::ofstream file(userPath + "/latest/need_restore");
            file.close();
        }
    }

    for (auto const &dir : NATO_USER_DIRS) {
        std::error_code errCode;
        std::string natoUserPath = dir + "/" + std::to_string(userId_);
        if (!std::filesystem::exists(natoUserPath + "/latest/need_restore", errCode) ||
            !std::filesystem::exists(natoUserPath + "/fscrypt_version", errCode)) {
            std::filesystem::create_directories(natoUserPath + "/latest", errCode);
            std::ofstream restoreFile(natoUserPath + "/latest/need_restore");
            std::ofstream fscryptFile(natoUserPath + "/fscrypt_version");
            restoreFile.close();
            fscryptFile.close();
        }
    }
}

void StorageDaemonTest::DestroyConfigDir()
{
    for (auto const &dir : USER_DIRS) {
        std::string userPath = dir + "/" + std::to_string(userId_);
        std::error_code errCode;
        std::filesystem::remove_all(userPath, errCode);
        if (std::filesystem::exists(userPath + "_bak", errCode)) {
            std::filesystem::rename(userPath + "_bak", userPath, errCode);
        }
    }

    for (auto const &dir : NATO_USER_DIRS) {
        std::error_code errCode;
        std::filesystem::remove_all(dir, errCode);
        if (std::filesystem::exists(dir + "_bak", errCode)) {
            std::filesystem::rename(dir + "_bak", dir, errCode);
        }
    }
}

void StorageDaemonTest::SetUpTestCase(void)
{
    PrepareConfigDir();
}

void StorageDaemonTest::TearDownTestCase(void)
{
    DestroyConfigDir();
}

void StorageDaemonTest::SetUp()
{
    storageDaemon_ = std::make_shared<StorageDaemon>();

    keyManagerMock_ = std::make_shared<KeyManagerMock>();
    KeyManagerMock::iKeyManagerMock_ = keyManagerMock_;
    keyManagerExtMock_ = std::make_shared<KeyManagerExtMock>();
    KeyManagerExtMock::iKeyManagerExtMock_ = keyManagerExtMock_;
    userManagerMock_ = std::make_shared<UserManagerMock>();
    UserManagerMock::iUserManagerMock_ = userManagerMock_;
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    mountManagerMock_ = std::make_shared<MountManagerMoc>();
    MountManagerMoc::mountManagerMoc = mountManagerMock_;
    iamClientMock_ = std::make_shared<IamClientMoc>();
    IamClientMoc::iamClientMoc = iamClientMock_;

    EXPECT_CALL(*keyManagerMock_, GetKeyDirByUserAndType(_, _)).WillRepeatedly(Invoke(GetKeyDirByUserAndType));
    EXPECT_CALL(*keyManagerMock_, GetNatoNeedRestorePath(_, _)).WillRepeatedly(Invoke(GetNatoNeedRestorePath));
    EXPECT_CALL(*userManagerMock_, DestroyUserDirs(_, _)).WillRepeatedly(Return(E_OK));
}

void StorageDaemonTest::TearDown(void)
{
    storageDaemon_ = nullptr;
    token_.clear();
    secret_.clear();

    KeyManagerMock::iKeyManagerMock_ = nullptr;
    keyManagerMock_ = nullptr;
    KeyManagerExtMock::iKeyManagerExtMock_ = nullptr;
    keyManagerExtMock_ = nullptr;
    UserManagerMock::iUserManagerMock_ = nullptr;
    userManagerMock_ = nullptr;
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    MountManagerMoc::mountManagerMoc = nullptr;
    mountManagerMock_ = nullptr;
    IamClientMoc::iamClientMoc = nullptr;
    iamClientMock_ = nullptr;
    RefreshConfigDir();
}

/**
 * @tc.name: StorageDaemonTest_GetCryptoFlag_001
 * @tc.desc: Verify the GetCryptoFlag function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GetCryptoFlag_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    uint32_t flags;
    std::vector<std::pair<KeyType, uint32_t>> testCases = {
        {EL1_KEY, IStorageDaemonEnum::CRYPTO_FLAG_EL1},
        {EL2_KEY, IStorageDaemonEnum::CRYPTO_FLAG_EL2},
        {EL3_KEY, IStorageDaemonEnum::CRYPTO_FLAG_EL3},
        {EL4_KEY, IStorageDaemonEnum::CRYPTO_FLAG_EL4},
        {EL5_KEY, IStorageDaemonEnum::CRYPTO_FLAG_EL5},
    };

    for (const auto &testCase : testCases) {
        EXPECT_EQ(storageDaemon_->GetCryptoFlag(testCase.first, flags), E_OK);
        EXPECT_EQ(flags, testCase.second);
    }

    auto ret = storageDaemon_->GetCryptoFlag(static_cast<KeyType>(EL5_KEY + 1), flags);
    EXPECT_EQ(ret, E_KEY_TYPE_INVALID);
}

#ifdef USER_CRYPTO_MIGRATE_KEY
/**
 * @tc.name: StorageDaemonTest_GetNeedRestoreFilePathByType_001
 * @tc.desc: Verify the GetNeedRestoreFilePathByType function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GetNeedRestoreFilePathByType_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    int32_t userId = 100;
    std::vector<std::pair<KeyType, std::string>> testCases = {
        {EL1_KEY, USER_EL1_DIR},
        {EL2_KEY, USER_EL2_DIR},
        {EL3_KEY, USER_EL3_DIR},
        {EL4_KEY, USER_EL4_DIR},
        {EL5_KEY, USER_EL5_DIR},
    };

    for (const auto& testCase : testCases) {
        auto ret = storageDaemon_->GetNeedRestoreFilePathByType(userId, testCase.first);
        EXPECT_EQ(ret, storageDaemon_->GetNeedRestoreFilePath(userId, testCase.second));
    }

    auto ret = storageDaemon_->GetNeedRestoreFilePathByType(userId, static_cast<KeyType>(EL5_KEY + 1));
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: StorageDaemonTest_RestoreOneUserKey_001
 * @tc.desc: Verify the RestoreOneUserKey when KeyType invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreOneUserKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, static_cast<KeyType>(EL5_KEY + 1)), E_KEY_TYPE_INVALID);
}

/**
 * @tc.name: StorageDaemonTest_RestoreOneUserKey_002
 * @tc.desc: Verify the RestoreOneUserKey when elNeedRestorePath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreOneUserKey_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    auto path = storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL1_KEY);
    std::error_code errCode;
    std::filesystem::remove(path, errCode);

    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, EL1_KEY), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_RestoreOneUserKey_003
 * @tc.desc: Verify the RestoreOneUserKey when RestoreUserKey failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreOneUserKey_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillRepeatedly(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, EL2_KEY), E_MIGRETE_ELX_FAILED);
    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, EL1_KEY), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_RestoreOneUserKey_004
 * @tc.desc: Verify the RestoreOneUserKey when PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreOneUserKey_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, EL1_KEY), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_RestoreOneUserKey_005
 * @tc.desc: Verify the RestoreOneUserKey when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreOneUserKey_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    // userId < START_APP_CLONE_USER_ID
    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId_, EL1_KEY), E_OK);

    // userId > START_APP_CLONE_USER_ID && userId < MAX_APP_CLONE_USER_ID
    int32_t userId = StorageService::START_APP_CLONE_USER_ID + 1;
    CreateNeedRestoreFile(userId, EL2_KEY);
    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId, EL2_KEY), E_OK);
    DeleteNeedRestoreFile(userId, EL2_KEY);

    // userId > MAX_APP_CLONE_USER_ID
    userId = StorageService::MAX_APP_CLONE_USER_ID + 1;
    CreateNeedRestoreFile(userId, EL3_KEY);
    EXPECT_EQ(storageDaemon_->RestoreOneUserKey(userId, EL3_KEY), E_OK);
    DeleteNeedRestoreFile(userId, EL3_KEY);
}

/**
 * @tc.name: StorageDaemonTest_RestoreUserKey_001
 * @tc.desc: Verify the RestoreUserKey when NeedRestorePath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreUserKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::error_code errCode;
    std::filesystem::remove(storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL1_KEY), errCode);
    std::filesystem::remove(storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL2_KEY), errCode);
    std::filesystem::remove(storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL3_KEY), errCode);
    std::filesystem::remove(storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL4_KEY), errCode);
    std::filesystem::remove(storageDaemon_->GetNeedRestoreFilePathByType(userId_, EL5_KEY), errCode);

    EXPECT_EQ(storageDaemon_->RestoreUserKey(userId_, 1), -EEXIST);
}

/**
 * @tc.name: StorageDaemonTest_RestoreUserKey_002
 * @tc.desc: Verify the RestoreUserKey when EL1 or EL2 failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_RestoreUserKey_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    // el1 failed
    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->RestoreUserKey(userId_, 1), E_ERR);

    // el2 failed
    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_ERR)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillRepeatedly(Return(E_OK));
    EXPECT_EQ(storageDaemon_->RestoreUserKey(userId_, 1), E_OK);
}
#endif

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs when GenerateUserKeys failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirs_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
#ifdef USER_CRYPTO_MIGRATE_KEY
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeys(_, _)).WillOnce(Return(-EEXIST)).WillOnce(Return(-EEXIST));
    EXPECT_CALL(*keyManagerMock_, RestoreUserKey(_, _)).WillOnce(Return(E_ERR)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillRepeatedly(Return(E_OK));
    // RestoreUserKey failed
    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_ERR);
    // RestoreUserKey success
    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_OK);
#endif

    EXPECT_CALL(*keyManagerMock_, GenerateUserKeys(_, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_ERR);
#endif
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirs_002
 * @tc.desc: Verify the PrepareUserDirs when PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirs_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeys(_, _)).WillRepeatedly(Return(E_OK));
#endif
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirs_003
 * @tc.desc: Verify the PrepareUserDirs when all operations succeed
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirs_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeys(_, _)).WillRepeatedly(Return(E_OK));
#endif
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillRepeatedly(Return(E_OK));
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerExtMock_, GenerateUserKeys(_, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
#endif
    // KeyManagerExt GenerateUserKeys failed
    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_OK);
    // KeyManagerExt GenerateUserKeys success
    EXPECT_EQ(storageDaemon_->PrepareUserDirs(userId_, 1), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs when DestroyUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_DestroyUserDirs_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*userManagerMock_, DestroyUserDirs(_, _)).WillOnce(Return(E_ERR));
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, DeleteUserKeys(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerExtMock_, DeleteUserKeys(_)).WillOnce(Return(E_OK));
#endif

    EXPECT_EQ(storageDaemon_->DestroyUserDirs(userId_, 1), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_DestroyUserDirs_002
 * @tc.desc: Verify the DestroyUserDirs when DeleteUserKeys failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_DestroyUserDirs_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*userManagerMock_, DestroyUserDirs(_, _)).WillRepeatedly(Return(E_OK));
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, DeleteUserKeys(_)).WillOnce(Return(E_ERR));
#endif

    EXPECT_EQ(storageDaemon_->DestroyUserDirs(userId_, 1), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_DestroyUserDirs_003
 * @tc.desc: Verify the DestroyUserDirs when all operations succeed
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_DestroyUserDirs_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*userManagerMock_, DestroyUserDirs(_, _)).WillRepeatedly(Return(E_OK));
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, DeleteUserKeys(_)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerExtMock_, DeleteUserKeys(_)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
#endif
    // KeyManagerExt DeleteUserKeys failed
    EXPECT_EQ(storageDaemon_->DestroyUserDirs(userId_, 1), E_OK);
    // KeyManagerExt DeleteUserKeys success
    EXPECT_EQ(storageDaemon_->DestroyUserDirs(userId_, 1), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_CompleteAddUser_001
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_CompleteAddUser_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    // userId < START_APP_CLONE_USER_ID
    EXPECT_EQ(storageDaemon_->CompleteAddUser(userId_), E_OK);
    // userId > START_APP_CLONE_USER_ID && userId < MAX_APP_CLONE_USER_ID
    EXPECT_EQ(storageDaemon_->CompleteAddUser(StorageService::START_APP_CLONE_USER_ID + 1), E_OK);
    // userId < MAX_APP_CLONE_USER_ID
    EXPECT_EQ(storageDaemon_->CompleteAddUser(StorageService::MAX_APP_CLONE_USER_ID + 1), E_OK);
    EXPECT_EQ(storageDaemon_->CompleteAddUser(userId_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_InitGlobalKey_001
 * @tc.desc: Verify the InitGlobalKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InitGlobalKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, InitGlobalDeviceKey()).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->InitGlobalKey(), E_ERR);
    EXPECT_EQ(storageDaemon_->InitGlobalKey(), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_InitGlobalUserKeys_001
 * @tc.desc: Verify the InitGlobalUserKeys when el1NeedRestorePath exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InitGlobalUserKeys_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
#ifdef USER_CRYPTO_MIGRATE_KEY
    CreateNeedRestoreFile(StorageService::START_USER_ID, EL1_KEY);

    g_saveStringToFile = false;
    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), false);
    g_saveStringToFile = true;

#endif
    EXPECT_CALL(*keyManagerMock_, InitGlobalUserKeys()).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareAllUserEl1Dirs()).WillOnce(Return(E_OK));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_OK);

#ifdef USER_CRYPTO_MIGRATE_KEY
    DeleteNeedRestoreFile(StorageService::START_USER_ID, EL1_KEY);
#endif
#endif
}

/**
 * @tc.name: StorageDaemonTest_InitGlobalUserKeys_002
 * @tc.desc: Verify the InitGlobalUserKeys when el1NeedRestorePath not exist and InitGlobalUserKeys failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InitGlobalUserKeys_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_FILE_SHARING
    g_setupFileSharingDir = -1;
#endif

#ifdef USER_CRYPTO_MANAGER
#ifdef USER_CRYPTO_MIGRATE_KEY
    auto path = storageDaemon_->GetNeedRestoreFilePath(StorageService::START_USER_ID, USER_EL1_DIR);
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak", errCode);
    }
    
#endif
    EXPECT_CALL(*keyManagerMock_, InitGlobalUserKeys()).WillOnce(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_ERR);

#ifdef USER_CRYPTO_MIGRATE_KEY
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
#endif
#endif
}

/**
 * @tc.name: StorageDaemonTest_InitGlobalUserKeys_003
 * @tc.desc: Verify the InitGlobalUserKeys when PrepareAllUserEl1Dirs or PrepareAppdataDir failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_InitGlobalUserKeys_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
#ifdef USER_CRYPTO_MIGRATE_KEY
    auto path = storageDaemon_->GetNeedRestoreFilePath(StorageService::START_USER_ID, USER_EL1_DIR);
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak", errCode);
    }
    
#endif
    EXPECT_CALL(*keyManagerMock_, InitGlobalUserKeys()).WillRepeatedly(Return(E_OK));
#endif
    EXPECT_CALL(*userManagerMock_, PrepareAllUserEl1Dirs()).WillOnce(Return(E_ERR))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*mountManagerMock_, PrepareAppdataDir(_)).WillOnce(Return(E_ERR))
        .WillOnce(Return(E_ERR)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_ERR);
    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_ERR);
    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_OK);
    EXPECT_EQ(storageDaemon_->InitGlobalUserKeys(), E_OK);

#ifdef USER_CRYPTO_MANAGER
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
#endif
#endif
}

/**
 * @tc.name: StorageDaemonTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateUserKeys_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    uint64_t secureUid = 1;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    EXPECT_CALL(*keyManagerMock_, UpdateUserAuth(_, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->UpdateUserAuth(userId_, secureUid, token_, oldSecret, newSecret), E_ERR);
    EXPECT_EQ(storageDaemon_->UpdateUserAuth(userId_, secureUid, token_, oldSecret, newSecret), E_OK);
#endif
}

#ifdef USER_CRYPTO_MIGRATE_KEY
/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuth_001
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuth  when return UpdateUserAuthOld.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuth_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL1_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL1_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuth(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuth_002
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuth  when return UpdateUserAuthOld.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuth_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL1_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "4";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL1_KEY), "4");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuth(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_001
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when KeyType invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto ret = storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_,
        static_cast<KeyType>(EL5_KEY + 1), token_, secret_);
    EXPECT_EQ(ret, E_KEY_TYPE_INVALID);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_002
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when
    ActiveCeSceSeceUserKey failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR));
    
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_003
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when UpdateCeEceSeceUserAuth failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillRepeatedly(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_ERR);
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_004
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when UpdateCeEceSeceKeyContext failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_005
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_006
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_006, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_OK);
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL2_KEY, token_, secret_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_007
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthOld when ActiveCeSceSeceUserKey failed with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthOld_007, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ACTIVE_REPEATED));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_OK));
    
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthOld(userId_, EL1_KEY, token_, secret_), E_OK);
}


/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_001
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx when KeyType invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto ret = storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_,
        static_cast<KeyType>(EL5_KEY + 1), token_, secret_, "");
    EXPECT_EQ(ret, E_KEY_TYPE_INVALID);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_002
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx when ActiveCeSceSeceUserKey failed
    with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_ERR));

    // new_need_restore not equal UPDATE_V4
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL1_KEY, token_, secret_, "1"), E_ERR);

    // new_need_restore equal UPDATE_V4
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL1_KEY, token_, secret_, "3"), E_ERR);

    // new_need_restore equal UPDATE_V4 and SaveStringToFileSync failed
    g_saveStringToFileSync = false;
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL1_KEY, token_, secret_, "3"), E_ERR);

    // new_need_restore equal UPDATE_V4 and SaveStringToFileSync success
    g_saveStringToFileSync = true;
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL1_KEY, token_, secret_, "3"), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_003
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx when NatoNeedRestorePath exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL3_KEY, token_, secret_, "1"), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_004
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx
    when NatoNeedRestorePath not exist and PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    auto natoPath = KeyManager::GetInstance().GetNatoNeedRestorePath(userId_, EL3_KEY) + FSCRYPT_VERSION_DIR;
    std::error_code errCode;
    std::filesystem::remove(natoPath, errCode);
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL3_KEY, token_, secret_, "1"), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_005
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
    auto el2NatoPath = KeyManager::GetInstance().GetNatoNeedRestorePath(userId_, EL2_KEY) + FSCRYPT_VERSION_DIR;
    std::error_code errCode;
    std::filesystem::remove(el2NatoPath, errCode);
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL3_KEY, token_, secret_, "1"), E_OK);
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL2_KEY, token_, secret_, "1"), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_006
 * @tc.desc: Verify the PrepareUserDirsAndUpdateUserAuthVx when ActiveCeSceSeceUserKey failed with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateUserAuthVx_006, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ACTIVE_REPEATED));
    auto el2NatoPath = KeyManager::GetInstance().GetNatoNeedRestorePath(userId_, EL2_KEY) + FSCRYPT_VERSION_DIR;
    std::error_code errCode;
    std::filesystem::remove(el2NatoPath, errCode);
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateUserAuthVx(userId_, EL1_KEY, token_, secret_, "1"), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_001
 * @tc.desc: Verify the PrepareUserDirsAndUpdateAuth4Nato when NatoNeedRestorePath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    auto natoPath = KeyManager::GetInstance().GetNatoNeedRestorePath(userId_, EL3_KEY) + RESTORE_DIR;
    std::error_code errCode;
    std::filesystem::remove(natoPath, errCode);

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, EL3_KEY, token_), E_KEY_TYPE_INVALID);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_002
 * @tc.desc: Verify the PrepareUserDirsAndUpdateAuth4Nato when ActiveElxUserKey4Nato failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_ERR));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, EL3_KEY, token_), E_ERR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_003
 * @tc.desc: Verify the PrepareUserDirsAndUpdateAuth4Nato when KeyType invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    std::error_code errCode;
    std::filesystem::create_directories("." + std::string(RESTORE_DIR), errCode);
    EXPECT_CALL(*keyManagerMock_, GetNatoNeedRestorePath(_, _)).WillOnce(Return("."));
    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillOnce(Return(E_OK));

    auto ret = storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, static_cast<KeyType>(EL5_KEY + 1), token_);
    EXPECT_EQ(ret, E_KEY_TYPE_INVALID);

    std::filesystem::remove_all("./latest", errCode);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_004
 * @tc.desc: Verify the PrepareUserDirsAndUpdateAuth4Nato whena PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_ERR));

    auto ret = storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, EL3_KEY, token_);
    EXPECT_EQ(ret, E_NATO_PREPARE_USER_DIR_ERROR);
}

/**
 * @tc.name: StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_005
 * @tc.desc: Verify the PrepareUserDirsAndUpdateAuth4Nato whena all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_PrepareUserDirsAndUpdateAuth4Nato_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

    EXPECT_CALL(*keyManagerMock_, ActiveElxUserKey4Nato(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, EL3_KEY, token_), E_OK);
    EXPECT_EQ(storageDaemon_->PrepareUserDirsAndUpdateAuth4Nato(userId_, EL2_KEY, token_), E_OK);
}

/**
 * @tc.name: StorageDaemonTest_IsNeedRestorePathExist_001
 * @tc.desc: Verify the IsNeedRestorePathExist function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_IsNeedRestorePathExist_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
    EXPECT_EQ(storageDaemon_->IsNeedRestorePathExist(userId_, true), true);
    EXPECT_EQ(storageDaemon_->IsNeedRestorePathExist(userId_, false), true);
}
#endif

/**
 * @tc.name: StorageDaemonTest_GenerateKeyAndPrepareUserDirs_001
 * @tc.desc: Verify the GenerateKeyAndPrepareUserDirs function when GenerateUserKeyByType failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateKeyAndPrepareUserDirs_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL1_KEY, token_, secret_), E_ERR);
#endif
}

/**
 * @tc.name: StorageDaemonTest_GenerateKeyAndPrepareUserDirs_002
 * @tc.desc: Verify the GenerateKeyAndPrepareUserDirs function when KeyType invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateKeyAndPrepareUserDirs_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillOnce(Return(E_OK));
    auto ret = storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_,
        static_cast<KeyType>(EL5_KEY + 1), token_, secret_);
    EXPECT_EQ(ret, E_KEY_TYPE_INVALID);
#endif
}

/**
 * @tc.name: StorageDaemonTest_GenerateKeyAndPrepareUserDirs_003
 * @tc.desc: Verify the GenerateKeyAndPrepareUserDirs function when type is not el5 or
    uece has not already create.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateKeyAndPrepareUserDirs_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));

    // type is not el5
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL1_KEY, token_, secret_), E_OK);

    // keyUeceDir not exist
    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(userId_);
    std::error_code errCode;
    if (std::filesystem::exists(keyUeceDir, errCode)) {
        std::filesystem::rename(keyUeceDir, keyUeceDir + "_bak", errCode);
    }
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL5_KEY, token_, secret_), E_OK);
    if (std::filesystem::exists(keyUeceDir + "_bak", errCode)) {
        std::filesystem::rename(keyUeceDir + "_bak", keyUeceDir, errCode);
    }

    // keyUeceDir exist but empty
    if (std::filesystem::exists(keyUeceDir, errCode)) {
        std::filesystem::rename(keyUeceDir, keyUeceDir + "_bak", errCode);
    }
    std::filesystem::create_directories(keyUeceDir);
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL5_KEY, token_, secret_), E_OK);
    std::filesystem::remove_all(keyUeceDir);
    if (std::filesystem::exists(keyUeceDir + "_bak", errCode)) {
        std::filesystem::rename(keyUeceDir + "_bak", keyUeceDir, errCode);
    }
#endif
}

/**
 * @tc.name: StorageDaemonTest_GenerateKeyAndPrepareUserDirs_004
 * @tc.desc: Verify the GenerateKeyAndPrepareUserDirs function when uece has already create.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateKeyAndPrepareUserDirs_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillRepeatedly(Return(E_OK));

    std::string keyUeceDir = std::string(UECE_DIR) + "/" + std::to_string(userId_);
    std::error_code errCode;
    if (std::filesystem::exists(keyUeceDir, errCode)) {
        std::filesystem::rename(keyUeceDir, keyUeceDir + "_bak", errCode);
    }
    std::filesystem::create_directories(keyUeceDir + "/test");
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string el0NeedRestore = std::string(DATA_SERVICE_EL0_STORAGE_DAEMON_SD) + NEED_RESTORE_SUFFIX;
    if (std::filesystem::exists(el0NeedRestore, errCode)) {
        std::filesystem::rename(el0NeedRestore, el0NeedRestore + "_bak", errCode);
    }
    std::ofstream file(el0NeedRestore);
    file.close();
#endif
    // el0NeedRestore exist
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL5_KEY, token_, secret_), E_OK);

#ifdef USER_CRYPTO_MIGRATE_KEY
    std::filesystem::remove(el0NeedRestore, errCode);
#endif
    // el0NeedRestore not exist
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL5_KEY, token_, secret_), E_OK);

#ifdef USER_CRYPTO_MIGRATE_KEY
    if (std::filesystem::exists(el0NeedRestore + "_bak", errCode)) {
        std::filesystem::rename(el0NeedRestore + "_bak", el0NeedRestore, errCode);
    }
#endif
    std::filesystem::remove_all(keyUeceDir);
    if (std::filesystem::exists(keyUeceDir + "_bak", errCode)) {
        std::filesystem::rename(keyUeceDir + "_bak", keyUeceDir, errCode);
    }
#endif
}

/**
 * @tc.name: StorageDaemonTest_GenerateKeyAndPrepareUserDirs_005
 * @tc.desc: Verify the GenerateKeyAndPrepareUserDirs function when PrepareUserDirs failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_GenerateKeyAndPrepareUserDirs_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->GenerateKeyAndPrepareUserDirs(userId_, EL1_KEY, token_, secret_), E_ERR);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepare_001
 * @tc.desc: Verify the ActiveUserKeyAndPrepare when ActiveCeSceSeceUserKey success.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepare_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepare_002
 * @tc.desc: Verify the ActiveUserKeyAndPrepare when ActiveCeSceSeceUserKey failed with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepare_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ACTIVE_REPEATED));

    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ACTIVE_REPEATED);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepare_003
 * @tc.desc: Verify the ActiveUserKeyAndPrepare when ActiveCeSceSeceUserKey failed with -ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepare_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(-ENOENT));
    EXPECT_CALL(*keyManagerMock_, GenerateUserKeyByType(_, _, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillOnce(Return(E_OK));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ERR);
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepare_004
 * @tc.desc: Verify the ActiveUserKeyAndPrepare when ActiveCeSceSeceUserKey failed
    with not E_ACTIVE_REPEATED and -ENOENT and elNeedRestorePath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepare_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);

#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_ERR));
    // token is empty, secret is empty
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ERR);
 
#ifdef USER_CRYPTO_MIGRATE_KEY
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL1_KEY) + RESTORE_DIR;
    std::error_code errCode;
    std::filesystem::remove(path, errCode);
 
    secret_.push_back(1);
    // token is empty, secret is not empty
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ERR);
 
    secret_.clear();
    token_.push_back(1);
    // token is empty but secret is not empty
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ERR);
 
    secret_.push_back(1);
    // token is not empty, secret is not empty
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_ERR);
#endif
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepare_005
 * @tc.desc: Verify the ActiveUserKeyAndPrepare when ActiveCeSceSeceUserKey failed
    with not E_ACTIVE_REPEATED and -ENOENT and elNeedRestorePath exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepare_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    token_.push_back(1);
    secret_.push_back(1);
#ifdef USER_CRYPTO_MIGRATE_KEY
    auto path = KeyManager::GetInstance().GetKeyDirByUserAndType(userId_, EL1_KEY) + RESTORE_DIR;
    std::ofstream file(path);
    file << "1";
    file.close();
    EXPECT_EQ(storageDaemon_->GetNeedRestoreVersion(userId_, EL1_KEY), "1");
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR)).WillOnce(Return(E_OK));
    EXPECT_CALL(*iamClientMock_, GetSecureUid(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceUserAuth(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, UpdateCeEceSeceKeyContext(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*userManagerMock_, PrepareUserDirs(_, _)).WillRepeatedly(Return(E_OK));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepare(userId_, EL1_KEY, token_, secret_), E_OK);
#endif
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepareElX_001
 * @tc.desc: Verify the ActiveUserKeyAndPrepareElX when EL3_KEY failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepareElX_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_ERR));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepareElX(userId_, token_, secret_), E_ERR);
#endif
}
 
/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepareElX_002
 * @tc.desc: Verify the ActiveUserKeyAndPrepareElX when EL4_KEY failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepareElX_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepareElX(userId_, token_, secret_), E_ERR);
#endif
}
 
/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepareElX_003
 * @tc.desc: Verify the ActiveUserKeyAndPrepareElX when EL5_KEY failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepareElX_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepareElX(userId_, token_, secret_), E_ERR);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepareElX_004
 * @tc.desc: Verify the ActiveUserKeyAndPrepareElX when EL345_KEY failed with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepareElX_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_ACTIVE_REPEATED));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepareElX(userId_, token_, secret_), E_ACTIVE_REPEATED);
#endif
}
 
/**
 * @tc.name: StorageDaemonTest_ActiveUserKeyAndPrepareElX_005
 * @tc.desc: Verify the ActiveUserKeyAndPrepareElX when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKeyAndPrepareElX_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_OK));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKeyAndPrepareElX(userId_, token_, secret_), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Single_001
 * @tc.desc: Verify the ActiveUserKey4Single when ActiveCeSceSeceUserKey and ActiveUserKeyAndPrepareElX failed
    with E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Single_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).Times(4)
        .WillRepeatedly(Return(E_ACTIVE_REPEATED));
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));
 
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Single_002
 * @tc.desc: Verify the ActiveUserKey4Single when ActiveCeSceSeceUserKey failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Single_002, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillRepeatedly(Return(E_ERR));

    // token is empty and secret is empty
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);

    // token not empty and secret is empty
    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);

    // token is empty and secret not empty
    token_.clear();
    secret_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);

    // token not empty and secret not empty
    token_.push_back(1);
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_ACTIVE_EL2_FAILED);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Single_003
 * @tc.desc: Verify the ActiveUserKey4Single when ActiveUserKeyAndPrepareElX failed with not E_ACTIVE_REPEATED.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Single_003, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_ERR));
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));

    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_ERR);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Single_004
 * @tc.desc: Verify the ActiveUserKey4Single when NotifyUeceActivation failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Single_004, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).Times(4)
        .WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_ERR));
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_UNLOCK_APP_KEY2_FAILED);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveUserKey4Single_005
 * @tc.desc: Verify the ActiveUserKey4Single when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveUserKey4Single_005, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).Times(4)
        .WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*keyManagerMock_, NotifyUeceActivation(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(storageDaemon_->ActiveUserKey4Single(userId_, token_, secret_), E_OK);
#endif
}

/**
 * @tc.name: StorageDaemonTest_ActiveAppCloneUserKey_001
 * @tc.desc: Verify the ActiveAppCloneUserKey.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonTest, StorageDaemonTest_ActiveAppCloneUserKey_001, TestSize.Level1)
{
    ASSERT_TRUE(storageDaemon_ != nullptr);
 
#ifdef USER_CRYPTO_MANAGER
    CreateNeedRestoreFile(StorageService::START_APP_CLONE_USER_ID, EL1_KEY);

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_NOT_SUPPORT));
    storageDaemon_->ActiveAppCloneUserKey();

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_NOT_SUPPORT));
    storageDaemon_->ActiveAppCloneUserKey();

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK))
        .WillOnce(Return(E_NOT_SUPPORT));
    storageDaemon_->ActiveAppCloneUserKey();

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _))
        .WillOnce(Return(E_ACTIVE_REPEATED)).WillOnce(Return(E_ACTIVE_REPEATED)).WillOnce(Return(E_ACTIVE_REPEATED));
    storageDaemon_->ActiveAppCloneUserKey();

    EXPECT_CALL(*keyManagerMock_, ActiveCeSceSeceUserKey(_, _, _, _))
        .WillOnce(Return(E_OK)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
    storageDaemon_->ActiveAppCloneUserKey();

    DeleteNeedRestoreFile(StorageService::START_APP_CLONE_USER_ID, EL1_KEY);
#endif
}
} // Test
} // STORAGE_DAEMON
} // OHOS