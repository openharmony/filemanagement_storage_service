/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "gtest/gtest.h"
#include "common/help_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/hi_audit.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

constexpr int MAX_LOG_FILE_SIZE = 3 * 1024 * 1024;

class HiAuditTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: HiAuditTest_Write_001
 * @tc.desc: Verify the Write function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(HiAuditTest, HiAuditTest_Write_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HiAuditTest_Write_001 start";
    AuditLog storageAuditLog = { false, "FAILED TO Mount", "ADD", "Mount", 1, "FAIL" };
    while (HiAudit::GetInstance().writeLogSize_ < MAX_LOG_FILE_SIZE) {
        HiAudit::GetInstance().Write(storageAuditLog);
    }
    HiAudit::GetInstance().Write(storageAuditLog);
    EXPECT_GT(HiAudit::GetInstance().writeLogSize_, 0);
    EXPECT_GT(HiAudit::GetInstance().writeFd_, 0);
    GTEST_LOG_(INFO) << "HiAuditTest_Write_001 end";
}
} // STORAGE_DAEMON
} // OHOS
