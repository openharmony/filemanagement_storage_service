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

#include "storage/app_status_manager.h"

#include "iservice_registry.h"
#include "int_wrapper.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
using namespace OHOS::StorageService;
static constexpr int32_t WANT_DEFAULT_VALUE = -1;
BmsSubscriber::BmsSubscriber(const EventFwk::CommonEventSubscribeInfo &info) : EventFwk::CommonEventSubscriber(info) {}

AppStatusManager &AppStatusManager::GetInstance()
{
    static AppStatusManager instance;
    return instance;
}

bool AppStatusManager::SubscribeCommonEvent(void)
{
    LOGI("bmsSubscriber start");
    if (bmsSubscriber == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        bmsSubscriber = std::make_shared<BmsSubscriber>(subscribeInfo);
        if (!EventFwk::CommonEventManager::SubscribeCommonEvent(bmsSubscriber)) {
            bmsSubscriber = nullptr;
            LOGE("bms subscribe common event failed.");
            return false;
        }
    }
    LOGI("bmsSubscriber success");
    return true;
}

int32_t AppStatusManager::DelBundleExtStats(int32_t userId, std::string &businessName)
{
    StorageRdbAdapter &rdbAdapter = StorageRdbAdapter::GetInstance();
    std::vector<NativeRdb::ValueObject> values;
    values.emplace_back(NativeRdb::ValueObject(businessName));
    values.emplace_back(NativeRdb::ValueObject(userId));
    int32_t deleteRows = ROWCOUNT_INIT;
    int32_t ret = rdbAdapter.Delete(deleteRows, BUNDLE_EXT_STATS_TABLE, WHERE_CLAUSE, values);
    if (ret != OHOS::E_OK) {
        LOGE("DelBundleExtStats failed.");
        std::string extraData = "errCode=" + std::to_string(ret);
        StorageRadar::ReportSpaceRadar("DelBundleExtStats", E_DEL_EXT_BUNDLE_STATS_ERROR, extraData);
    }
    return ret;
}

void BmsSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED) {
        int32_t userId = want.GetIntParam(USER_ID, WANT_DEFAULT_VALUE);
        std::string businessName = want.GetStringParam(BUNDLE_NAME);
        LOGI("receive app remove action, userId: %{public}d, businessName: %{public}s", userId, businessName.c_str());
        AppStatusManager::GetInstance().DelBundleExtStats(userId, businessName);
    }
}
}  // namespace StorageManager
}  // namespace OHOS