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

#include <mutex>
#include <chrono>
#include <unistd.h>
#include <filesystem>
#include "storage_rdb_adapter.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageManager {
constexpr const char* STORAGE_MANAGER_RDB_PATH = "/data/service/el1/public/storage_manager/database/";
constexpr const char* STORAGE_MANAGER_DATABASE_NAME = "storage_manager_db.db";
constexpr int32_t RDB_VERSION = 1;
constexpr int32_t RDB_INIT_MAX_TIMES = 30;
constexpr int32_t RDB_INIT_INTERVAL_TIME = 100000;

StorageRdbAdapter &StorageRdbAdapter::GetInstance()
{
    static StorageRdbAdapter instance_;
    return instance_;
}

int32_t StorageRdbAdapter::Init()
{
    LOGI("rdb init start");
    int32_t retryTimes = RDB_INIT_MAX_TIMES;
    while (retryTimes > 0) {
        std::error_code errorCode;
        if (!std::filesystem::exists(STORAGE_MANAGER_RDB_PATH, errorCode)) {
            usleep(RDB_INIT_INTERVAL_TIME);
            retryTimes--;
            continue;
        }
        if (GetRDBPtr() == E_OK) {
            LOGI("rdb init success");
            return E_OK;
        }
        usleep(RDB_INIT_INTERVAL_TIME);
        retryTimes--;
    }
    LOGE("rdb init failed");
    return E_RDB_INIT_ERROR;
}

int32_t StorageRdbAdapter::UnInit()
{
    LOGI("rdb adapter unInit");
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    store_ = nullptr;
    return E_OK;
}

int32_t StorageRdbAdapter::GetRDBPtr()
{
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    if (store_ != nullptr) {
        return E_OK;
    }
    int32_t version = RDB_VERSION;
    OpenCallback helper;
    NativeRdb::RdbStoreConfig config(std::string(STORAGE_MANAGER_RDB_PATH) +
        std::string(STORAGE_MANAGER_DATABASE_NAME));
    int32_t errCode = E_OK;
    store_ = NativeRdb::RdbHelper::GetRdbStore(config, version, helper, errCode);
    if (store_ == nullptr) {
        LOGE("get rdb store failed, errCode is %{public}d", errCode);
        return E_RDB_STORE_NULL;
    }
    return E_OK;
}

int32_t StorageRdbAdapter::Put(int64_t &outRowId, const std::string &table, const NativeRdb::ValuesBucket &values)
{
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    if (store_ == nullptr) {
        LOGE("rdb store is null when put");
        return E_RDB_STORE_NULL;
    }
    int32_t ret = store_->Insert(outRowId, table, values);
    if (ret != E_OK) {
        LOGE("rdb adapter put failed when put, ret is %{public}d", ret);
        return E_TB_PUT_ERROR;
    }
    return E_OK;
}

int32_t StorageRdbAdapter::Delete(int32_t &deleteRows, const std::string &table, const std::string &whereClause,
    const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    if (store_ == nullptr) {
        LOGE("rdb store is null when delete");
        return E_RDB_STORE_NULL;
    }
    int32_t ret = store_->Delete(deleteRows, table, whereClause, bindArgs);
    if (ret != E_OK) {
        LOGE("rdb adapter delete failed when delete, ret is %{public}d", ret);
        return E_TB_DELETE_ERROR;
    }
    return E_OK;
}

int32_t StorageRdbAdapter::Update(int32_t &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
    const std::string &whereClause, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    if (store_ == nullptr) {
        LOGE("rdb store is null when update");
        return E_RDB_STORE_NULL;
    }
    int32_t ret = store_->Update(changedRows, table, values, whereClause, bindArgs);
    if (ret != E_OK) {
        LOGE("rdb adapter update failed when, ret is %{public}d", ret);
        return E_TB_UPDATE_ERROR;
    }
    return E_OK;
}

std::shared_ptr<NativeRdb::ResultSet> StorageRdbAdapter::Get(const std::string &sql,
    const std::vector<NativeRdb::ValueObject> &args)
{
    std::shared_ptr<NativeRdb::ResultSet> resultSet = nullptr;
    std::lock_guard<std::mutex> lock(rdbAdapterMtx_);
    if (store_ == nullptr) {
        LOGE("rdb store is null when get");
        return nullptr;
    }
    resultSet = store_->QueryByStep(sql, args);
    if (resultSet == nullptr) {
        LOGE("result set is null when get");
        return nullptr;
    }
    return resultSet;
}

int32_t OpenCallback::CreateTable(NativeRdb::RdbStore &store)
{
    std::lock_guard<std::mutex> lock(rdbStoreMtx_);
    std::string sql = StorageService::CREATE_BUNDLE_EXT_STATS_TABLE_SQL;
    int32_t errCode = store.ExecuteSql(sql);
    if (errCode != E_OK) {
        LOGE("create ext bundle stats table failed, errCode is %{public}d", errCode);
        return E_DB_CREATE_BUNDLE_TABLE_ERROR;
    }
    sql = StorageService::CREATE_CLEAN_NOTIFY_TABLE_SQL;
    errCode = store.ExecuteSql(sql);
    if (errCode != E_OK) {
        LOGE("create clean notify table failed, errCode is %{public}d", errCode);
        return E_DB_CREATE_CLEAN_NOTIFY_TABLE_ERROR;
    }
    return E_OK;
}

int32_t OpenCallback::OnCreate(NativeRdb::RdbStore &store)
{
    LOGI("rdb store create");
    int32_t ret = CreateTable(store);
    if (ret != E_OK) {
        LOGE("create table failed");
        return ret;
    }
    LOGI("rdb store create success");
    return E_OK;
}

int32_t OpenCallback::OnUpgrade(NativeRdb::RdbStore &store, int oldVersion, int newVersion)
{
    LOGI("rdb store upgrade, old version : %{public}d, new version : %{public}d", oldVersion, newVersion);
    return E_OK;
}

} // namespace StorageManager
} // namespace OHOS