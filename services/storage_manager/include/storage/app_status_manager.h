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
#ifndef STORAGE_APP_STATUS_MANAGER_H
#define STORAGE_APP_STATUS_MANAGER_H

#include <string>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "rdb_adapter/storage_rdb_adapter.h"

namespace OHOS {
namespace StorageManager {
class BmsSubscriber : public EventFwk::CommonEventSubscriber {
public:
    BmsSubscriber() = default;
    explicit BmsSubscriber(const EventFwk::CommonEventSubscribeInfo &info);
    virtual ~BmsSubscriber() = default;
    virtual void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
};

class AppStatusManager {
public:
    static AppStatusManager &GetInstance();
    virtual ~AppStatusManager() = default;
    bool SubscribeCommonEvent(void);
    int32_t DelBundleExtStats(int32_t userId, std::string &businessName);
private:
    AppStatusManager() = default;
    std::shared_ptr<BmsSubscriber> bmsSubscriber = nullptr;
};
}  // namespace StorageManager
}  // namespace OHOS
#endif // STORAGE_APP_STATUS_MANAGER_H