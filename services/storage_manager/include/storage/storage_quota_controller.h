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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_QUOTA_CONTROLLER_H
#define OHOS_STORAGE_MANAGER_STORAGE_QUOTA_CONTROLLER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <sys/types.h>

#include "cJSON.h"

namespace OHOS {
namespace StorageManager {
struct BaselineCfg {
    int32_t uid;
    int32_t baseline;
};

class StorageQuotaController final {
public:
    static StorageQuotaController &GetInstance();
    virtual ~StorageQuotaController() = default;

    void UpdateBaseLineByUid();

private:
    StorageQuotaController() = default;
    int32_t ReadCcmConfigFile(std::vector<BaselineCfg> &params);
    int32_t SaveUidAndBaseLine(cJSON* configArray, std::vector<BaselineCfg> &params);
    std::mutex quotaControllerMtx_;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_QUOTA_CONTROLLER_H
