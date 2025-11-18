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

#ifndef I_STORAGE_RDB_ADAPTER_H
#define I_STORAGE_RDB_ADAPTER_H

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "rdb_helper.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace StorageService {

class IRdbAdapter {
public:
    virtual ~IRdbAdapter() = default;
    
    virtual int32_t Init() = 0;
    virtual int32_t UnInit() = 0;
    virtual int32_t GetRDBPtr() = 0;
    virtual int32_t Put(int64_t &outRowId, const std::string &table, const NativeRdb::ValuesBucket &Values) = 0;
    virtual int32_t Delete(int32_t &deleteRows, const std::string &table, const std::string &whereClause,
        const std::vector<NativeRdb::ValueObject> &bindArgs) = 0;
    virtual int32_t Update(int32_t &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<NativeRdb::ValueObject> &bindArgs) = 0;
    virtual std::shared_ptr<NativeRdb::ResultSet> Get(const std::string &sql,
        const std::vector<NativeRdb::ValueObject> &args) = 0;
};

} // namespace StorageService
} // namespace OHOS
#endif // I_STORAGE_RDB_ADAPTER_H