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
using OHOS::Storage::StorageDaemon::KeepStatus;
using OHOS::Storage::StorageDaemon::ZipUtil;

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

/**
 * @tc.number: HiAudit_WriteToFile_WriteError_001
 * @tc.desc: Verify writeLogSize_ unchanged when write() returns error
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_WriteToFile_WriteError_001, TestSize.Level1)
{
    auto &audit = HiAudit::GetInstance();
    HiAuditStateGuard guard(audit);

    std::string roPath = "/data/log/hiaudit/storagedaemon/ro_test.csv";
    int roFd = open(roPath.c_str(), O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR);
    ASSERT_GE(roFd, 0);
    audit.writeFd_ = roFd;
    audit.writeLogSize_ = 100;
    uint32_t before = audit.writeLogSize_.load();

    audit.WriteToFile("test content");

    EXPECT_EQ(audit.writeLogSize_.load(), before);
    remove(roPath.c_str());
}

/**
 * @tc.number: HiAudit_WriteToFile_Normal_001
 * @tc.desc: Verify writeLogSize_ grows by content length on normal write
 * @tc.type: FUNC
 */
HWTEST_F(HiAuditTest, HiAudit_WriteToFile_Normal_001, TestSize.Level1)
{
    auto &audit = HiAudit::GetInstance();
    HiAuditStateGuard guard(audit);

    std::string rwPath = "/data/log/hiaudit/storagedaemon/rw_test.csv";
    int rwFd = open(rwPath.c_str(), O_CREAT | O_TRUNC | O_RDWR, S_IRUSR | S_IWUSR);
    ASSERT_GE(rwFd, 0);
    audit.writeFd_ = rwFd;
    audit.writeLogSize_ = 0;

    std::string content = "hello audit";
    audit.WriteToFile(content);

    EXPECT_EQ(audit.writeLogSize_.load(), static_cast<uint32_t>(content.length()));
    remove(rwPath.c_str());
}
} // namespace StorageDaemon
} // namespace OHOS
