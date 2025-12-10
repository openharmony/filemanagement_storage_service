/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dfx_report/storage_dfx_reporter.h"
#include "mock/storage_daemon_communication_mock.h"
#include "mock/storage_status_manager_mock.h"
#include "statistic_info.h"
#include "storage_total_status_service_mock.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;
using namespace testing;


class StorageDfxReporterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown(){};
};

void StorageDfxReporterTest::SetUpTestCase()
{
}

void StorageDfxReporterTest::TearDownTestCase()
{
}

void StorageDfxReporterTest::SetUp()
{
}
} // namespace StorageManager
} // namespace OHOS