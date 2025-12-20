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

#include <cstdio>
#include <gtest/gtest.h>

#include "volume/volume_manager_service_ext.h"
#include "disk/disk_manager_service.h"
#include "volume_core.h"
#include "storage_service_errno.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class VolumeManagerServiceExtTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_Init_0000
 * @tc.name: Volume_manager_service_ext_Init_0000
 * @tc.desc: Test function of Init for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_Init_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_Init_0000";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    vmServiceExt.Init();
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_Init_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_UnInit_0000
 * @tc.name: Volume_manager_service_ext_UnInit_0000
 * @tc.desc: Test function of UnInit for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_UnInit_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_UnInit_0000";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    int32_t num = 0;
    vmServiceExt.handler_ = (void *)&num;
    vmServiceExt.UnInit();
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_UnInit_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_UnInit_0001
 * @tc.name: Volume_manager_service_ext_UnInit_0001
 * @tc.desc: Test function of UnInit for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_UnInit_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_UnInit_0001";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    vmServiceExt.UnInit();
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_UnInit_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_NotifyUsbFuseMount_0000
 * @tc.name: Volume_manager_service_ext_NotifyUsbFuseMount_0000
 * @tc.desc: Test function of NotifyUsbFuseMount for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_NotifyUsbFuseMount_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_NotifyUsbFuseMount_0000";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    int32_t num = 0;
    vmServiceExt.handler_ = (void *)&num;
    int fuseFd = 0;
    std::string volumeId = {};
    std::string fsUuid = {};
    int32_t result = vmServiceExt.NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_NotifyUsbFuseMount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_NotifyUsbFuseMount_0001
 * @tc.name: Volume_manager_service_ext_NotifyUsbFuseMount_0001
 * @tc.desc: Test function of NotifyUsbFuseMount for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_NotifyUsbFuseMount_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_NotifyUsbFuseMount_0001";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    int fuseFd = 0;
    std::string volumeId = {};
    std::string fsUuid = {};
    vmServiceExt.handler_ = nullptr;
    int32_t result = vmServiceExt.NotifyUsbFuseMount(fuseFd, volumeId, fsUuid);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_NotifyUsbFuseMount_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_NotifyUsbFuseUmount_0000
 * @tc.name: Volume_manager_service_ext_NotifyUsbFuseUmount_0000
 * @tc.desc: Test function of NNotifyUsbFuseUmount for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_NotifyUsbFuseUmount_0000,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_NotifyUsbFuseUmount_0000";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    int32_t num = 0;
    vmServiceExt.handler_ = (void *)&num;
    std::string volumeId = {};
    int32_t result = vmServiceExt.NotifyUsbFuseUmount(volumeId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_NotifyUsbFuseUmount_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_NotifyUsbFuseUmount_0001
 * @tc.name: Volume_manager_service_ext_NotifyUsbFuseUmount_0001
 * @tc.desc: Test function of NotifyUsbFuseUmount for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_NotifyUsbFuseUmount_0001,
    testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_NotifyUsbFuseUmount_0001";
    auto &vmServiceExt =VolumeManagerServiceExt::GetInstance();
    std::string volumeId = {};
    vmServiceExt.handler_ = nullptr;
    int32_t result = vmServiceExt.NotifyUsbFuseUmount(volumeId);
    EXPECT_NE(result, E_OK);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_NotifyUsbFuseUmount_0001";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_IsUsbFuseByType_0000
 * @tc.name: Volume_manager_service_ext_IsUsbFuseByType_0000
 * @tc.desc: Test function of IsUsbFuseByType for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_IsUsbFuseByType_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_IsUsbFuseByType_0000";
    auto &vmServiceExt = VolumeManagerServiceExt::GetInstance();
    int32_t num = 0;
    vmServiceExt.handler_ = (void *)&num;
    std::string fsType = "f2fs";
    auto enabled = vmServiceExt.IsUsbFuseByType(fsType);
    EXPECT_TRUE(enabled);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_IsUsbFuseByType_0000";
}

/**
 * @tc.number: SUB_STORAGE_Volume_manager_service_ext_IsUsbFuseByType_0001
 * @tc.name: Volume_manager_service_ext_IsUsbFuseByType_0001
 * @tc.desc: Test function of IsUsbFuseByType for UNSUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPF
 */
HWTEST_F(VolumeManagerServiceExtTest, Volume_manager_service_ext_IsUsbFuseByType_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-begin Volume_manager_service_ext_IsUsbFuseByType_0001";
    auto &vmServiceExt = VolumeManagerServiceExt::GetInstance();
    vmServiceExt.handler_ = nullptr;
    std::string fsType = "f2fs";
    auto enabled = vmServiceExt.IsUsbFuseByType(fsType);
    EXPECT_TRUE(enabled);
    GTEST_LOG_(INFO) << "VolumeManagerServiceExtTest-end Volume_manager_service_ext_IsUsbFuseByType_0001";
}
} // namespace