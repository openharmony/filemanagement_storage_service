/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "datashare_result_set.h"
#include "datashare_result_set_mock.h"
#include "storage_service_log.h"

namespace OHOS {
namespace DataShare {
int32_t DataShareResultSet::GetColumnIndex(const std::string &columnName, int32_t &columnIndex)
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GetColumnIndex(columnName, columnIndex);
    }
    return 0;
}

int32_t DataShareResultSet::GetString(int32_t columnIndex, std::string &value)
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GetString(columnIndex, value);
    }
    return 0;
}

int32_t DataShareResultSet::GetLong(int32_t columnIndex, int64_t &value)
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GetLong(columnIndex, value);
    }
    return 0;
}

int32_t DataShareResultSet::GetInt(int32_t columnIndex, int32_t &value)
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GetInt(columnIndex, value);
    }
    return 0;
}

int32_t DataShareResultSet::GoToFirstRow()
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GoToFirstRow();
    }
    return 0;
}

int32_t DataShareResultSet::GoToNextRow()
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GoToNextRow();
    }
    return 0;
}

int32_t DataShareResultSet::Close()
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->Close();
    }
    return 0;
}

int32_t DataShareResultSet::GetRowCount(int32_t &count)
{
    if (DataShareResultSetMock::proxy_ != nullptr) {
        return DataShareResultSetMock::proxy_->GetRowCount(count);
    }
    return -1;
}
} // namespace OHOS
} // namespace DataShare