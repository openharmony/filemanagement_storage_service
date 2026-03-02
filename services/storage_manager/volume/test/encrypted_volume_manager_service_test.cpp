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

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0000";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_VOL_MOUNT_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0000";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0001";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::MOUNTED);
    DelayedSingleton<StorageDaemonCommunication>::instance_ = nullptr;
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    DelayedSingleton<StorageDaemonCommunication>::instance_ = storageDaemonCommunicationMock_;
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0001";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0002";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::MOUNTED);
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0002";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0003";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::MOUNTED);
    int32_t result = encryptVmService.Encrypt(volumeId, pazzword);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0003";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_Encrypt_0004, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_Encrypt_0004";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    std::string pazzword = "testPasswd";
    int32_t result = encryptVmService.Encrypt("volumeId", pazzword);
    EXPECT_EQ(result, E_VOLUMEEX_IS_NULLPTR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_Encrypt_0004";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0000";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_VOL_MOUNT_ERR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0000";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0001";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById("volumeId-2", progress);
    EXPECT_EQ(result, E_VOLUMEEX_IS_NULLPTR);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0001";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0002,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0002";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::ENCRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0002";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0003,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0003";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::DECRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0003";
}

HWTEST_F(EncryptedVolumeManagerServiceTest, Storage_manager_proxy_GetCryptProgressById_0004,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-begin Storage_manager_proxy_GetCryptProgressById_0004";
    auto &encryptVmService = EncryptedVolumeManagerService::GetInstance();
    auto &vmService = VolumeManagerService::GetInstance();
    std::string volumeId = "vol-1-11";
    int fsType = 1;
    std::string diskId = "disk-1-11";
    VolumeCore vc(volumeId, fsType, diskId);
    vmService.OnVolumeCreated(vc);
    auto volumePtr = vmService.volumeMap_.ReadVal(volumeId);
    volumePtr->SetState(VolumeState::DECRYPTING);
    int32_t progress = 0;
    int32_t result = encryptVmService.GetCryptProgressById(volumeId, progress);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "EncryptedVolumeManagerServiceTest-end Storage_manager_proxy_GetCryptProgressById_0004";
}
} // namespace
