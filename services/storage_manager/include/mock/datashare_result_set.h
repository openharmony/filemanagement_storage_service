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

#ifndef DATASHARE_RESULT_SET_H
#define DATASHARE_RESULT_SET_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace DataShare {
class DataShareResultSet {
public:
    static int32_t GetColumnIndex(const std::string &columnName, int32_t &columnIndex);
    static int32_t GetString(int32_t columnIndex, std::string &value);
    static int32_t GetLong(int32_t columnIndex, int64_t &value);
    static int32_t GetInt(int32_t columnIndex, int32_t &value);
    static int32_t GoToFirstRow();
    static int32_t GoToNextRow();
    static int32_t Close();
    static int32_t GetRowCount(int32_t &count);
};
} // namespace OHOS
} // namespace DataShare
#endif  // DATASHARE_RESULT_SET_H