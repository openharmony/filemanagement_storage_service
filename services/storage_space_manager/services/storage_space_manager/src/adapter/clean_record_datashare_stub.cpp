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

#include "clean_record_datashare_stub.h"
#include "storage_space_manager_errno.h"
#include "storage_space_manager_hilog.h"
#include "storage_space_manager_provider.h"
#include "ipc_skeleton.h"
#include "clean_record_store.h"
#include "rdb_utils.h"

namespace OHOS {
namespace StorageSpaceManager {
using namespace OHOS::DataShare;
using namespace OHOS::Security::AccessToken;
constexpr int32_t SINGLE_PARAMS_SIZE = 3;
DataShareNonSilentConfig CleanRecordDataShareStub::GetConfig()
{
    NonSilentConfigRecord record = {
        "datashare://StorageSpaceMgr/SAID=8650/app_cache_clean_record",
        "ohos.permission.STORAGE_MANAGER",
        "ohos.permission.STORAGE_MANAGER"
    };
    DataShareNonSilentConfig saconfig;
    saconfig.records.push_back(record);
    return saconfig;
}

bool CleanRecordDataShareStub::CheckCallingPermission(const Uri &uri, bool isRead)
{
    NonSilentConfigRecord record = MatchConfig(uri);
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (isRead) {
        if (!record.readPermission.empty()) {
            auto status = AccessTokenKit::VerifyAccessToken(tokenId, record.readPermission);
            if (status == PermissionState::PERMISSION_GRANTED) {
                LOGI("check permission success!");
                return true;
            }
        }
    } else {
        if (!record.writePermission.empty()) {
            auto status = AccessTokenKit::VerifyAccessToken(tokenId, record.writePermission);
            if (status == PermissionState::PERMISSION_GRANTED) {
                LOGI("check permission success!");
                return true;
            }
        }
    }
    LOGE("check permission failed!");
    return false;
}

NonSilentConfigRecord CleanRecordDataShareStub::MatchConfig(const Uri &uri)
{
    auto config = GetConfig();
    for (auto &record : config.records) {
        if (uri.ToString() == record.uri) {
            return record;
        }
    }
    return {"", "", ""};
}

int32_t CleanRecordDataShareStub::Insert(const Uri &uri, const DataShareValuesBucket &value)
{
    if (CheckCallingPermission(uri, false)) {
        LOGI("Insert not supported!");
        return E_OK;
    }
    LOGE("no permission!");
    return E_PERMISSION_DENIED;
}

int32_t CleanRecordDataShareStub::Update(const Uri &uri,
    const DataSharePredicates &predicates, const DataShareValuesBucket &value)
{
    if (CheckCallingPermission(uri, false)) {
        LOGE("Update not supported!");
        return E_OK;
    }
    LOGE("no permission!");
    return E_PERMISSION_DENIED;
}

int32_t CleanRecordDataShareStub::Delete(const Uri &uri, const DataSharePredicates &predicates)
{
    if (CheckCallingPermission(uri, false)) {
        LOGE("Delete not supported!");
        return E_OK;
    }
    LOGE("no permission!");
    return E_PERMISSION_DENIED;
}

std::string CleanRecordDataShareStub::AnalyzingPredicates(const DataSharePredicates &predicates)
{
    const auto operations = predicates.GetOperationList();
    if (operations.size() != 1) {
        LOGE("predicates param num error!");
        return "";
    }

    OperationItem oper = operations[0];
    if (oper.singleParams.size() != SINGLE_PARAMS_SIZE) {
        LOGE("singleParams param num error!");
        return "";
    }
    std::string singleValue = oper.GetSingle(1);
    return singleValue;
}

int32_t CleanRecordDataShareStub::HandlingIndividualConditions(const nlohmann::json &conditions,
    nlohmann::json &resultItem)
{
    int32_t week_index = 0;
    int64_t startTime = 0;
    int64_t endTime = 0;
    if (conditions.contains("week_index") && conditions["week_index"].is_number_integer() &&
        conditions.contains("begin_time") && conditions["begin_time"].is_number_integer() &&
        conditions.contains("end_time") && conditions["end_time"].is_number_integer()) {
        week_index = conditions["week_index"].get<int32_t>();
        startTime = conditions["begin_time"].get<int64_t>();
        endTime = conditions["end_time"].get<int64_t>();
    } else {
        LOGE("conditions data error");
        return E_INVALID_ARGUMENT;
    }
    std::shared_ptr<NativeRdb::ResultSet> resultSet =
        DelayedSingleton<CleanRecordStore>::GetInstance()->Get(startTime, endTime);
    if (resultSet == nullptr) {
        LOGE("resultSet is nullptr");
        return E_FAIL;
    }
    int32_t rowCount = -1;
    int64_t revenue = 0;
    int32_t columnIndex = 0;
    auto ret = resultSet->GetRowCount(rowCount);
    if (ret != E_OK || rowCount == 0) {
        LOGE("by condition not find data");
    } else {
        while (resultSet->GoToNextRow() == E_OK) {
            int64_t freedSize = 0;
            resultSet->GetColumnIndex("freed_size", columnIndex);
            resultSet->GetLong(columnIndex, freedSize);
            revenue += freedSize;
        }
    }
    resultSet->Close();
    resultItem["week_index"] = week_index;
    resultItem["revenue"] = revenue;
    return E_OK;
}

std::shared_ptr<DataShareResultSet> CleanRecordDataShareStub::BuildResultSet(std::string resultObj)
{
    auto rdbResultSet = DelayedSingleton<CleanRecordStore>::GetInstance()->QueryByResultString(resultObj);
    if (rdbResultSet == nullptr) {
        LOGE("QueryByResultString failed");
        return nullptr;
    }
    auto ResultSetBridge = RdbDataShareAdapter::RdbUtils::ToResultSetBridge(rdbResultSet);
    std::shared_ptr<DataShareResultSet> dataShareResultSet = std::make_shared<DataShareResultSet>(ResultSetBridge);
    return dataShareResultSet;
}

std::shared_ptr<DataShareResultSet> CleanRecordDataShareStub::Query(const Uri &uri,
    const DataSharePredicates &predicates, std::vector<std::string> &columns, DatashareBusinessError &businessError)
{
    auto provider = DelayedSingleton<StorageSpaceManagerProvider>::GetInstance();
    provider->AddRunningIpcCount();
    if (!CheckCallingPermission(uri, true)) {
        businessError.SetCode(E_PERMISSION_DENIED);
        provider->SubtractRunningIpcCount();
        return nullptr;
    }
    std::string singleValue = AnalyzingPredicates(predicates);
    if (singleValue.empty() || !nlohmann::json::accept(singleValue)) {
        LOGE("predicates param error");
        businessError.SetCode(E_INVALID_ARGUMENT);
        provider->SubtractRunningIpcCount();
        return nullptr;
    }
    nlohmann::json predicatesObj = nlohmann::json::parse(singleValue, nullptr, false);
    if (predicatesObj.is_discarded() ||!predicatesObj.contains("conditions")) {
        LOGE("singleValue parse fail");
        businessError.SetCode(E_INVALID_ARGUMENT);
        provider->SubtractRunningIpcCount();
        return nullptr;
    }
    auto conditions = predicatesObj["conditions"];
    if (!conditions.is_array()) {
        LOGE("conditions is not an array");
        businessError.SetCode(E_INVALID_ARGUMENT);
        provider->SubtractRunningIpcCount();
        return nullptr;
    }
    nlohmann::json resultArray = nlohmann::json::array();
    for (const auto &cond : conditions) {
        nlohmann::json resultItem;
        int32_t ret = HandlingIndividualConditions(cond, resultItem);
        if (ret != E_OK) {
            businessError.SetCode(ret);
            provider->SubtractRunningIpcCount();
            return nullptr;
        }
        resultArray.push_back(resultItem);
    }
    nlohmann::json resultObj;
    resultObj["weekly_revenue"] = resultArray;
    LOGI("query result: %{public}s", resultObj.dump().c_str());

    std::shared_ptr<DataShareResultSet> dataShareResultSet = BuildResultSet(resultObj.dump());
    if (dataShareResultSet == nullptr) {
        businessError.SetCode(E_FAIL);
    }
    provider->SubtractRunningIpcCount();
    return dataShareResultSet;
}
} // namespace StorageSpaceManager
} // namespace OHOS