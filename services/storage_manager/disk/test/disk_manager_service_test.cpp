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

#include <cstdio>
#include <gtest/gtest.h>

#include "disk/disk_manager_service.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "mock/file_utils_mock.h"
#include "mock/mock_parameters.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
using namespace StorageDaemon;
const std::string FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE = "const.enterprise.external_storage_device.manage.enable";

class DiskManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void DiskManagerServiceTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void DiskManagerServiceTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void DiskManagerServiceTest::SetUp(void)
{
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void DiskManagerServiceTest::TearDown()
{
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskCreated_0000
 * @tc.name: Disk_manager_service_OnDiskCreated_0000
 * @tc.desc: Test function of OnDiskCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskCreated_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskCreated_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-1";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-1";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    result = dmService.GetDiskById(diskId);
    dmService.OnDiskDestroyed(diskId);

    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskCreated_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskCreated_0001
 * @tc.name: Disk_manager_service_OnDiskCreated_0001
 * @tc.desc: Test function of OnDiskCreated interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskCreated_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskCreated_0001";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-2";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-2";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    dmService.OnDiskCreated(disk);
    result = dmService.GetDiskById(diskId);
    dmService.OnDiskDestroyed(diskId);

    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskCreated_0001";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskDestroyed_0000
 * @tc.name: Disk_manager_service_OnDiskDestroyed_0000
 * @tc.desc: Test function of OnDiskDestroyed interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskDestroyed_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskDestroyed_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-3";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-3";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    result = dmService.GetDiskById(diskId);
    EXPECT_NE(result, nullptr);
    dmService.OnDiskDestroyed(diskId);
    result = dmService.GetDiskById(diskId);

    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskDestroyed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_OnDiskDestroyed_0001
 * @tc.name: Disk_manager_service_OnDiskDestroyed_0001
 * @tc.desc: Test function of OnDiskDestroyed interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_OnDiskDestroyed_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_OnDiskDestroyed_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-4";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-4";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskDestroyed(diskId);
    result = dmService.GetDiskById(diskId);

    EXPECT_EQ(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_OnDiskDestroyed_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetDiskById_0000
 * @tc.name: Disk_manager_service_GetDiskById_0000
 * @tc.desc: Test function of GetDiskById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetDiskById_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetDiskById_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-5";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-5";
    int32_t flag = 1;
    std::shared_ptr<Disk> result;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    result = dmService.GetDiskById(diskId);
    dmService.OnDiskDestroyed(diskId);
    EXPECT_NE(result, nullptr);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_GetDiskById_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetDiskById_0001
 * @tc.name: Disk_manager_service_GetDiskById_0001
 * @tc.desc: Test function of GetDiskById interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetDiskById_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetDiskById_0001";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-6";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-6";
    int32_t flag = 1;
    int32_t result = E_OK;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    result = dmService.GetDiskById(diskId, disk);
    dmService.OnDiskDestroyed(diskId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_GetDiskById_0001";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetDiskById_0002
 * @tc.name: Disk_manager_service_GetDiskById_0002
 * @tc.desc: Test function of GetDiskById interface for FAILED.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetDiskById_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetDiskById_0002";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-7";
    std::string diskId_1 = "diskId-1-11";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-7";
    int32_t flag = 1;
    int32_t result = E_OK;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    result = dmService.GetDiskById(diskId_1, disk);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_GetDiskById_0002";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_Partition_0000
 * @tc.name: Disk_manager_service_Partition_0000
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_Partition_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_Partition_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-8";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-8";
    int32_t flag = 1;
    int32_t type = 1;
    int32_t result = E_OK;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
<<<<<<< HEAD
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
=======
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
>>>>>>> 97f5f8fe (daemon代码)
    dmService.OnDiskCreated(disk);
    result = dmService.Partition(diskId, type);
    dmService.OnDiskDestroyed(diskId);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_Partition_0000";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_Partition_0001
 * @tc.name: Disk_manager_service_Partition_0001
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_Partition_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_Partition_0001";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-9";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-9";
    int32_t flag = 1;
    int32_t type = 1;
    int32_t result = E_OK;
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    result = dmService.Partition(diskId, type);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_Partition_0001";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_Partition_0002
 * @tc.name: Disk_manager_service_Partition_0002
 * @tc.desc: Test function of Partition interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_Partition_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_Partition_0002";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-9";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-9";
    int32_t flag = 1;
    int32_t type = 1;
    int32_t result = E_OK;
    system::SetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    system::GetBoolParameter(FUSE_PARAM_SERVICE_ENTERPRISE_ENABLE, false);
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    result = dmService.Partition(diskId, type);
    EXPECT_EQ(result, E_NON_EXIST);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service_Partition_0002";
}

/**
 * @tc.number: SUB_STORAGE_Disk_manager_service_GetAllDisks_0000
 * @tc.name: Disk_manager_service_GetAllDisks_0000
 * @tc.desc: Test function of GetAllDisks interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(DiskManagerServiceTest, Disk_manager_service_GetAllDisks_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-begin Disk_manager_service_GetAllDisks_0000";
    DiskManagerService& dmService = DiskManagerService::GetInstance();
    std::string diskId = "diskId-1-10";
    int64_t sizeBytes = 1024;
    std::string sysPath = "/";
    std::string vendor = "vendor-10";
    int32_t flag = 1;
    Disk disk(diskId, sizeBytes, sysPath, vendor, flag);
    dmService.OnDiskCreated(disk);
    vector<Disk> result = dmService.GetAllDisks();
    if (result.size() > 0)
    {
        GTEST_LOG_(INFO) << result[0].GetSizeBytes();
        GTEST_LOG_(INFO) << result[0].GetDiskId();
        GTEST_LOG_(INFO) << result[0].GetSysPath();
        GTEST_LOG_(INFO) << result[0].GetVendor();
        GTEST_LOG_(INFO) << result[0].GetFlag();
    }
    EXPECT_GE(result.size(), 0);
    dmService.OnDiskDestroyed(diskId);
    GTEST_LOG_(INFO) << "DiskManagerServiceTest-end Disk_manager_service__0000";
}
} // namespace
