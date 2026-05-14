/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "disk_manager/volume/volume_operator_factory.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class ExtVolumeOperatorFactoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ExtVolumeOperatorFactoryTest::SetUpTestCase(void) {}
void ExtVolumeOperatorFactoryTest::TearDownTestCase(void) {}
void ExtVolumeOperatorFactoryTest::SetUp(void) {}
void ExtVolumeOperatorFactoryTest::TearDown(void) {}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_Vfat, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::CreateOperator("vfat");
    EXPECT_NE(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_Ntfs, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::CreateOperator("ntfs");
    EXPECT_NE(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_Exfat, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::CreateOperator("exfat");
    EXPECT_NE(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_UnknownType, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::CreateOperator("unknown_fs");
    EXPECT_EQ(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_EmptyType, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::CreateOperator("");
    EXPECT_EQ(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, CreateOperator_SameInstance, TestSize.Level1)
{
    auto op1 = VolumeOperatorFactory::CreateOperator("vfat");
    auto op2 = VolumeOperatorFactory::CreateOperator("vfat");
    EXPECT_EQ(op1.get(), op2.get());
}

HWTEST_F(ExtVolumeOperatorFactoryTest, GetGenericOperator_NotNull, TestSize.Level1)
{
    auto op = VolumeOperatorFactory::GetGenericOperator();
    EXPECT_NE(op, nullptr);
}

HWTEST_F(ExtVolumeOperatorFactoryTest, GetGenericOperator_SameInstance, TestSize.Level1)
{
    auto op1 = VolumeOperatorFactory::GetGenericOperator();
    auto op2 = VolumeOperatorFactory::GetGenericOperator();
    EXPECT_EQ(op1.get(), op2.get());
}

HWTEST_F(ExtVolumeOperatorFactoryTest, GetGenericOperator_SharedWithCreateOperator, TestSize.Level1)
{
    auto generic = VolumeOperatorFactory::GetGenericOperator();
    ASSERT_NE(generic, nullptr);
    auto vfatOp = VolumeOperatorFactory::CreateOperator("vfat");
    ASSERT_NE(vfatOp, nullptr);
    EXPECT_EQ(generic.get(), vfatOp.get());
}

} // namespace StorageDaemon
} // namespace OHOS
