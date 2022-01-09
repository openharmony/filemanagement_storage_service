/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "storage/storage_status_service.h"
#include "utils/storage_manager_errno.h"
#include "utils/storage_manager_log.h"
#include "installd_client.h"
#include "os_account_manager.h"

using namespace std;

namespace OHOS {
namespace StorageManager {

StorageStatusService::StorageStatusService() {}
StorageStatusService::~StorageStatusService() {}

int StorageStatusService::GetCurrentUserId() {
    AccountSA::OsAccountInfo osAccountInfo;
    if (AccountSA::OsAccountManager::QueryCurrentOsAccount(osAccountInfo) != E_OK) {
        LOGE("StorageStatusService::An error occurred in querying current os account.");
        return DEFAULT_USER_ID;
    } else {
        return osAccountInfo.GetLocalId();
    }
}

vector<int64_t> StorageStatusService::GetBundleStats(std::string uuid, std::string pkgName) {
    vector<int64_t> result = {0,0,0};
    int userId = GetCurrentUserId();
    LOGI("StorageStatusService::userId is:%d", userId);
    vector<int64_t> bundleStats;
    AppExecFwk::InstalldClient::GetInstance()->GetBundleStats(pkgName, userId, bundleStats); // 错误码的定义要再对齐一下
    if (bundleStats.size() != dataDir.size() ) {
        LOGE("StorageStatusService::An error occurred in querying bundle stats.");
        return result;
    }
    for (int i = 0; i < bundleStats.size(); i++) {
        if (bundleStats[i] == E_ERR) {
            LOGE("StorageStatusService::Failed to query %s data.", dataDir[i].c_str());
            bundleStats[i] = 0;
        }
    }
    result[0] = bundleStats[0]; //魔鬼数字？
    result[1] = bundleStats[1] + bundleStats[2] + bundleStats[3];
    result[2] = bundleStats[4];
    LOGI("StorageStatusService result: result[0]:%lld, result[1]:%lld, result[2]:%lld", result[0], result[1], result[2]);
    return result;
}
} // StorageManager
} // OHOS