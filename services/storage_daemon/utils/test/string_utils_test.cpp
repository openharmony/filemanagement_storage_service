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
#include "string_utils.h"

#include <gtest/gtest.h>
#include <tuple>
#include <climits>

namespace OHOS {
namespace StorageDaemon {
namespace Test {
using namespace testing;
using namespace testing::ext;
class StringUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: UserPathResolverTest_ConvertStringToInt_001
 * @tc.desc: Verify the ConvertStringToInt.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StringUtilsTest, UserPathResolverTest_ConvertStringToInt_001, TestSize.Level1)
{
    std::map <std::string, std::pair<bool, int64_t>> testMap{
        {"12345", {true, 12345}},
        {"", {false, 0}},
        {"abc", {false, 0}},
        {"123abc", {false, 0}},
        {"9223372036854775807", {true, LLONG_MAX}},
        {"-9223372036854775808", {true, LLONG_MIN}},
        {"9223372036854775808", {false, 0}},
        {"-9223372036854775809", {false, 0}},
        {" 12345", {true, 12345}},
    };
    for (const auto &test : testMap) {
        std::cout << "test case: " << test.first << std::endl;
        int64_t value = 0;
        EXPECT_EQ(ConvertStringToInt(test.first, value, BASE_DECIMAL), test.second.first);
        EXPECT_EQ(value, test.second.second);
    }
}

/**
 * @tc.name: UserPathResolverTest_ParseKeyValuePairs_001
 * @tc.desc: Verify the ParseKeyValuePairs.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StringUtilsTest, UserPathResolverTest_ParseKeyValuePairs_001, TestSize.Level1)
{
    std::map <std::string, std::unordered_map<std::string, std::string>> testMap {
        {"key1=value1,key2=value2", {{"key1", "value1"}, {"key2", "value2"}}},
        {"key1,key2", {{"key1", ""}, {"key2", ""}}},
        {"", {}},
        {"key=value", {{"key", "value"}}},
        {"key1=,key2=value2", {{"key1", ""}, {"key2", "value2"}}},
        {"=value1,key2=value2", {{"key2", "value2"}}},
        {",,key2=value2", {{"key2", "value2"}}},
        {"key=value1,key=value2", {{"key", "value2"}}},
        {"=,key2=", {{"key2", ""}}},
        {" key1 = value1 , key2 = value2 ", {{" key1 ", " value1 "}, {" key2 ", " value2 "}}},
        {"key1=value1!@#$%^&*(),key2=value2", {{"key1", "value1!@#$%^&*()"}, {"key2", "value2"}}}
    };
    for (const auto &test : testMap) {
        std::cout << "test case: " << test.first << std::endl;
        auto ret = ParseKeyValuePairs(test.first, ',');
        EXPECT_EQ(ret, test.second);
    }
}

/**
 * @tc.name: UserPathResolverTest_ReplaceAndCount_001
 * @tc.desc: Verify the ReplaceAndCount.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(StringUtilsTest, UserPathResolverTest_ReplaceAndCount_001, TestSize.Level1)
{
    std::map <std::tuple<std::string, std::string, std::string>, std::pair<std::string, int32_t>> testMap {
        {{"hello world, hello universe", "hello", "hi"}, {"hi world, hi universe", 2}},
        {{"", "hello", "hi"}, {"", 0}},
        {{"hello world", "", "hi"}, {"hello world", 0}},
        {{"hello world, hello universe", "hello", ""}, {" world,  universe", 2}},
        {{"hello world", "universe", "hi"}, {"hello world", 0}},
        {{"hello world", "hello", "hello"}, {"hello world", 1}},
        {{"hello world, hello", "hello", "hi"}, {"hi world, hi", 2}},
        {{"hello", "hello", "hi"}, {"hi", 1}},
        {{"hellohello", "hello", "hi"}, {"hihi", 2}},
        {{"hello@world, hello@universe", "@world", "@galaxy"}, {"hello@galaxy, hello@universe", 1}},
        {{"hello world, hello universe", "hello", "hellohello"}, {"hellohello world, hellohello universe", 2}},
    };
    for (const auto &test : testMap) {
        auto str = std::get<0>(test.first);
        std::cout << "test case: " << str << std::endl;
        auto count = ReplaceAndCount(str, std::get<1>(test.first), std::get<2>(test.first));
        EXPECT_EQ(count, test.second.second);
        EXPECT_EQ(str, test.second.first);
    }
}
} // Test
} // STORAGE_DAEMON
} // OHOS