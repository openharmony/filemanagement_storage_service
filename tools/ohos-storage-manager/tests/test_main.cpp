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

#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace StorageManager {
namespace {

TEST(FormatBytesTest, BasicConversion)
{
    EXPECT_EQ(FormatBytes(0), "0.00B");
    EXPECT_EQ(FormatBytes(1024), "1.00KB");
    EXPECT_EQ(FormatBytes(1024 * 1024), "1.00MB");
    EXPECT_EQ(FormatBytes(1024 * 1024 * 1024), "1.00GB");
}

TEST(FormatBytesTest, LargeValues)
{
    EXPECT_EQ(FormatBytes(1024LL * 1024 * 1024 * 1024), "1.00TB");
    EXPECT_EQ(FormatBytes(500 * 1024 * 1024), "500.00MB");
}

TEST(PrintSuccessTest, JsonFormat)
{
    testing::internal::CaptureStdout();
    PrintSuccess("{\"value\":123}");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("\"success\":true") != std::string::npos);
}

TEST(PrintErrorTest, JsonFormat)
{
    testing::internal::CaptureStdout();
    PrintError(-1, "test error");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(output.find("\"success\":false") != std::string::npos);
    EXPECT_TRUE(output.find("\"code\":-1") != std::string::npos);
    EXPECT_TRUE(output.find("test error") != std::string::npos);
}

}
}
}
