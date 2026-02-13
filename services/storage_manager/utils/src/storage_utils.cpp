/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "utils/storage_utils.h"

namespace OHOS {
namespace StorageManager {
int64_t GetRoundSize(int64_t size)
{
    int64_t val = 1;
    int64_t multiple = UNIT;
    while (val * multiple < size) {
        auto tmpVal = static_cast<uint64_t>(val);
        tmpVal <<= 1;
        val = static_cast<int64_t>(tmpVal);
        if (val > THRESHOLD && multiple < ONE_GB) {
            val = 1;
            multiple *= UNIT;
        }
    }
    return val * multiple;
}

std::string GetAnonyString(const std::string &value)
{
    constexpr size_t INT32_SHORT_ID_LENGTH = 20;
    constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
    constexpr size_t INT32_MIN_ID_LENGTH = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

bool IsPathStartWithFileMgr(int32_t userId, const std::string &path)
{
    const std::string prefix = "/mnt/data/" + std::to_string(userId) + "/userExternal/";
    if (path.size() <= prefix.size()) {
        return false;
    }
    return path.compare(0, prefix.length(), prefix) == 0;
}
} // namespace STORAGE_Manager
} // namespace OHOS