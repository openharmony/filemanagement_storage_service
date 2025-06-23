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

#ifndef STORAGE_MANAGER_IMPL_H
#define STORAGE_MANAGER_IMPL_H

#include <nocopyable.h>
#include <singleton.h>

#include "bundle_stats.h"
#include "istorage_manager.h"
#include "storage_manager_ffi.h"

namespace OHOS {
namespace CJStorageManager {

class CjStorageStatusService : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(CjStorageStatusService);

public:
    int32_t Connect();
    int32_t GetCurrentBundleStats(StorageManager::BundleStats &bundleStats, uint32_t statFlag);
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetFreeSize(int64_t &freeSize);

    int32_t ResetProxy();

private:
    sptr<StorageManager::IStorageManager> storageManager_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex mutex_;
};

class CjSmDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    CjSmDeathRecipient() = default;
    virtual ~CjSmDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};

int32_t Convert2CjErrNum(int32_t errNum);
} // namespace CJStorageManager
} // namespace OHOS

#endif