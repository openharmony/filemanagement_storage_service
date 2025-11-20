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

#include "mock/storage_rdb_adapter_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageManager {

int MockRdbStore::BatchInsert(
    int64_t &outInsertNum, const std::string &table, const std::vector<NativeRdb::ValuesBucket> &initialBatchValues)
{
    return E_ERR;
}
int MockRdbStore::Replace(int64_t &outRowId, const std::string &table, const NativeRdb::ValuesBucket &initialValues)
{
    return E_ERR;
}

int MockRdbStore::Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
    const std::string &whereClause, const std::vector<std::string> &whereArgs)
{
    return E_ERR;
}
int MockRdbStore::UpdateWithConflictResolution(int &changedRows, const std::string &table,
    const NativeRdb::ValuesBucket &values, const std::string &whereClause, const std::vector<std::string> &whereArgs,
    NativeRdb::ConflictResolution conflictResolution)
{
    return E_ERR;
}
int MockRdbStore::UpdateWithConflictResolution(int &changedRows, const std::string &table,
    const NativeRdb::ValuesBucket &values, const std::string &whereClause,
    const std::vector<NativeRdb::ValueObject> &bindArgs, NativeRdb::ConflictResolution conflictResolution)
{
    return E_ERR;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> MockRdbStore::Query(int &errCode, bool distinct,
    const std::string &table, const std::vector<std::string> &columns, const std::string &whereClause,
    const std::vector<NativeRdb::ValueObject> &bindArgs, const std::string &groupBy, const std::string &indexName,
    const std::string &orderBy, const int &limit, const int &offset)
{
    return nullptr;
}
std::shared_ptr<NativeRdb::AbsSharedResultSet> MockRdbStore::QuerySql(
    const std::string &sql, const std::vector<std::string> &selectionArgs)
{
    return nullptr;
}
std::shared_ptr<NativeRdb::AbsSharedResultSet> MockRdbStore::QuerySql(
    const std::string &sql, const std::vector<NativeRdb::ValueObject> &selectionArgs)
{
    return nullptr;
}

int MockRdbStore::ExecuteAndGetLong(int64_t &outValue, const std::string &sql,
    const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_ERR;
}
int MockRdbStore::ExecuteAndGetString(
    std::string &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_ERR;
}
int MockRdbStore::ExecuteForLastInsertedRowId(
    int64_t &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_ERR;
}
int MockRdbStore::ExecuteForChangedRowCount(
    int64_t &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs)
{
    return E_ERR;
}
int MockRdbStore::Backup(const std::string &databasePath, const std::vector<uint8_t> &destEncryptKey)
{
    return E_ERR;
}
int MockRdbStore::Attach(
    const std::string &alias, const std::string &pathName, const std::vector<uint8_t> destEncryptKey)
{
    return E_ERR;
}

int MockRdbStore::Count(int64_t &outValue, const NativeRdb::AbsRdbPredicates &predicates)
{
    return E_ERR;
}

std::shared_ptr<NativeRdb::ResultSet> MockRdbStore::QueryByStep(
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns, bool preCount)
{
    return nullptr;
}
std::shared_ptr<NativeRdb::ResultSet> MockRdbStore::RemoteQuery(const std::string &device,
    const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns, int &errCode)
{
    return nullptr;
}
int MockRdbStore::Update(int &changedRows, const NativeRdb::ValuesBucket &values,
    const NativeRdb::AbsRdbPredicates &predicates)
{
    return E_ERR;
}

int MockRdbStore::GetStatus()
{
    return E_ERR;
}
void MockRdbStore::SetStatus(int status) {}
int MockRdbStore::GetVersion(int &version)
{
    return E_ERR;
}
int MockRdbStore::SetVersion(int version)
{
    return E_ERR;
}

bool MockRdbStore::IsInTransaction()
{
    return false;
}
std::string MockRdbStore::GetPath()
{
    return "";
}

bool MockRdbStore::IsHoldingConnection()
{
    return false;
}

bool MockRdbStore::IsOpen() const
{
    return false;
}

bool MockRdbStore::IsReadOnly() const
{
    return false;
}

bool MockRdbStore::IsMemoryRdb() const
{
    return false;
}

int MockRdbStore::SetDistributedTables(const std::vector<std::string> &tables, int type,
    const DistributedRdb::DistributedConfig &distributedConfig)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::Sync(const SyncOption &option, const NativeRdb::AbsRdbPredicates &predicate, const AsyncBrief &async)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::Sync(const SyncOption &option, const NativeRdb::AbsRdbPredicates &predicate, const AsyncDetail &async)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::Sync(const SyncOption &option, const std::vector<std::string> &tables, const AsyncDetail &async)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::Subscribe(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::UnSubscribe(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::RegisterAutoSyncCallback(std::shared_ptr<DetailProgressObserver> syncObserver)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::UnregisterAutoSyncCallback(std::shared_ptr<DetailProgressObserver> syncObserver)
{
    return NativeRdb::E_ERROR;
}

int MockRdbStore::Notify(const std::string &event)
{
    return NativeRdb::E_ERROR;
}

bool MockRdbStore::DropDeviceData(const std::vector<std::string> &devices, const DropOption &option)
{
    return false;
}

MockRdbStore::ModifyTime MockRdbStore::GetModifyTime(
    const std::string &table, const std::string &columnName, std::vector<PRIKey> &keys)
{
    return {};
}

int MockRdbStore::CleanDirtyData(const std::string &table, uint64_t cursor)
{
    return NativeRdb::E_ERROR;
};

int MockRdbStore::GetRebuilt(NativeRdb::RebuiltType &rebuilt)
{
    return E_OK;
}

int MockRdbStore::ChangeDbFileForRestore(
    const std::string newPath, const std::string backupPath, const std::vector<uint8_t> &newKey)
{
    return NativeRdb::E_ERROR;
}

std::string MockRdbStore::ObtainDistributedTableName(const std::string &device, const std::string &table, int &errCode)
{
    return "";
}
} // namespace StorageManager
} // namespace OHOS