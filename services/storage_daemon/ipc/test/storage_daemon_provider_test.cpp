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

#include "ipc/storage_daemon_provider.h"
#include "message_parcel.h"
#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "system_ability_definition.h"
#include "test/common/help_utils.h"
#include "userdata_dir_info.h"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;
class StorageDaemonProviderTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    StorageDaemonProvider *storageDaemonProviderTest_;
};

std::vector<uint8_t> GenerateTestVector(uint8_t startValue, size_t length)
{
    std::vector<uint8_t> result(length);
    for (size_t i = 0; i < length; ++i) {
        result[i] = startValue + static_cast<uint8_t>(i);
    }
    return result;
}

std::vector<uint8_t> GenerateTestData(uint8_t startValue, size_t length)
{
    std::vector<uint8_t> result(length);
    for (size_t i = 0; i < length; ++i) {
        result[i] = startValue + static_cast<uint8_t>(i);
    }
    return result;
}

void StringVecToRawData(const std::vector<std::string> &stringVec, StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));

    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

void StorageDaemonProviderTest::SetUp(void)
{
    storageDaemonProviderTest_ = new StorageDaemonProvider();
    StorageTest::StorageTestUtils::ClearTestResource();
}

void StorageDaemonProviderTest::TearDown(void)
{
    StorageTest::StorageTestUtils::ClearTestResource();
    if (storageDaemonProviderTest_ != nullptr) {
        delete storageDaemonProviderTest_;
        storageDaemonProviderTest_ = nullptr;
    }
}

