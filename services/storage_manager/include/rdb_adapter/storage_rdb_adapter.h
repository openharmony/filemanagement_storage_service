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

#ifndef STORAGE_RDB_ADAPTER_H
#define STORAGE_RDB_ADAPTER_H

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "i_storage_rdb_adapter.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace StorageManager {
using namespace OHOS::NativeRdb;
class StorageRdbAdapter final : public StorageService::IRdbAdapter {
public:
    static StorageRdbAdapter &GetInstance();
    virtual ~StorageRdbAdapter() = default;

    int32_t Init() override;
    int32_t UnInit() override;
    int32_t GetRDBPtr() override;
    int32_t Put(int64_t &outRowId, const std::string &table, const ValuesBucket &Values) override;
    int32_t Delete(int32_t &deleteRows, const std::string &table, const std::string &whereClause,
        const std::vector<ValueObject> &bindArgs = {}) override;
    int32_t Update(int32_t &changedRows, const std::string &table, const ValuesBucket &values,
        const std::string &whereClause, const std::vector<ValueObject> &bindArgs = {}) override;
    std::shared_ptr<ResultSet> Get(const std::string &sql,
        const std::vector<ValueObject> &args = {}) override;

private:
    StorageRdbAdapter() = default;
    std::shared_ptr<RdbStore> store_ = nullptr;
    std::mutex rdbAdapterMtx_;
};

class OpenCallback : public RdbOpenCallback {
public:
    int32_t OnCreate(RdbStore &store) override;
    int32_t OnUpgrade(RdbStore &store, int oldVersion, int newVersion) override;
    int32_t CreateTable(RdbStore &store);
private:
    std::mutex rdbStoreMtx_;
};
} // namespace StorageManager
} // namespace OHOS
#endif // STORAGE_RDB_ADAPTER_H