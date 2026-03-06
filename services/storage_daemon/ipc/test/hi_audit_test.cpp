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

#include "hi_audit.h"
#include "zip_utils.h"

#include <fcntl.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;
using OHOS::Storage::DistributedFile::KeepStatus;
using OHOS::Storage::DistributedFile::ZipUtil;

class HiAuditStateGuard {
public:
    explicit HiAuditStateGuard(HiAudit &audit) : audit_(audit), writeFd_(audit.writeFd_),
        writeLogSize_(audit.writeLogSize_.load()) {}
    ~HiAuditStateGuard()
    {
        if (audit_.writeFd_ >= 0 && audit_.writeFd_ != writeFd_) {
            close(audit_.writeFd_);
        }
        audit_.writeFd_ = writeFd_;
        audit_.writeLogSize_ = writeLogSize_;
    }

private:
    HiAudit &audit_;
    int writeFd_;
    uint32_t writeLogSize_;
};

class HiAuditTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() override {};
    void TearDown() override {};
};

/**
 * @tc.number: HiAudit_GetMilliseconds_001
 * @tc.desc: Verify GetMilliseconds returns valid timestamp
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_GetMilliseconds_001, TestSize.Level1)
{
    auto &audit = HiAudit::GetInstance();
    uint64_t timestamp1 = audit.GetMilliseconds();
    uint64_t timestamp2 = audit.GetMilliseconds();
    EXPECT_GT(timestamp1, 0ULL);
    EXPECT_GE(timestamp2, timestamp1);
    EXPECT_LT(timestamp2 - timestamp1, 1000ULL);
}

/**
 * @tc.number: HiAudit_GetFormattedTimestampEndWithMilli_001
 * @tc.desc: Verify GetFormattedTimestampEndWithMilli formats correctly
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_GetFormattedTimestampEndWithMilli_001, TestSize.Level1)
{
    auto &audit = HiAudit::GetInstance();
    std::string timestamp = audit.GetFormattedTimestampEndWithMilli();
    EXPECT_GT(timestamp.length(), 10U);
    EXPECT_TRUE(timestamp.find("2025") != std::string::npos || timestamp.find("2026") != std::string::npos);
}

/**
 * @tc.number: HiAudit_ZipUtil_GetDestFilePath_001
 * @tc.desc: Verify ZipUtil::GetDestFilePath with simple paths
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtil_GetDestFilePath_001, TestSize.Level1)
{
    EXPECT_EQ(ZipUtil::GetDestFilePath("", "dest.csv", KeepStatus::KEEP_NONE_PARENT_PATH), "dest.csv");
    EXPECT_EQ(ZipUtil::GetDestFilePath("simple.txt/", "", KeepStatus::KEEP_NONE_PARENT_PATH), "simple.txt/");
    EXPECT_EQ(ZipUtil::GetDestFilePath("test//simple.txt", "", KeepStatus::KEEP_NONE_PARENT_PATH), "simple.txt");
}

/**
 * @tc.number: HiAudit_ZipUtil_CloseZipFile_001
 * @tc.desc: Verify ZipUtil::CloseZipFile handles null pointer
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_ZipUtil_CloseZipFile_001, TestSize.Level1)
{
    zipFile nullZip = nullptr;
    ZipUtil::CloseZipFile(nullZip);
    EXPECT_EQ(nullZip, nullptr);
}
} // namespace StorageDaemon
} // namespace OHOS
