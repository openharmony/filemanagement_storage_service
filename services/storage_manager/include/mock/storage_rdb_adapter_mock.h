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
#ifndef STORAGE_RDB_ADAPTER_MOCK_H
#define STORAGE_RDB_ADAPTER_MOCK_H

#include "gmock/gmock.h"
#include "rdb_helper.h"
#include "rdb_store.h"

namespace OHOS {
namespace StorageManager {
namespace Moc {
static std::shared_ptr<OHOS::NativeRdb::RdbStore> g_mockRdbStore = nullptr;
}
inline void MockGetRdbStore(std::shared_ptr<OHOS::NativeRdb::RdbStore> mockRdbStore)
{
    Moc::g_mockRdbStore = mockRdbStore;
}
inline void MockSetRdbStore()
{
    Moc::g_mockRdbStore = nullptr;
}

class MockResultSet : public OHOS::NativeRdb::AbsSharedResultSet {
public:
    MOCK_METHOD(int, GoToFirstRow, (), (override));
    MOCK_METHOD(int, GoToNextRow, (), (override));
    MOCK_METHOD(int, GetString, (int columnIndex, std::string &value), (override));
    MOCK_METHOD(int, GetInt, (int columnIndex, int &value), (override));
    MOCK_METHOD(int, Close, (), (override));
    MOCK_METHOD(int, GetColumnCount, (int &count), (override));
    MOCK_METHOD(int, GetRowCount, (int &count), (override));
};
class MockRdbStore : public OHOS::NativeRdb::RdbStore {
public:
    MockRdbStore() = default;
    MOCK_METHOD(int, ExecuteSql, (const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs),
        (override));
    MOCK_METHOD(int, InsertWithConflictResolution,
        (int64_t  &outRowId, const std::string &table, const NativeRdb::ValuesBucket &values,
            NativeRdb::ConflictResolution conflictResolution),
        (override));

    MOCK_METHOD(int, Delete, (int &deletedRows, const std::string &table, const std::string &whereClause,
        const std::vector<NativeRdb::ValueObject> &bindArgs),
        (override));

    MOCK_METHOD(std::shared_ptr<NativeRdb::AbsSharedResultSet>, Query,
        (const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns), (override));
    MOCK_METHOD(int, BeginTransaction, (), (override));
    MOCK_METHOD(int, RollBack, (), (override));
    MOCK_METHOD(int, Commit, (), (override));
    MOCK_METHOD(int, Insert, (int64_t &outRowId, const std::string &table,
        const NativeRdb::ValuesBucket &initialValues), (override));
    MOCK_METHOD2(Restore, int(const std::string&, const std::vector<uint8_t>&));
    MOCK_METHOD(int, Update,
        (int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
            const std::string &whereClause, const std::vector<NativeRdb::ValueObject> &bindArgs),
        (override));
    MOCK_METHOD(std::shared_ptr<NativeRdb::ResultSet>, QueryByStep,
        (const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs, bool preCount),
        (override));
    ~MockRdbStore() override = default;

    int BatchInsert(int64_t &outInsertNum, const std::string &table,
        const std::vector<NativeRdb::ValuesBucket> &initialBatchValues) override;
    int Replace(int64_t &outRowId, const std::string &table, const NativeRdb::ValuesBucket &initialValues) override;

    int Update(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<std::string> &whereArgs) override;
    int UpdateWithConflictResolution(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<std::string> &whereArgs,
        NativeRdb::ConflictResolution conflictResolution) override;
    int UpdateWithConflictResolution(int &changedRows, const std::string &table, const NativeRdb::ValuesBucket &values,
        const std::string &whereClause, const std::vector<NativeRdb::ValueObject> &bindArgs,
        NativeRdb::ConflictResolution conflictResolution) override;