/**
 * @tc.name: StorageDaemonProviderTest_Shutdown_001
 * @tc.desc: Verify the Shutdown function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Shutdown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Shutdown_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->Shutdown();
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Shutdown_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Mount_001
 * @tc.desc: Verify the Mount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Mount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Mount_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-1";
    uint32_t flag = 0;
    auto ret = storageDaemonProviderTest_->Mount(volId, flag);
    EXPECT_NE(ret, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Mount_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMount_001
 * @tc.desc: Verify the UMount function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMount_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-2";
    auto ret = storageDaemonProviderTest_->UMount(volId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMount_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Check_001
 * @tc.desc: Verify the Check function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Check_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Check_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-3";
    auto ret = storageDaemonProviderTest_->Check(volId);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Check_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Format_001
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Format_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Format_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-4";
    std::string fsType = "exfat";
    auto ret = storageDaemonProviderTest_->Format(volId, fsType);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Format_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Format_002
 * @tc.desc: Verify the Format function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Format_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Format_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string emptyVolId = "";
    const std::string testFsType = "ext4";
    auto ret = storageDaemonProviderTest_->Format(emptyVolId, testFsType);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Format_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Partition_001
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Partition_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Partition_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string diskId = "disk-1-0";
    int32_t type = 0;
    auto ret = storageDaemonProviderTest_->Partition(diskId, type);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Partition_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_Partition_002
 * @tc.desc: Verify the Partition function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_Partition_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Partition_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string emptyDiskId = "";
    const int32_t testType = 1;
    auto ret = storageDaemonProviderTest_->Partition(emptyDiskId, testType);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_Partition_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetVolumeDescription_001
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetVolumeDescription_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-5";
    std::string des = "des";
    auto ret = storageDaemonProviderTest_->SetVolumeDescription(volId, des);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetVolumeDescription_002
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetVolumeDescription_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string emptyVolId = "";
    const std::string testDesc = "Empty Volume";
    auto ret = storageDaemonProviderTest_->SetVolumeDescription(emptyVolId, testDesc);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetVolumeDescription_003
 * @tc.desc: Verify the SetVolumeDescription function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetVolumeDescription_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string testVolId = "vol-empty-desc-003";
    const std::string emptyDesc = "";
    auto ret = storageDaemonProviderTest_->SetVolumeDescription(testVolId, emptyDesc);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetVolumeDescription_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_QueryUsbIsInUse_001
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_QueryUsbIsInUse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string diskPath = "";
    bool isInUse = true;
    auto ret = storageDaemonProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_QueryUsbIsInUse_002
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_QueryUsbIsInUse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string diskPath = "/..";
    bool isInUse = true;
    auto ret = storageDaemonProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_QueryUsbIsInUse_002
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_QueryUsbIsInUse_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string diskPath = "";
    bool isInUse = false;
    auto ret = storageDaemonProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_QueryUsbIsInUse_004
 * @tc.desc: Verify the QueryUsbIsInUse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_QueryUsbIsInUse_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_004 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string diskPath = "/..";
    bool isInUse = false;
    auto ret = storageDaemonProviderTest_->QueryUsbIsInUse(diskPath, isInUse);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_QueryUsbIsInUse_004 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_StartUser_001
 * @tc.desc: Verify the StartUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StartUser_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t ret = storageDaemonProviderTest_->StartUser(StorageService::START_USER_ID - 1);
    EXPECT_TRUE(ret == E_USERID_RANGE) << "user's dirs are not prepare";
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StartUser_001 end";
}

/**
 * @tc.name: Storage_Manager_StorageDaemonProviderTest_StartUser_002
 * @tc.desc: check the StartUser function when args are normal
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_StartUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StartUser_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);

    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = storageDaemonProviderTest_->PrepareUserDirs(StorageTest::USER_ID5, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = storageDaemonProviderTest_->StartUser(StorageTest::USER_ID5);
    EXPECT_TRUE(ret == E_OK) << "check StartUser";

    storageDaemonProviderTest_->StopUser(StorageTest::USER_ID5);
    storageDaemonProviderTest_->DestroyUserDirs(StorageTest::USER_ID5, flags);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StartUser_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_StopUser_001
 * @tc.desc: Verify the StopUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StopUser_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->StopUser(StorageService::START_USER_ID - 1);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StopUser_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_StopUser_002
 * @tc.desc: check the StopUser function when dir does not exist.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_StopUser_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StopUser_002 start";

    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2;
    auto ret = storageDaemonProviderTest_->PrepareUserDirs(StorageTest::USER_ID1, flags);
    EXPECT_TRUE(ret == E_OK) << "create user dirs error";

    ret = storageDaemonProviderTest_->StartUser(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_OK) << "user's dirs are not prepare";

    ret = storageDaemonProviderTest_->StopUser(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_OK) << "dir is not mount";

    ret = storageDaemonProviderTest_->DestroyUserDirs(StorageTest::USER_ID1, flags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StopUser_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_PrepareUserDirs_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const uint32_t testFlags = 0;
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->PrepareUserDirs(StorageTest::USER_ID1, testFlags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_PrepareUserDirs_002
 * @tc.desc: Verify the PrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_PrepareUserDirs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_PrepareUserDirs_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const uint32_t testFlags = -1;
    auto ret = storageDaemonProviderTest_->PrepareUserDirs(StorageService::START_USER_ID - 1, testFlags);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_PrepareUserDirs_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DestroyUserDirs_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const uint32_t testFlags = 0x02;
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->DestroyUserDirs(StorageTest::USER_ID1, testFlags);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DestroyUserDirs_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DestroyUserDirs_002
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DestroyUserDirs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DestroyUserDirs_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const uint32_t testFlags = -1;
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->DestroyUserDirs(StorageService::START_USER_ID - 1, testFlags);
    EXPECT_NE(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DestroyUserDirs_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CompleteAddUser_001
 * @tc.desc: Verify the CompleteAddUser function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CompleteAddUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CompleteAddUser_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->CompleteAddUser(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CompleteAddUser_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InitGlobalKey_001
 * @tc.desc: Verify the InitGlobalKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InitGlobalKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InitGlobalKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t ret = storageDaemonProviderTest_->InitGlobalKey();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InitGlobalKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InitGlobalUserKeys_001
 * @tc.desc: Verify the InitGlobalUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InitGlobalUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InitGlobalUserKeys_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t ret = storageDaemonProviderTest_->InitGlobalUserKeys();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InitGlobalUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DeleteUserKeys_001
 * @tc.desc: Verify the DeleteUserKeys function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteUserKeys_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->DeleteUserKeys(StorageTest::USER_ID1);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteUserKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_EraseAllUserEncryptedKeys_001
 * @tc.desc: Verify the EraseAllUserEncryptedKeys function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_EraseAllUserEncryptedKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_EraseAllUserEncryptedKeys_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->EraseAllUserEncryptedKeys();
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_EraseAllUserEncryptedKeys_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateUserAuth_001
 * @tc.desc: Verify the UpdateUserAuth function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUserAuth_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint64_t secureUid = 123456789;
    std::vector<uint8_t> token = {0x01, 0x02, 0x03};
    std::vector<uint8_t> oldSecret = {0x04, 0x05, 0x06};
    std::vector<uint8_t> newSecret = {0x07, 0x08, 0x09};
    int32_t ret =
        storageDaemonProviderTest_->UpdateUserAuth(StorageTest::USER_ID1, secureUid, token, oldSecret, newSecret);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUserAuth_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_001
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto authToken = GenerateTestVector(0x01, 32);
    auto newSecret = GenerateTestVector(0x02, 32);
    uint64_t secureUid = 12345678901234;
    std::vector<std::vector<uint8_t>> plainText = {GenerateTestVector(0x03, 64), GenerateTestVector(0x04, 64)};
    int32_t ret = storageDaemonProviderTest_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid,
                                                                           StorageTest::USER_ID1, plainText);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_ActiveUserKey_001
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_ActiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ActiveUserKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto token = GenerateTestData(0x01, 32);
    auto secret = GenerateTestData(0x02, 32);
    int32_t ret = storageDaemonProviderTest_->ActiveUserKey(StorageTest::USER_ID1, token, secret);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ActiveUserKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_ActiveUserKey_002
 * @tc.desc: Verify the ActiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_ActiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ActiveUserKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    int32_t ret = storageDaemonProviderTest_->ActiveUserKey(StorageService::START_USER_ID - 1, token, secret);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ActiveUserKey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InactiveUserKey_001
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InactiveUserKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t ret = storageDaemonProviderTest_->InactiveUserKey(StorageTest::USER_ID1);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InactiveUserKey_002
 * @tc.desc: Verify the InactiveUserKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InactiveUserKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = storageDaemonProviderTest_->InactiveUserKey(StorageService::START_USER_ID - 1);
    EXPECT_EQ(ret, E_USERID_RANGE);
#endif
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserKey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateKeyContext_001
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateKeyContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateKeyContext_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool needRemoveTmpKey = true;
    int32_t result = storageDaemonProviderTest_->UpdateKeyContext(StorageTest::USER_ID1, needRemoveTmpKey);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateKeyContext_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateKeyContext_002
 * @tc.desc: Verify the UpdateKeyContext function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateKeyContext_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateKeyContext_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool needRemoveTmpKey = true;
    uint32_t secureUid = -1;
    int32_t result = storageDaemonProviderTest_->UpdateKeyContext(secureUid, needRemoveTmpKey);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateKeyContext_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountCryptoPathAgain_001
 * @tc.desc: Verify the MountCryptoPathAgain function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountCryptoPathAgain_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountCryptoPathAgain_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t userId = 123;
    int32_t result = storageDaemonProviderTest_->MountCryptoPathAgain(userId);
    EXPECT_EQ(result, -ENOENT);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountCryptoPathAgain_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_LockUserScreen_001
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_LockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_LockUserScreen_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t result = storageDaemonProviderTest_->LockUserScreen(StorageTest::USER_ID1);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_LockUserScreen_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_LockUserScreen_002
 * @tc.desc: Verify the LockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_LockUserScreen_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_LockUserScreen_002 start";
#ifdef USER_CRYPTO_MANAGER
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t ret = storageDaemonProviderTest_->LockUserScreen(StorageService::START_USER_ID - 1);
    EXPECT_EQ(ret, E_USERID_RANGE);
#endif
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_LockUserScreen_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UnlockUserScreen_001
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UnlockUserScreen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UnlockUserScreen_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<uint8_t> token = {0x01, 0x02, 0x03};
    std::vector<uint8_t> secret = {0xAA, 0xBB, 0xCC};
    int32_t result = storageDaemonProviderTest_->UnlockUserScreen(StorageTest::USER_ID1, token, secret);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UnlockUserScreen_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UnlockUserScreen_002
 * @tc.desc: Verify the UnlockUserScreen function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UnlockUserScreen_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UnlockUserScreen_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
#ifdef USER_CRYPTO_MANAGER
    std::vector<uint8_t> token = {-1, -1, -1, -1};
    std::vector<uint8_t> secret = {-1, -1, -1};
    int32_t ret = storageDaemonProviderTest_->UnlockUserScreen(StorageService::START_USER_ID - 1, token, secret);
    EXPECT_EQ(ret, E_OK);
#endif
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UnlockUserScreen_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetLockScreenStatus_001
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetLockScreenStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetLockScreenStatus_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool lockStatus = true;
    int32_t result = storageDaemonProviderTest_->GetLockScreenStatus(StorageTest::USER_ID1, lockStatus);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetLockScreenStatus_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GenerateAppkey_001
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GenerateAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GenerateAppkey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string keyId;
    uint32_t hashId = 1234;
    bool needreset = false;
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t result = storageDaemonProviderTest_->GenerateAppkey(StorageTest::USER_ID1, hashId, keyId, needreset);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GenerateAppkey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GenerateAppkey_002
 * @tc.desc: Verify the GenerateAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GenerateAppkey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GenerateAppkey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string keyId;
    uint32_t hashId = 1234;
    uint32_t secureUid = -1;
    bool needreset = false;
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t result = storageDaemonProviderTest_->GenerateAppkey(secureUid, hashId, keyId, needreset);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GenerateAppkey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DeleteAppkey_001
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DeleteAppkey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteAppkey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string keyId;
    int32_t result = storageDaemonProviderTest_->DeleteAppkey(StorageTest::USER_ID1, keyId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteAppkey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DeleteAppkey_002
 * @tc.desc: Verify the DeleteAppkey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DeleteAppkey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteAppkey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string keyId;
    uint32_t secureUid = -1;
    int32_t result = storageDaemonProviderTest_->DeleteAppkey(secureUid, keyId);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteAppkey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateRecoverKey_001
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateRecoverKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t userType = 1;
    const std::vector<uint8_t> token = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

    const std::vector<uint8_t> secret = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                         0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
                                         0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30};

    int32_t result = storageDaemonProviderTest_->CreateRecoverKey(StorageTest::USER_ID1, userType, token, secret);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateRecoverKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateRecoverKey_002
 * @tc.desc: Verify the CreateRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateRecoverKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateRecoverKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t userType = 1;
    uint32_t secureUid = -1;
    const std::vector<uint8_t> token = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10};

    const std::vector<uint8_t> secret = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
                                         0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
                                         0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30};

    int32_t result = storageDaemonProviderTest_->CreateRecoverKey(secureUid, userType, token, secret);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateRecoverKey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetRecoverKey_001
 * @tc.desc: Verify the SetRecoverKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetRecoverKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetRecoverKey_001 start";
    const std::vector<uint8_t> testKey_32 = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xFE, 0xDC, 0xBA,
                                             0x98, 0x76, 0x54, 0x32, 0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
                                             0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t result = storageDaemonProviderTest_->SetRecoverKey(testKey_32);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetRecoverKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateShareFile_001
 * @tc.desc: Verify the CreateShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 0;
    uint32_t flag = 0;
    std::vector<int32_t> funcResult;
    int32_t ret = storageDaemonProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    ASSERT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateShareFile_002
 * @tc.desc: Verify the CreateShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<std::string> uriStrVec;
    for (int i = 0; i < 200000; ++i) {
        std::string uriStr = "/data/fileshareX/file_" + std::to_string(i) + ".txt";
        uriStrVec.push_back(uriStr);
    }
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 0;
    uint32_t flag = 0;
    std::vector<int32_t> funcResult;
    int32_t ret = storageDaemonProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    ASSERT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateShareFile_003
 * @tc.desc: Verify the CreateShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateShareFile_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<std::string> uriStrVec;
    for (int i = 0; i < 200001; ++i) {
        std::string uriStr = "/data/fileshareX/file_" + std::to_string(i) + ".txt";
        uriStrVec.push_back(uriStr);
    }
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 0;
    uint32_t flag = 0;
    std::vector<int32_t> funcResult;
    int32_t ret = storageDaemonProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    ASSERT_EQ(ret, ERR_DEAD_OBJECT);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CreateShareFile_004
 * @tc.desc: Verify the CreateShareFile function when the data is invalid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CreateShareFile_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_004 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<uint8_t> binaryData = {3, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0};
    StorageFileRawData fileRawData {
        .size = 7,
        .data = binaryData.data(),
    };
    uint32_t tokenId = 0;
    uint32_t flag = 0;
    std::vector<int32_t> funcResult;

    int32_t ret = storageDaemonProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    EXPECT_EQ(ret, ERR_DEAD_OBJECT);

    fileRawData.size = 9;
    ret = storageDaemonProviderTest_->CreateShareFile(fileRawData, tokenId, flag, funcResult);
    EXPECT_EQ(ret, ERR_DEAD_OBJECT);

    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CreateShareFile_004 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetUserStatistics_001
 * @tc.desc: Verify the SetUserStatistics function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetUserStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetUserStatistics_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = 100;
    std::map<uint32_t, RadarStatisticInfo> opStatisticsTemp;
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::KEY_LOAD_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].keyLoadSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::KEY_LOAD_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].keyLoadFailCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::KEY_UNLOAD_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].keyUnloadSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::KEY_UNLOAD_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].keyUnloadFailCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_ADD_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userAddSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_ADD_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userAddFailCount, 1);

    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetUserStatistics_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetUserStatistics_002
 * @tc.desc: Verify the SetUserStatistics function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetUserStatistics_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetUserStatistics_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = 100;
    std::map<uint32_t, RadarStatisticInfo> opStatisticsTemp;
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_REMOVE_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userRemoveSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_REMOVE_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userRemoveFailCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_START_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userStartSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_START_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userStartFailCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_STOP_SUCCESS);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userStopSuccCount, 1);

    storageDaemonProviderTest_->SetUserStatistics(userId, RadarStatisticInfoType::USER_STOP_FAIL);
    opStatisticsTemp.clear();
    storageDaemonProviderTest_->GetTempStatistics(opStatisticsTemp);
    ASSERT_TRUE(opStatisticsTemp.find(userId) != opStatisticsTemp.end());
    EXPECT_EQ(opStatisticsTemp[userId].userStopFailCount, 1);

    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetUserStatistics_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_DeleteShareFile_001
 * @tc.desc: Verify the DeleteShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_DeleteShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteShareFile_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string uriStr = "file1";
    std::vector<std::string> uriStrVec = {uriStr};
    StorageFileRawData rawData;
    StringVecToRawData(uriStrVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    uint32_t tokenId = 0;
    int32_t ret = storageDaemonProviderTest_->DeleteShareFile(tokenId, fileRawData);

    EXPECT_EQ(ret, -EINVAL);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_DeleteShareFile_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetBundleQuota_001
 * @tc.desc: Verify the SetBundleQuota function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetBundleQuota_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetBundleQuota_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string bundleDataDirPath = "/data/data/com.example.app";
    int32_t limitSizeMb = 100;

    int32_t result =
        storageDaemonProviderTest_->SetBundleQuota(StorageTest::USER_ID1, bundleDataDirPath, limitSizeMb);
    EXPECT_NE(result, E_ERR);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetBundleQuota_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetOccupiedSpace_001
 * @tc.desc: Verify the GetOccupiedSpace function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetOccupiedSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetOccupiedSpace_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t idType = 1;
    int32_t id = 1000;
    int64_t size = 0;

    int32_t result = storageDaemonProviderTest_->GetOccupiedSpace(idType, id, size);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetOccupiedSpace_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateMemoryPara_001
 * @tc.desc: Verify the UpdateMemoryPara function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateMemoryPara_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateMemoryPara_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t size = 1;
    int32_t oldSize = 2;
    int32_t result = storageDaemonProviderTest_->UpdateMemoryPara(size, oldSize);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateMemoryPara_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/documents";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    int32_t result = storageDaemonProviderTest_->MountDfsDocs(StorageTest::USER_ID1, relativePath, networkId, deviceId);
    EXPECT_NE(result, E_ERR);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountDfsDocs_001
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/documents";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    int32_t result =
        storageDaemonProviderTest_->UMountDfsDocs(StorageTest::USER_ID1, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetFileEncryptStatus_001
 * @tc.desc: Verify the GetFileEncryptStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetFileEncryptStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetFileEncryptStatus_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool isEncrypted = false;
    bool needCheckDirMount = true;
    int32_t result =
        storageDaemonProviderTest_->GetFileEncryptStatus(StorageTest::USER_ID1, isEncrypted, needCheckDirMount);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetFileEncryptStatus_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetUserNeedActiveStatus_001
 * @tc.desc: Verify the GetUserNeedActiveStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetUserNeedActiveStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetUserNeedActiveStatus_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool needActive = true;
    int32_t result = storageDaemonProviderTest_->GetUserNeedActiveStatus(StorageTest::USER_ID1, needActive);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetUserNeedActiveStatus_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetUserNeedActiveStatus_002
 * @tc.desc: Verify the GetUserNeedActiveStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetUserNeedActiveStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetUserNeedActiveStatus_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool needActive = true;
    uint32_t secureUid = -1;
    int32_t result = storageDaemonProviderTest_->GetUserNeedActiveStatus(secureUid, needActive);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetUserNeedActiveStatus_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountMediaFuse_001
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountMediaFuse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t devFd = 1001;
    int32_t result = storageDaemonProviderTest_->MountMediaFuse(StorageTest::USER_ID1, devFd);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountMediaFuse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountMediaFuse_001
 * @tc.desc: Verify the UMountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountMediaFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountMediaFuse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t result = storageDaemonProviderTest_->UMountMediaFuse(StorageTest::USER_ID1);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountMediaFuse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountFileMgrFuse_001
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountFileMgrFuse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "";
    int32_t fuseFd = 0;
    testing::internal::CaptureStderr();
    int32_t result = storageDaemonProviderTest_->MountFileMgrFuse(StorageTest::USER_ID1, path, fuseFd);
    EXPECT_TRUE(result > 0);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountFileMgrFuse_001
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountFileMgrFuse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "";
    int32_t result = storageDaemonProviderTest_->UMountFileMgrFuse(StorageTest::USER_ID1, path);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFileOccupied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFileOccupied_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string path = "test_file.txt";
    const std::vector<std::string> inputList = {"unrelated_process_1", "unrelated_process_2"};
    std::vector<std::string> outputList;
    bool status = true;
    int32_t result = storageDaemonProviderTest_->IsFileOccupied(path, inputList, outputList, status);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFileOccupied_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_ResetSecretWithRecoveryKey_001
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_ResetSecretWithRecoveryKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ResetSecretWithRecoveryKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t rkType = 1;
    std::vector<uint8_t> key = {0x01, 0x23, 0x45, 0x67};
    int32_t result = storageDaemonProviderTest_->ResetSecretWithRecoveryKey(StorageTest::USER_ID1, rkType, key);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ResetSecretWithRecoveryKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_TryToFix_001
 * @tc.desc: Verify the TryToFix function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_TryToFix_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_TryToFix_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string volId = "vol-1-1";
    int32_t result = storageDaemonProviderTest_->TryToFix(volId, 0);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_TryToFix_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountUsbFuse_001
 * @tc.desc: Verify the MountUsbFuse function with normal parameters.
 * @tc.type: FUNC
 * @tc.require: issueI9G5A0
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountUsbFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountUsbFuse_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);

    std::string volumeId = "vol-usb-001";
    int fuseFd = -1;
    std::string fsUuid;
    int32_t result = storageDaemonProviderTest_->MountUsbFuse(volumeId, fsUuid, fuseFd);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountUsbFuse_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountDisShareFile_001
 * @tc.desc: Verify the MountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountDisShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDisShareFile_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = 100;
    std::map<std::string, std::string> shareFiles = {{{"/data/sharefile1", "/data/sharefile2"}}};
    auto ret = storageDaemonProviderTest_->MountDisShareFile(userId, shareFiles);
    EXPECT_TRUE(ret == E_NON_EXIST);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDisShareFile_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountDisShareFile_001
 * @tc.desc: Verify the UMountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountDisShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDisShareFile_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = 100;
    std::string networkId = "sharefile1";
    auto ret = storageDaemonProviderTest_->UMountDisShareFile(userId, networkId);
    EXPECT_TRUE(ret == E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDisShareFile_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_OnAddSystemAbility_001
 * @tc.desc: Verify the OnAddSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_OnAddSystemAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_OnAddSystemAbility_001 start";
    StorageDaemonProvider::SystemAbilityStatusChangeListener listener;

#ifdef EXTERNAL_STORAGE_MANAGER
    listener.OnAddSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, "device1");
#endif
    listener.OnAddSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, "device2");

    listener.OnAddSystemAbility(999999, "device3");

    SUCCEED();
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_OnAddSystemAbility_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_OnRemoveSystemAbility_001
 * @tc.desc: Verify the OnRemoveSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_OnRemoveSystemAbility_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_OnRemoveSystemAbility_001 start";
    StorageDaemonProvider::SystemAbilityStatusChangeListener listener;

    listener.OnRemoveSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, "device1");

    listener.OnRemoveSystemAbility(999999, "device2");

    SUCCEED();
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_OnRemoveSystemAbility_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_StorageRadarThd_001
 * @tc.desc: Verify the StorageRadarThd function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_StorageRadarThd_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StorageRadarThd_001 start";
    StorageStatisticRadar::GetInstance().CleanStatisticFile();

    std::map<uint32_t, RadarStatisticInfo> testStats;
    RadarStatisticInfo info = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    testStats[1234] = info;
    StorageStatisticRadar::GetInstance().UpdateStatisticFile(testStats);

    std::map<uint32_t, RadarStatisticInfo> checkStats;
    StorageStatisticRadar::GetInstance().ReadStatisticFile(checkStats);
    ASSERT_FALSE(checkStats.empty());

    StorageDaemonProvider* provider = new StorageDaemonProvider();

    delete provider;
    SUCCEED();
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_StorageRadarThd_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CheckUserid_001
 * @tc.desc: Verify the CheckUserid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CheckUserid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->CheckUserIdRange(StorageService::START_USER_ID - 1);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CheckUserid_002
 * @tc.desc: Verify the CheckUserid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CheckUserid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->CheckUserIdRange(StorageService::START_USER_ID);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_CheckUserid_003
 * @tc.desc: Verify the CheckUserid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_CheckUserid_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto ret = storageDaemonProviderTest_->CheckUserIdRange(StorageService::MAX_USER_ID + 1);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_CheckUserid_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFilePathInvalid_001
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFilePathInvalid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "../";
    bool result = storageDaemonProviderTest_->IsFilePathInvalid(path);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFilePathInvalid_002
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFilePathInvalid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "/..";
    int32_t result = storageDaemonProviderTest_->IsFilePathInvalid(path);
    EXPECT_TRUE(result);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFilePathInvalid_003
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFilePathInvalid_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "/valid/path/test/";
    int32_t result = storageDaemonProviderTest_->IsFilePathInvalid(path);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFilePathInvalid_004
 * @tc.desc: Verify the IsFilePathInvalid function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFilePathInvalid_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_004 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "/valid/path/test";
    int32_t result = storageDaemonProviderTest_->IsFilePathInvalid(path);
    EXPECT_FALSE(result);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFilePathInvalid_004 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_SetDirEncryptionPolicy_001
 * @tc.desc: Verify the SetDirEncryptionPolicy function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_SetDirEncryptionPolicy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetDirEncryptionPolicy_004 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = 100;
    std::string path = "/..";
    int32_t result = storageDaemonProviderTest_->SetDirEncryptionPolicy(userId, path, true);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_SetDirEncryptionPolicy end";
}

/**
 * @tc.name: StorageDaemonProviderTest_GetLockScreenStatus_002
 * @tc.desc: Verify the GetLockScreenStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_GetLockScreenStatus_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetLockScreenStatus_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    bool lockStatus = true;
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->GetLockScreenStatus(userId, lockStatus);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_GetLockScreenStatus_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_002
 * @tc.desc: Verify the UpdateUseAuthWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    auto authToken = GenerateTestVector(0x01, 32);
    auto newSecret = GenerateTestVector(0x02, 32);
    uint64_t secureUid = 100;
    uint32_t userid = -1;
    std::vector<std::vector<uint8_t>> plainText = {GenerateTestVector(0x03, 64), GenerateTestVector(0x04, 64)};
    int32_t ret = storageDaemonProviderTest_->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid,
                                                                           userid, plainText);
    EXPECT_EQ(ret, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UpdateUseAuthWithRecoveryKey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountDfsDocs_002
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/..";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    int32_t result = storageDaemonProviderTest_->MountDfsDocs(StorageTest::USER_ID1, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountDfsDocs_003
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountDfsDocs_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/documents";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->MountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDfsDocs_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountDfsDocs_002
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/..";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    int32_t result =
        storageDaemonProviderTest_->UMountDfsDocs(StorageTest::USER_ID1, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountDfsDocs_003
 * @tc.desc: Verify the UMountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountDfsDocs_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_003 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string relativePath = "/documents";
    std::string networkId = "network-123";
    std::string deviceId = "device-456";
    uint32_t userId = -1;
    int32_t result =
        storageDaemonProviderTest_->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDfsDocs_003 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountMediaFuse_002
 * @tc.desc: Verify the MountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountMediaFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountMediaFuse_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t devFd = 1001;
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->MountMediaFuse(userId, devFd);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountMediaFuse_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountMediaFuse_002
 * @tc.desc: Verify the UMountMediaFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountMediaFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountMediaFuse_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->UMountMediaFuse(userId);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountMediaFuse_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountFileMgrFuse_002
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountFileMgrFuse_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "";
    int32_t fuseFd = 0;
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountFileMgrFuse_002
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountFileMgrFuse_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountFileMgrFuse_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "";
    uint32_t userId = -1;
    int32_t result = storageDaemonProviderTest_->UMountFileMgrFuse(userId, path);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountFileMgrFuse_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_IsFileOccupied_002
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_IsFileOccupied_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFileOccupied_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    const std::string path = "/..";
    const std::vector<std::string> inputList = {"unrelated_process_1", "unrelated_process_2"};
    std::vector<std::string> outputList;
    bool status = true;
    int32_t result = storageDaemonProviderTest_->IsFileOccupied(path, inputList, outputList, status);
    EXPECT_EQ(result, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_IsFileOccupied_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_ResetSecretWithRecoveryKey_002
 * @tc.desc: Verify the ResetSecretWithRecoveryKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_ResetSecretWithRecoveryKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ResetSecretWithRecoveryKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    uint32_t rkType = 1;
    uint32_t userId = -1;
    std::vector<uint8_t> key = {0x01, 0x23, 0x45, 0x67};
    int32_t result = storageDaemonProviderTest_->ResetSecretWithRecoveryKey(userId, rkType, key);
    EXPECT_EQ(result, E_USERID_RANGE);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ResetSecretWithRecoveryKey_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_MountDisShareFile_002
 * @tc.desc: Verify the MountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_MountDisShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDisShareFile_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = -1;
    std::map<std::string, std::string> shareFiles = {{{"/data/sharefile1", "/data/sharefile2"}}};
    auto ret = storageDaemonProviderTest_->MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_MountDisShareFile_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_UMountDisShareFile_002
 * @tc.desc: Verify the UMountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_UMountDisShareFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDisShareFile_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    int32_t userId = -1;
    std::string networkId = "sharefile1";
    auto ret = storageDaemonProviderTest_->UMountDisShareFile(userId, networkId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_UMountDisShareFile_002 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InactiveUserPublicDirKey_001
 * @tc.desc: Verify the InactiveUserPublicDirKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InactiveUserPublicDirKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserPublicDirKey_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = storageDaemonProviderTest_->InactiveUserPublicDirKey(StorageService::START_USER_ID);
    EXPECT_EQ(ret, E_OK);
#endif
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserPublicDirKey_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_InactiveUserPublicDirKey_002
 * @tc.desc: Verify the InactiveUserPublicDirKey function.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_InactiveUserPublicDirKey_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserPublicDirKey_002 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
#ifdef USER_CRYPTO_MANAGER
    int32_t ret = storageDaemonProviderTest_->InactiveUserPublicDirKey(StorageService::START_USER_ID - 1);
    EXPECT_EQ(ret, E_USERID_RANGE);
#endif
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_InactiveUserPublicDirKey_002 end";
}

/**
 * @tc.number: SUB_STORAGE_StorageDaemonProvider_GetDataSizeByPath_001
 * @tc.name: StorageDaemonProvider_GetDataSizeByPath_001
 * @tc.desc: Test function of GetDataSizeByPath interface using mock.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProvider_GetDataSizeByPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProvider_GetDataSizeByPath_001 start";

    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string path = "/data/test/file.txt";
    int64_t size = 0;
    int32_t result = storageDaemonProviderTest_->GetDataSizeByPath(path, size);
    EXPECT_EQ(result, E_FILE_PATH_INVALID);

    GTEST_LOG_(INFO) << "StorageDaemonProvider_GetDataSizeByPath_001 end";
}

/**
 * @tc.number: SUB_STORAGE_StorageDaemonProvider_GetRmgResourceSize_001
 * @tc.name: StorageDaemonProvider_GetRmgResourceSize_001
 * @tc.desc: Test function of GetRmgResourceSize interface using mock.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require:
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProvider_GetRmgResourceSize_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProvider_GetRmgResourceSize_001 start";

    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::string rgmName = "rgm_hmos";
    uint64_t totalSize = 0;

    int32_t result = storageDaemonProviderTest_->GetRmgResourceSize(rgmName, totalSize);
    EXPECT_EQ(result, E_OK);

    GTEST_LOG_(INFO) << "StorageDaemonProvider_GetRmgResourceSize_001 end";
}

/**
 * @tc.name: StorageDaemonProviderTest_ListUserdataDirInfo_001
 * @tc.desc: Verify the ListUserdataDirInfo function.
 * @tc.type: FUNC
 * @tc.require: AR20251022750568
 */
HWTEST_F(StorageDaemonProviderTest, StorageDaemonProviderTest_ListUserdataDirInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ListUserdataDirInfo_001 start";
    ASSERT_TRUE(storageDaemonProviderTest_ != nullptr);
    std::vector<UserdataDirInfo> scanDirs;

    int32_t result =
        storageDaemonProviderTest_->ListUserdataDirInfo(scanDirs);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonProviderTest_ListUserdataDirInfo_001 end";
}
} // namespace StorageDaemon
} // namespace OHOS