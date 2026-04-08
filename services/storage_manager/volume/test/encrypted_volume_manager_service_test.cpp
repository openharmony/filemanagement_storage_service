/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <gtest/gtest.h>
#include <sys/xattr.h>

#include "volume/volume_manager_service.h"
#include "volume/encrypted_volume_manager_service.h"
#include "disk/disk_manager_service.h"
#include "mock/file_utils_mock.h"
#include "mock/mock_parameters.h"
#include "volume_core.h"
#include "storage_service_errno.h"
#include "volume_manager_service_mock.h"
#include "storage_daemon_communication/storage_daemon_communication.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
using namespace StorageDaemon;
using namespace testing;

class EncryptedVolumeManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    std::shared_ptr<MockVolumeManagerService> volumeManagerServiceMock_;
    std::shared_ptr<MockStorageDaemonCommunication> storageDaemonCommunicationMock_;
    std::shared_ptr<VolumeExternalMock> volumeExternMock;
};

void EncryptedVolumeManagerServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void EncryptedVolumeManagerServiceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void EncryptedVolumeManagerServiceTest::SetUp(void)
{
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    volumeManagerServiceMock_ = std::make_shared<MockVolumeManagerService>();
    storageDaemonCommunicationMock_ = std::make_shared<MockStorageDaemonCommunication>();
    volumeExternMock = std::make_shared<VolumeExternalMock>();
    DelayedSingleton<StorageDaemonCommunication>::instance_ = storageDaemonCommunicationMock_;
}

void EncryptedVolumeManagerServiceTest::TearDown()
{
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    DelayedSingleton<StorageDaemonCommunication>::instance_ = nullptr;
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::MOUNTED);
    DelayedSingleton<StorageDaemonCommunication>::instance_ = nullptr;
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    DelayedSingleton<StorageDaemonCommunication>::instance_ = storageDaemonCommunicationMock_;
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::MOUNTED);
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_3, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_3";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::MOUNTED);
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_3";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_4, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_4";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    int32_t result = encryptVmService.Encrypt("volumeId", pazzword);
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_4";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById("volumeId-2", progress);
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_3, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_3";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::DECRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_3";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_4, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_4";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::DECRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_4";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptUuidById_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptUuidById_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    std::string uuid;
    int32_t result = encryptVmService.GetCryptUuidById("non_exist_vol", uuid);
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptUuidById_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptUuidById_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptUuidById_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    std::string uuid;
    int32_t result = encryptVmService.GetCryptUuidById(volumeId, uuid);
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptUuidById_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptUuidById_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptUuidById_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_LOCKED);
    std::string uuid;
    int32_t result = encryptVmService.GetCryptUuidById(volumeId, uuid);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptUuidById_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_BindRecoverKeyToPasswd_0,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_BindRecoverKeyToPasswd_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    int32_t result = encryptVmService.BindRecoverKeyToPasswd("non_exist", "pwd123", "key456");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_BindRecoverKeyToPasswd_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_BindRecoverKeyToPasswd_1,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_BindRecoverKeyToPasswd_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.BindRecoverKeyToPasswd(volumeId,  "pwd123", "key456");
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_BindRecoverKeyToPasswd_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_BindRecoverKeyToPasswd_2,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_BindRecoverKeyToPasswd_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_UNLOCKED);
    int32_t result = encryptVmService.BindRecoverKeyToPasswd(volumeId, "pwd123", "key456");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_BindRecoverKeyToPasswd_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_UpdateCryptPasswd_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_UpdateCryptPasswd_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    int32_t result = encryptVmService.UpdateCryptPasswd("non_exist", "old123", "new456");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_UpdateCryptPasswd_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_UpdateCryptPasswd_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_UpdateCryptPasswd_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.UpdateCryptPasswd(volumeId, "old123", "new456");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_UpdateCryptPasswd_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_UpdateCryptPasswd_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_UpdateCryptPasswd_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_UNLOCKED);
    int32_t result = encryptVmService.UpdateCryptPasswd(volumeId, "old123", "new456");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_UpdateCryptPasswd_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_ResetCryptPasswd_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_ResetCryptPasswd_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    int32_t result = encryptVmService.ResetCryptPasswd("non_exist", "key123", "new456");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_ResetCryptPasswd_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_ResetCryptPasswd_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_ResetCryptPasswd_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.ResetCryptPasswd(volumeId, "key123", "new456");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_ResetCryptPasswd_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_ResetCryptPasswd_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_ResetCryptPasswd_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_UNLOCKED);
    int32_t result = encryptVmService.ResetCryptPasswd(volumeId, "key123", "new456");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_ResetCryptPasswd_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_VerifyCryptPasswd_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_VerifyCryptPasswd_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int32_t result = encryptVmService.VerifyCryptPasswd("non_exist", "pwd123");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_VerifyCryptPasswd_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_VerifyCryptPasswd_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_VerifyCryptPasswd_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.VerifyCryptPasswd(volumeId, "pwd123");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_VerifyCryptPasswd_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_VerifyCryptPasswd_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_VerifyCryptPasswd_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_LOCKED);
    int32_t result = encryptVmService.VerifyCryptPasswd(volumeId, "pwd123");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_VerifyCryptPasswd_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Unlock_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Unlock_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int32_t result = encryptVmService.Unlock("non_exist", "pwd123");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Unlock_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Unlock_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Unlock_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.Unlock(volumeId, "pwd123");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Unlock_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Unlock_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Unlock_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::ENCRYPTED_AND_LOCKED);
    int32_t result = encryptVmService.Unlock(volumeId, "pwd123");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Unlock_2";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Decrypt_0, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Decrypt_0";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int32_t result = encryptVmService.Decrypt("non_exist", "pwd123");
    EXPECT_EQ(result, E_PARAMS_NULLPTR_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Decrypt_0";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Decrypt_1, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Decrypt_1";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    int32_t result = encryptVmService.Decrypt(volumeId, "pwd123");
    EXPECT_EQ(result, E_VOL_STATE);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Decrypt_1";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Decrypt_2, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Decrypt_2";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    VolumeCore vc(volumeId, 1, "disk-1-11");
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_[volumeId];
    volumePtr->SetState(VolumeState::MOUNTED);
    int32_t result = encryptVmService.Decrypt(volumeId, "pwd123");
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Decrypt_2";
}
} // namespace
