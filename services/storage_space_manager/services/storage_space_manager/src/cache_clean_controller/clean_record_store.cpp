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

#include "cache_clean_controller/clean_record_store.h"

#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"

namespace OHOS {
namespace StorageSpaceManager {

namespace {
constexpr const char *DATABASE_NAME = "app_cache_clean_record.db";
constexpr const char *DATA_DIR = "/data/service/el1/public/database/storage_space_manager";
constexpr int32_t DATABASE_VERSION = 1;

constexpr const char *CREATE_CLEAN_RECORD_TABLE_SQL =
    "CREATE TABLE IF NOT EXISTS app_cache_clean_record ("
    "id INTEGER PRIMARY KEY AUTOINCREMENT, "
    "clean_time BIGINT NOT NULL DEFAULT 0, "
    "freed_size BIGINT NOT NULL DEFAULT 0, "
    "clean_before BIGINT NOT NULL DEFAULT 0, "
    "clean_after BIGINT NOT NULL DEFAULT 0)";
} // namespace

CleanRecordStore::CleanRecordStore() {}

CleanRecordStore::~CleanRecordStore() {}

int32_t CleanRecordStore::Init()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ != nullptr) {
        LOGI("RDB store already initialized");
        return E_OK;
    }

    NativeRdb::RdbStoreConfig config(DATA_DIR + std::string("/") + DATABASE_NAME);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    config.SetAllowRebuild(true);

    CleanRecordDbOpenCallback callback;
    int32_t errCode = E_OK;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_VERSION, callback, errCode);
    if (rdbStore_ == nullptr || errCode != E_OK) {
        LOGE("Failed to create/open RDB store, errCode=%{public}d", errCode);
        rdbStore_ = nullptr;
        return E_IO_ERROR;
    }

    LOGI("RDB store initialized successfully");
    return E_OK;
}

void CleanRecordStore::Close()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ != nullptr) {
        rdbStore_ = nullptr;
        LOGI("RDB store closed");
    }
}

int64_t CleanRecordStore::Insert(const NativeRdb::ValuesBucket& values)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        LOGE("RDB store is not initialized");
        return E_IO_ERROR;
    }

    int64_t rowId = -1;
    int32_t ret = rdbStore_->Insert(rowId, "app_cache_clean_record", values);
    if (ret != E_OK) {
        LOGE("Failed to insert into app_cache_clean_record, ret=%{public}d", ret);
        return ret;
    }

    LOGI("Inserted record, rowId=%{public}lld", static_cast<long long>(rowId));
    return rowId;
}

std::shared_ptr<NativeRdb::ResultSet> CleanRecordStore::Get(int64_t startTime, int64_t endTime)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        LOGE("RDB store is not initialized");
        return nullptr;
    }
    const std::string sql = "SELECT * FROM app_cache_clean_record WHERE clean_time > ? AND clean_time < ?";
    std::vector<NativeRdb::ValueObject> condition;
    condition.emplace_back(NativeRdb::ValueObject(startTime));
    condition.emplace_back(NativeRdb::ValueObject(endTime));
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;

    resultSet = rdbStore_->QueryByStep(sql, condition);
    if (resultSet == nullptr) {
        LOGE("Failed to query app_cache_clean_record");
        return nullptr;
    }

    return resultSet;
}

std::shared_ptr<NativeRdb::ResultSet> CleanRecordStore::QueryByResultString(const std::string &resultJson)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        LOGE("RDB store is not initialized");
        return nullptr;
    }

    int ret = rdbStore_->ExecuteSql(
        "CREATE TABLE IF NOT EXISTS query_result_record (query_result TEXT NOT NULL)");
    if (ret != E_OK) {
        LOGE("Failed to create query_result_record table, ret=%{public}d", ret);
        return nullptr;
    }
    rdbStore_->ExecuteSql("DELETE FROM query_result_record");

    NativeRdb::ValuesBucket values;
    values.PutString("query_result", resultJson);
    int64_t rowId = -1;
    ret = rdbStore_->Insert(rowId, "query_result_record", values);
    if (ret != E_OK) {
        LOGE("rdbStore_ Insert fail ret:%{public}d", ret);
        return nullptr;
    }

    NativeRdb::RdbPredicates predicates("query_result_record");
    auto resultSet = rdbStore_->Query(predicates, {"query_result"});
    if (resultSet == nullptr) {
        LOGE("Failed to query query_result_record");
        return nullptr;
    }
    int32_t rowCount = -1;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != E_OK || rowCount == 0) {
        LOGE("rdbStore_ Query fail");
        return nullptr;
    }

    return resultSet;
}

int32_t CleanRecordStore::Delete(int64_t time)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ == nullptr) {
        LOGE("RDB store is not initialized");
        return E_IO_ERROR;
    }

    int32_t deletedRows = 0;
    std::vector<NativeRdb::ValueObject> condition;
    condition.emplace_back(NativeRdb::ValueObject(time));
    int32_t ret = rdbStore_->Delete(deletedRows, "app_cache_clean_record", "clean_time < ?", condition);
    if (ret != E_OK) {
        LOGE("Failed to delete from app_cache_clean_record, ret=%{public}d", ret);
        return ret;
    }

    LOGI("Deleted %{public}d rows from app_cache_clean_record", deletedRows);
    return deletedRows;
}

int CleanRecordStore::CleanRecordDbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    LOGI("Creating app_cache_clean_record table");
    int ret = rdbStore.ExecuteSql(CREATE_CLEAN_RECORD_TABLE_SQL);
    if (ret != E_OK) {
        LOGE("Failed to create app_cache_clean_record table, ret=%{public}d", ret);
    }
    return ret;
}

int CleanRecordStore::CleanRecordDbOpenCallback::OnUpgrade(NativeRdb::RdbStore &rdbStore,
    int currentVersion, int targetVersion)
{
    LOGI("Upgrading app_cache_clean_record database from version %{public}d : %{public}d",
        currentVersion, targetVersion);
    return E_OK;
}

} // namespace StorageSpaceManager
} // namespace OHOS