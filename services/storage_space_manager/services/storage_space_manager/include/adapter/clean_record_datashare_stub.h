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
 
#ifndef CLEAN_RECORD_DATASHARE_STUB_H
#define CLEAN_RECORD_DATASHARE_STUB_H
 
#include "accesstoken_kit.h"
#include "datashare_stub.h"
#include "datashare_sa_provider_info.h"
#include "nlohmann/json.hpp"
 
namespace OHOS {
namespace StorageSpaceManager {
using namespace OHOS::DataShare;
using namespace OHOS::Security::AccessToken;
class CleanRecordDataShareStub : public DataShareStub {
public:
    explicit CleanRecordDataShareStub() {}
    virtual ~CleanRecordDataShareStub() {}
    DataShareNonSilentConfig GetConfig() override;
    int Insert(const Uri &uri, const DataShareValuesBucket &value) override;
    int Update(const Uri &uri, const DataSharePredicates &predicates, const DataShareValuesBucket &value) override;
    int Delete(const Uri &uri, const DataSharePredicates &predicates) override;
    std::shared_ptr<DataShareResultSet> Query(const Uri &uri, const DataSharePredicates &predicates,
        std::vector<std::string> &columns, DatashareBusinessError &businessError) override;
    int OpenFile(const Uri &uri, const std::string &mode) override {return 0;}
    int OpenRawFile(const Uri &uri, const std::string &mode) override {return 0;}
    std::string GetType(const Uri &uri) override {return "";}
    int BatchInsert(const Uri &uri, const std::vector<DataShareValuesBucket> &values) override {return 0;}
    bool RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override {return true;}
    bool UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver) override
        {return true;}
    bool NotifyChange(const Uri &uri) override {return true;}
    Uri NormalizeUri(const Uri &uri) override {return Uri("");}
    Uri DenormalizeUri(const Uri &uri) override {return Uri("");}
    std::vector<std::string> GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter) override
        {return std::vector<std::string>();}
private:
    std::string AnalyzingPredicates(const DataSharePredicates &predicates);
    int32_t HandlingIndividualConditions(const nlohmann::json &conditions, nlohmann::json &resultItem);
    std::shared_ptr<DataShareResultSet> BuildResultSet(std::string resultObj);
    NonSilentConfigRecord MatchConfig(const Uri &uri);
    bool CheckCallingPermission(const Uri &uri, bool isRead);
};
} // namespace StorageSpaceManager
} // namespace OHOS
#endif // CLEAN_RECORD_DATASHARE_STUB_H