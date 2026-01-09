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
#ifndef STORAGE_DAEMON_UTILS_STRING_UTILS_H
#define STORAGE_DAEMON_UTILS_STRING_UTILS_H

#include <list>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t BASE_OCTAL = 8;
constexpr int32_t BASE_DECIMAL = 10;

std::string StringPrintf(const char *format, ...);

inline bool IsEndWith(const std::string &str, const std::string &end)
{
    return str.size() >= end.size() && str.substr(str.size() - end.size()) == end;
}

std::vector<std::string> SplitLine(std::string &line, std::string &token);
bool WriteFileSync(const char *path, const uint8_t *data, size_t size, std::string &errMsg);
bool SaveStringToFileSync(const std::string &path, const std::string &data, std::string &errMsg);
bool StringIsNumber(const std::string &content);
bool IsStringExist(const std::list<std::string> &strList, const std::string &content);
std::string ListToString(const std::list<std::string> &strList);
void GetAllUserIds(std::vector<int32_t> &userIds);
bool ConvertStringToInt(const std::string &str, int64_t &value, int32_t base = BASE_DECIMAL);
std::unordered_map<std::string, std::string> ParseKeyValuePairs(const std::string &input, char delimiter);
int32_t ReplaceAndCount(std::string &str, const std::string &target, const std::string &replacement);
bool ConvertStringToInt32(const std::string &context, int32_t &value);
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_UTILS_STRING_UTILS_H
