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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include <iremote_object.h>
#include <nocopyable.h>
#include <refbase.h>
#include <singleton.h>
#include <system_ability_status_change_stub.h>

#include "storage_space_manager_errno.h"

namespace OHOS {
namespace StorageSpaceManager {

class IStorageSpaceManager;

/** storage_space_manager SA 客户端；提供存储空间查询和缓存清理能力。
 *  通过 LoadSystemAbility 按需拉起 SA 进程，并在 SA 崩溃/重启后自动重连。 */
class StorageSpaceManagerClient : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageSpaceManagerClient);

public:
    /* ---------- Storage Space Query APIs ---------- */
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetFreeSize(int64_t &freeSize);
    int32_t GetTotalInodes(int64_t &totalInodes);
    int32_t GetFreeInodes(int64_t &freeInodes);

    /* ---------- Bundle Cache Management APIs ---------- */
    int32_t CleanBundleCache(int32_t userId);

    /* ---------- SA 加载回调（由 StorageSpaceManagerLoadCallback 调用） ---------- */
    void LoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    void LoadSystemAbilityFail();

    /* ---------- Inner-only (进程内，不导出) ---------- */
    int32_t ResetProxy();

private:
    // 取已缓存的 proxy；若未缓存则尝试快速路径（CheckSystemAbility），未命中返回 nullptr。
    sptr<IStorageSpaceManager> GetProxy();
    // 异步 LoadSystemAbility 拉起 SA 进程，条件变量等待 proxy 就绪。
    int32_t LoadStorageSpaceManagerService();
    // 连接 SA：快速复用 -> 未运行则按需拉起。供各业务方法调用。
    int32_t Connect(sptr<IStorageSpaceManager> &proxy);
    // 订阅 SA 上下线状态，SA 重启上线后清空 proxy 缓存，下次调用自动重连。
    void SubscribeSsmSA();
    void OnAddSystemAbility();

    // SA 崩溃死亡监听
    class SsmDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        SsmDeathRecipient() = default;
        ~SsmDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };

    // SA 上下线监听（重启自动重连）
    class SystemAbilityStatusListener : public SystemAbilityStatusChangeStub {
    public:
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

private:
    sptr<IStorageSpaceManager> storageSpaceManager_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    sptr<SystemAbilityStatusListener> statusListener_;
    std::mutex mutex_;
    std::condition_variable proxyConVar_;
    bool loadFinished_ = false;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_CLIENT_H
