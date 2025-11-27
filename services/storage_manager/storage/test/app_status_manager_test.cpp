/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "storage/app_status_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

class AppStatusManagerTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

/**
* @tc.number: App_status_DelBundleExtStats_test_0000
* @tc.name: App_status_DelBundleExtStats_test_0000
* @tc.desc: Test function of DelBundleExtStats
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: SR000GGUPF
*/
HWTEST_F(AppStatusManagerTest, App_status_DelBundleExtStats_test_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "App_status_DelBundleExtStats_test_0000 begin";
    AppStatusManager &service = AppStatusManager::GetInstance();
    int32_t userId = 100;
    std::string businessName = "test";
    int32_t ret = service.DelBundleExtStats(userId, businessName);
    EXPECT_EQ(ret, E_RDB_STORE_NULL);
    GTEST_LOG_(INFO) << "App_status_DelBundleExtStats_test_0000 end";
}
}
}