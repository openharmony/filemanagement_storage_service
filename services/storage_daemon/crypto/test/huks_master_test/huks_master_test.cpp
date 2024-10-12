/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "hks_param.h"
#include "huks_master.h"
#include "storage_service_errno.h"

using namespace testing::ext;
using namespace testing;
using namespace std;

namespace OHOS::StorageDaemon {
class HuksMasterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void HuksMasterTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void HuksMasterTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void HuksMasterTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void HuksMasterTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: HuksMaster_HuksHdiModuleDestroy_001
 * @tc.desc: Verify the HuksHdiModuleDestroy.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HuksHdiModuleDestroy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HuksHdiModuleDestroy_001 start";
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiModuleDestroy = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_ERROR_NULL_POINTER);
    }

    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HuksHdiModuleDestroy_001 end";
}

/**
 * @tc.name: HuksMaster_HdiModuleInit_001
 * @tc.desc: Verify the HdiModuleInit.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiModuleInit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiModuleInit_001 start";
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_SUCCESS);

    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiModuleInit = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_ERROR_NULL_POINTER);
    }
    
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiModuleInit_001 end";
}

/**
 * @tc.name: HuksMaster_HdiCreate_001
 * @tc.desc: Verify the HdiCreate.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiCreate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiCreate_001 start";
    // hdiHandle_ && halDevice_ not null
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);

    // release halDevice_
    ASSERT_NE(HuksMaster::GetInstance().hdiHandle_, nullptr);
    auto destroyHdi = reinterpret_cast<HkmHalDestroyHandle>(
        dlsym(HuksMaster::GetInstance().hdiHandle_, "HuksDestoryHdiDevicePtr"));
    if ((destroyHdi != nullptr) && (HuksMaster::GetInstance().halDevice_ != nullptr)) {
        (*destroyHdi)(HuksMaster::GetInstance().halDevice_);
    }
    HuksMaster::GetInstance().halDevice_ = nullptr;
    // "halDevice_ is nullptr"
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_ERROR_NULL_POINTER);
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_ERROR_NULL_POINTER);
    // hdiHandle_ not nullptr, halDevice_ is nullptr
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    // release all
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().hdiHandle_, nullptr);
    EXPECT_EQ(HuksMaster::GetInstance().halDevice_, nullptr);
    // hdiHandle_ is nullptr, already destroyed
    HuksMaster::GetInstance().HdiDestroy();
    // create hdiHandle_
    HuksMaster::GetInstance().hdiHandle_ = dlopen("libhuks_engine_core_standard.z.so", RTLD_LAZY);
    ASSERT_NE(HuksMaster::GetInstance().hdiHandle_, nullptr);
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    GTEST_LOG_(INFO) << "HuksMaster_HdiCreate_001 end";
}

/**
 * @tc.name: HuksMaster_HdiGenerateKey_001
 * @tc.desc: Verify the HdiGenerateKey.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiGenerateKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiGenerateKey_001 start";
    HksBlob hksAlias;
    HksBlob hksKeyOut;
    HksParamSet *paramSet = nullptr;
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiGenerateKey = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiGenerateKey(hksAlias, paramSet, hksKeyOut), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiGenerateKey(hksAlias, paramSet, hksKeyOut), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiGenerateKey_001 end";
}

/**
 * @tc.name: HuksMaster_HdiAccessInit_001
 * @tc.desc: Verify the HdiAccessInit.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiAccessInit_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessInit_001 start";
    HksBlob key;
    HksParamSet *paramSet = nullptr;
    HksBlob handle;
    HksBlob token;
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiInit = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessInit(key, paramSet, handle, token), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiAccessInit(key, paramSet, handle, token), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessInit_001 end";
}

/**
 * @tc.name: HuksMaster_HdiAccessFinish_001
 * @tc.desc: Verify the HdiAccessFinish.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiAccessFinish_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessFinish_001 start";
    HksBlob handle;
    HksParamSet *paramSet = nullptr;
    HksBlob inData;
    HksBlob outData;
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiFinish = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessFinish(handle, paramSet, inData, outData),
            HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiAccessFinish(handle, paramSet, inData, outData), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessFinish_001 end";
}

/**
 * @tc.name: HuksMaster_HdiAccessUpgradeKey_001
 * @tc.desc: Verify the HdiAccessUpgradeKey.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_HdiAccessUpgradeKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessUpgradeKey_001 start";
    HksBlob oldKey;
    HksBlob newKey;
    HksParamSet *paramSet = nullptr;
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiCreate(), true);
    if (HuksMaster::GetInstance().halDevice_ != nullptr) {
        HuksMaster::GetInstance().halDevice_->HuksHdiUpgradeKey = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().HdiDestroy();
    EXPECT_EQ(HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessUpgradeKey_001 end";
}
} // OHOS::StorageDaemon