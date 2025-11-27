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
#include "rdb_adapter/storage_rdb_adapter.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
using namespace OHOS::StorageService;
AppStatusManager &AppStatusManager::GetInstance()
{
    static AppStatusManager instance;
    return instance;
}

int32_t AppStatusManager::DelBundleExtStats(int32_t userId, std::string &businessName)
{
    StorageRdbAdapter &rdbAdapter = StorageRdbAdapter::GetInstance();
    std::vector<NativeRdb::ValueObject> values;
    values.emplace_back(NativeRdb::ValueObject(businessName));
    values.emplace_back(NativeRdb::ValueObject(userId));
    int32_t deleteRows = ROWCOUNT_INIT;
    int32_t ret = rdbAdapter.Delete(deleteRows, BUNDLE_EXT_STATS_TABLE, WHERE_CLAUSE, values);
    if (ret != E_OK) {
        LOGE("DelBundleExtStats failed.");
        std::string extraData = "errCode=" + std::to_string(ret);
        StorageRadar::ReportSpaceRadar("DelBundleExtStats", E_DEL_EXT_BUNDLE_STATS_ERROR, extraData);
    }
    return E_OK;
}
}  // namespace StorageManager
}  // namespace OHOS