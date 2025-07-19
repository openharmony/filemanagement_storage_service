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
#include <linux/kdev_t.h>

#include "volume/external_volume_info.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "mock/file_utils_mock.h"

namespace OHOS {
namespace StorageDaemon {
int32_t g_readMetadata = 0;
int g_extStorageMountForkExec = 0;
// int g_forkExec = 0;
int g_forkExecWithExit = 0;
int32_t ReadMetadata(const std::string &devPath, std::string &uuid, std::string &type, std::string &label)
{
    return g_readMetadata;
}

int ForkExecWithExit(std::vector<std::string> &cmd)
{
    return g_forkExecWithExit;
}

// int ForkExec(std::vector<std::string> &cmd, std::vector<std::string> *output)
// {
//     return g_forkExec;
// }

int ExtStorageMountForkExec(std::vector<std::string> &cmd)
{
    return g_extStorageMountForkExec;
}
} // STORAGE_DAEMON
} 

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class ExternalVolumeInfoTest1 : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    ExternalVolumeInfo* externalVolumeInfo_;
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void ExternalVolumeInfoTest1::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoTest1 SetUpTestCase";
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
}

void ExternalVolumeInfoTest1::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "ExternalVolumeInfoTest1 TearDownTestCase";
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
}

void ExternalVolumeInfoTest1::SetUp()
{
    externalVolumeInfo_ = new ExternalVolumeInfo();
}

void ExternalVolumeInfoTest1::TearDown(void)
{
    if (externalVolumeInfo_ != nullptr) {
        delete externalVolumeInfo_;
        externalVolumeInfo_ = nullptr;
    }
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest1_DoMount_001
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest1, Storage_Service_ExternalVolumeInfoTest1_DoMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoMount_001 start";

    ExternalVolumeInfo vol;
    uint32_t mountFlags = 0;
    // g_docheck = 0;
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(true));
    //auto ret = vol.DoMount(mountFlags);
    g_readMetadata = 0;
    externalVolumeInfo_->fsType_ = "f2fs";
    EXPECT_EQ(externalVolumeInfo_->DoMount(mountFlags), E_SYS_KERNEL_ERR);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoMount_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest1_DoMount_002
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest1, Storage_Service_ExternalVolumeInfoTest1_DoMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoMount_002 start";

    ExternalVolumeInfo vol;
    uint32_t mountFlags = 0;
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).WillOnce(testing::Return(false));
    auto ret = vol.DoMount(mountFlags);
    EXPECT_EQ(ret, E_DOCHECK_MOUNT);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoMount_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest1_DoUMount_001
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest1, Storage_Service_ExternalVolumeInfoTest1_DoUMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_001 start";

    ExternalVolumeInfo vol;
    bool force = true;
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).Times(2).WillOnce(testing::Return(true));
    auto ret = vol.DoUMount(force);
    EXPECT_EQ(ret, E_VOL_UMOUNT_ERR);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_001 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest1_DoUMount_002
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest1, Storage_Service_ExternalVolumeInfoTest1_DoUMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_002 start";

    ExternalVolumeInfo vol;
    bool force = false;
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).Times(2).WillOnce(testing::Return(true));
    auto ret = vol.DoUMount(force);
    EXPECT_EQ(ret, E_VOL_UMOUNT_ERR);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_002 end";
}

/**
 * @tc.name: Storage_Service_ExternalVolumeInfoTest1_DoUMount_003
 * @tc.desc: Verify the DoMount function.
 * @tc.type: FUNC
 * @tc.require: SR000GGUOT
 */
HWTEST_F(ExternalVolumeInfoTest1, Storage_Service_ExternalVolumeInfoTest1_DoUMount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_003 start";

    ExternalVolumeInfo vol;
    bool force = false;
    EXPECT_CALL(*fileUtilMoc_, IsUsbFuse()).Times(2).WillOnce(testing::Return(false));
    auto ret = vol.DoUMount(force);
    EXPECT_EQ(ret, E_VOL_UMOUNT_ERR);
    GTEST_LOG_(INFO) << "Storage_Service_ExternalVolumeInfoTest1_DoUMount_003 end";
}
} // STORAGE_DAEMON
} // OHOS
