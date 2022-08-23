/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
std::string StringPrintf(const char *format, ...);

inline bool IsEndWith(const std::string &str, const std::string &end)
{
    return str.size() >= end.size() && str.substr(str.size() - end.size()) == end;
}

std::vector<std::string> SplitLine(std::string &line, std::string &token);
bool WriteFileSync(const char *path, const uint8_t *data, size_t size);
bool SaveStringToFileSync(const std::string &path, const std::string &data);
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_DAEMON_UTILS_STRING_UTILS_H
