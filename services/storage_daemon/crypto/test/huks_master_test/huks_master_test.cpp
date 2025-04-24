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
 * @tc.name: HuksMaster_InitHdiProxyInstance_001
 * @tc.desc: Verify the InitHdiProxyInstance.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_InitHdiProxyInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_InitHdiProxyInstance_001 start";
    // first time init
    HuksMaster::GetInstance().g_hksHdiProxyInstance = nullptr;
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    // not first time init
    EXPECT_NE(HuksMaster::GetInstance().g_hksHdiProxyInstance, nullptr);
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);

    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().g_hksHdiProxyInstance, nullptr);

    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().g_hksHdiProxyInstance, nullptr);

    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    GTEST_LOG_(INFO) << "HuksMaster_InitHdiProxyInstance_001_001 end";
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
    HuksMaster::GetInstance().g_hksHdiProxyInstance = nullptr;
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_ERROR_NULL_POINTER);

    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_SUCCESS);

    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->ModuleInit = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiModuleInit(), HKS_ERROR_NULL_POINTER);
    }

    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_NE(HuksMaster::GetInstance().HdiModuleInit(), HKS_SUCCESS);
    GTEST_LOG_(INFO) << "HuksMaster_HdiModuleInit_001 end";
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
    HuksMaster::GetInstance().g_hksHdiProxyInstance = nullptr;
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_ERROR_NULL_POINTER);

    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->ModuleDestroy = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_ERROR_NULL_POINTER);
    }

    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_NE(HuksMaster::GetInstance().HdiModuleDestroy(), HKS_SUCCESS);
    GTEST_LOG_(INFO) << "HuksMaster_HuksHdiModuleDestroy_001 end";
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
    HuksBlob hksAlias;
    HuksBlob hksKeyOut;
    HksParamSet *paramSet = nullptr;
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->GenerateKey = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiGenerateKey(hksAlias, paramSet, hksKeyOut), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();;
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
    HuksBlob key;
    HksParamSet *paramSet = nullptr;
    HuksBlob handle;
    HuksBlob token;
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->Init = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessInit(key, paramSet, handle, token), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
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
    HuksBlob handle;
    HksParamSet *paramSet = nullptr;
    HuksBlob inData;
    HuksBlob outData;
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->Finish = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessFinish(handle, paramSet, inData, outData),
            HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
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
    HuksBlob oldKey;
    HuksBlob newKey;
    HksParamSet *paramSet = nullptr;
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    if (HuksMaster::GetInstance().g_hksHdiProxyInstance != nullptr) {
        HuksMaster::GetInstance().g_hksHdiProxyInstance->UpgradeKey = nullptr;
        EXPECT_EQ(HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey), HKS_ERROR_NULL_POINTER);
    }
    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().HdiAccessUpgradeKey(oldKey, paramSet, newKey), HKS_ERROR_NULL_POINTER);
    GTEST_LOG_(INFO) << "HuksMaster_HdiAccessUpgradeKey_001 end";
}

/**
 * @tc.name: HuksMaster_EncryptKeyEx_001
 * @tc.desc: Verify the EncryptKeyEx.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_EncryptKeyEx_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_EncryptKeyEx_001 start";
    UserAuth auth;
    KeyBlob rnd;
    KeyContext ctx;
    EXPECT_NE(HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx), E_OK);
    
    std::vector<uint8_t> blobVec{1, 2, 3, 4, 5};
    ctx.shield.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), ctx.shield.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx), E_OK);

    rnd.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), rnd.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx), E_OK);

    auth.secret.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), auth.secret.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx), E_OK);

    auth.token.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), auth.token.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKeyEx(auth, rnd, ctx), E_OK);
    GTEST_LOG_(INFO) << "HuksMaster_EncryptKeyEx_001 end";
}

/**
 * @tc.name: HuksMaster_EncryptKey_001
 * @tc.desc: Verify the EncryptKey.
 * @tc.type: FUNC
 * @tc.require: IAUK5E
 */
HWTEST_F(HuksMasterTest, HuksMaster_EncryptKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HuksMaster_EncryptKey_001 start";
    UserAuth auth;
    KeyContext ctx;
    KeyInfo key;
    bool isNeedNewNonce = false;
    EXPECT_NE(HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce), E_OK);
    
    std::vector<uint8_t> blobVec{1, 2, 3, 4, 5};
    ctx.shield.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), ctx.shield.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce), E_OK);

    key.key.Alloc(blobVec.size());
    std::copy(blobVec.begin(), blobVec.end(), key.key.data.get());
    EXPECT_NE(HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce), E_OK);

    HuksMaster::GetInstance().ReleaseHdiProxyInstance();
    EXPECT_EQ(HuksMaster::GetInstance().InitHdiProxyInstance(), HKS_SUCCESS);
    EXPECT_NE(HuksMaster::GetInstance().EncryptKey(ctx, auth, key, isNeedNewNonce), E_OK);
    GTEST_LOG_(INFO) << "HuksMaster_EncryptKey_001 end";
}
} // OHOS::StorageDaemon