    std::shared_ptr<NativeRdb::AbsSharedResultSet> Query(int &errCode, bool distinct, const std::string &table,
        const std::vector<std::string> &columns, const std::string &whereClause,
        const std::vector<NativeRdb::ValueObject> &bindArgs, const std::string &groupBy, const std::string &indexName,
        const std::string &orderBy, const int &limit, const int &offset) override;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> QuerySql(
        const std::string &sql, const std::vector<std::string> &selectionArgs) override;
    std::shared_ptr<NativeRdb::AbsSharedResultSet> QuerySql(
        const std::string &sql, const std::vector<NativeRdb::ValueObject> &selectionArgs) override;
    std::shared_ptr<NativeRdb::ResultSet> QueryByStep(
        const std::string &sql, const std::vector<std::string> &selectionArgs) override;

    int ExecuteAndGetLong(int64_t &outValue, const std::string &sql,
        const std::vector<NativeRdb::ValueObject> &bindArgs) override;
    int ExecuteAndGetString(
        std::string &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs) override;
    int ExecuteForLastInsertedRowId(
        int64_t &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs) override;
    int ExecuteForChangedRowCount(
        int64_t &outValue, const std::string &sql, const std::vector<NativeRdb::ValueObject> &bindArgs) override;
    int Backup(const std::string &databasePath, const std::vector<uint8_t> &destEncryptKey) override;
    int Attach(
        const std::string &alias, const std::string &pathName, const std::vector<uint8_t> destEncryptKey) override;

    int Count(int64_t &outValue, const NativeRdb::AbsRdbPredicates &predicates) override;

    std::shared_ptr<NativeRdb::ResultSet> QueryByStep(
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns, bool preCount) override;
    std::shared_ptr<NativeRdb::ResultSet> RemoteQuery(const std::string &device,
        const NativeRdb::AbsRdbPredicates &predicates, const std::vector<std::string> &columns, int &errCode) override;
    int Update(int &changedRows, const NativeRdb::ValuesBucket &values,
        const NativeRdb::AbsRdbPredicates &predicates) override;

    virtual int GetStatus();
    virtual void SetStatus(int status);
    int GetVersion(int &version) override;
    int SetVersion(int version) override;

    bool IsInTransaction() override;
    std::string GetPath() override;
    bool IsHoldingConnection() override;
    bool IsOpen() const override;
    bool IsReadOnly() const override;
    bool IsMemoryRdb() const override;
    virtual int ChangeDbFileForRestore(
        const std::string newPath, const std::string backupPath, const std::vector<uint8_t> &newKey);

    int SetDistributedTables(const std::vector<std::string> &tables, int type,
        const DistributedRdb::DistributedConfig &distributedConfig) override;

    std::string ObtainDistributedTableName(const std::string &device, const std::string &table, int &errCode) override;

    int Sync(const SyncOption &option, const NativeRdb::AbsRdbPredicates &predicate, const AsyncBrief &async) override;

    int Sync(const SyncOption &option, const NativeRdb::AbsRdbPredicates &predicate, const AsyncDetail &async) override;

    int Sync(const SyncOption &option, const std::vector<std::string> &tables, const AsyncDetail &async) override;

    int Subscribe(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer) override;

    int UnSubscribe(const SubscribeOption &option, std::shared_ptr<RdbStoreObserver> observer) override;

    int RegisterAutoSyncCallback(std::shared_ptr<DetailProgressObserver> syncObserver) override;

    int UnregisterAutoSyncCallback(std::shared_ptr<DetailProgressObserver> syncObserver) override;

    int Notify(const std::string &event) override;

    virtual bool DropDeviceData(const std::vector<std::string> &devices, const DropOption &option);

    ModifyTime GetModifyTime(
        const std::string &table, const std::string &columnName, std::vector<PRIKey> &keys) override;

    int CleanDirtyData(const std::string &table, uint64_t cursor) override;

    int GetRebuilt(NativeRdb::RebuiltType &rebuilt) override;
};
} // namespace StorageManager
} // namespace OHOS

namespace OHOS::NativeRdb {
inline std::shared_ptr<RdbStore> RdbHelper::GetRdbStore(
    const RdbStoreConfig &config, int version, RdbOpenCallback &openCallback, int &errCode)
{
    return OHOS::StorageManager::Moc::g_mockRdbStore;
}
} // namespace OHOS::NativeRdb


#endif  // STORAGE_RDB_ADAPTER_MOCK_H