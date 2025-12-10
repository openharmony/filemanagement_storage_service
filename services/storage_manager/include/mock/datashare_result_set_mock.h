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

#ifndef DATASHARE_RESULT_SET_MOCK_H
#define DATASHARE_RESULT_SET_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "datashare_result_set.h"

namespace OHOS {
namespace DataShare {
class DataShareResultSetMock : public DataShareResultSet {
public:
    MOCK_METHOD2(GetColumnIndex, int32_t(const std::string &, int32_t &));
    MOCK_METHOD2(GetString, int32_t(int32_t, std::string &));
    MOCK_METHOD2(GetLong, int32_t(int32_t, int64_t &));
    MOCK_METHOD2(GetInt, int32_t(int32_t, int32_t &));
    MOCK_METHOD0(GoToFirstRow, int32_t());
    MOCK_METHOD0(GoToNextRow, int32_t());
    MOCK_METHOD0(Close, int32_t());
    MOCK_METHOD1(GetRowCount, int32_t(int32_t &));
    static inline std::shared_ptr<DataShareResultSetMock> proxy_ = nullptr;
};
} // namespace OHOS
} // namespace DataShare
#endif  // DATASHARE_RESULT_SET_MOCK_H