/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H
#define OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H

#include <iostream>
#include <nocopyable.h>
#include <singleton.h>
#include <thread>
#include <vector>
#include "event_handler.h"

namespace OHOS {
namespace StorageManager {
class StorageMonitorService : public NoCopyable  {
    DECLARE_DELAYED_SINGLETON(StorageMonitorService);

public:
    void StartStorageMonitorTask();

private:
    void StartEventHandler();
    void Execute();
    void CheckAndCleanBundleCache();
    int64_t GetLowerThreshold(int64_t totalSize);

    std::mutex eventMutex_;
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MONITOR_SERVICE_H