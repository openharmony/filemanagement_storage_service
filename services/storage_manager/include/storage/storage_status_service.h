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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H
#define OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H

#include <vector>
#include <nocopyable.h>
#include <singleton.h>
#include <iostream>

namespace OHOS {
namespace StorageManager {
class StorageStatusService : public NoCopyable  {
    DECLARE_DELAYED_SINGLETON(StorageStatusService);

public:
    std::vector<int64_t> GetBundleStats(std::string pkgName);
private:
    int GetCurrentUserId();
    const std::vector<std::string> dataDir = {"app", "local", "distributed", "database", "cache"};
    const int DEFAULT_USER_ID = 100;
    enum BUNDLE_STATS {APP = 0, LOCAL, DISTRIBUTED, DATABASE, CACHE};
    enum BUNDLE_STATS_RESULT {APPSIZE = 0, CACHESIZE, DATASIZE};
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_STATUS_SERVICE_H