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

#ifndef CLEAN_RECORD_STORE_H
#define CLEAN_RECORD_STORE_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <singleton.h>
#include <nocopyable.h>

#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store.h"
#include "datashare_result_set.h"
#include "rdb_predicates.h"

namespace OHOS {
namespace StorageSpaceManager {

class CleanRecordStore : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(CleanRecordStore);
public:
    int32_t Init();
    void Close();
    int64_t Insert(const NativeRdb::ValuesBucket& values);
    std::shared_ptr<NativeRdb::ResultSet> Get(int64_t startTime, int64_t endTime);
    std::shared_ptr<NativeRdb::ResultSet> QueryByResultString(const std::string &resultJson);
    int32_t Delete(int64_t time);

private:
    class CleanRecordDbOpenCallback : public NativeRdb::RdbOpenCallback {
    public:
        int OnCreate(NativeRdb::RdbStore &rdbStore) override;
        int OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion) override;
    };

    std::shared_ptr<NativeRdb::RdbStore> rdbStore_ = nullptr;
    std::mutex rdbMutex_;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // CLEAN_RECORD_STORE_H