/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H
#define OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H

#include <nocopyable.h>
#include <sys/types.h>
#include <string>

namespace OHOS {
namespace StorageDaemon {
class QuotaManager final {
public:
    virtual ~QuotaManager() = default;
    static QuotaManager* GetInstance();

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb);
    int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size);
private:
    QuotaManager() = default;
    DISALLOW_COPY_AND_MOVE(QuotaManager);

    static QuotaManager* instance_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H