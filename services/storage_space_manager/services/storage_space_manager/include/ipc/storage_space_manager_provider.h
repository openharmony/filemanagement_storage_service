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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_PROVIDER_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_PROVIDER_H

#include "storage_space_manager_stub.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
}
}

namespace OHOS {
namespace StorageSpaceManager {

#ifndef STORAGE_SPACE_MANAGER_SA_ID
#define STORAGE_SPACE_MANAGER_SA_ID 8650
#endif

class StorageSpaceManagerProvider : public SystemAbility, public StorageSpaceManagerStub {
    DECLARE_SYSTEM_ABILITY(StorageSpaceManagerProvider)

public:
    explicit StorageSpaceManagerProvider(int32_t saId = STORAGE_SPACE_MANAGER_SA_ID, bool runOnCreate = false);
    ~StorageSpaceManagerProvider() override = default;

    /* ---------- SystemAbility 生命周期 ---------- */
    void OnStart(const SystemAbilityOnDemandReason& startReason) override;
    void OnStop() override;
    // todo cancleidle需要在入口处调用
    void OnActive(const SystemAbilityOnDemandReason& activeReason) override;
    int32_t OnIdle(const SystemAbilityOnDemandReason& idleReason) override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    /* ---------- IPC 接口实现（IDL 生成的 stub 调用） ---------- */
    int32_t GetTotalSize(int64_t &totalSize) override;
    int32_t GetSystemSize(int64_t &systemSize) override;
    int32_t GetFreeSize(int64_t &freeSize) override;
    int32_t GetTotalInodes(int64_t &totalInodes) override;
    int32_t GetFreeInodes(int64_t &freeInodes) override;
    int32_t CleanBundleCache(int32_t userId) override;

    /* ---------- IPC 计数（Stub 层调用，用于 idle 判断） ---------- */
    void AddRunningIpcCount();
    void SubtractRunningIpcCount();

private:
    /* ---------- 初始化 ---------- */
    bool Init();

    /* ---------- 延迟卸载 ---------- */
    int32_t CreateUnloadHandler();
    int32_t DestroyUnloadHandler();
    void DelayUnloadTask();

    /* ---------- idle 判断 ---------- */
    bool IsReadyIntoIdle();
    bool ExitIdleState();

    /* ---------- 状态 ---------- */
    std::atomic<bool> serviceReady_{false};
    std::atomic<bool> isStopped_{false};
    std::atomic<int32_t> runningIpcCount_{0};

    /* ---------- 依赖 SA 监听 ---------- */
    std::mutex depSaIdsMtx_;
    std::unordered_set<int32_t> depSaIds_;

    /* ---------- 延迟卸载 ---------- */
    std::mutex unloadMutex_;
    std::shared_ptr<AppExecFwk::EventHandler> unloadHandler_;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_PROVIDER_H
