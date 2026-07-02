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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_LOAD_CALLBACK_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_LOAD_CALLBACK_H

#include <cstdint>

#include <refbase.h>
#include <system_ability_load_callback_stub.h>

namespace OHOS {
class IRemoteObject;
}

namespace OHOS {
namespace StorageSpaceManager {

// storage_space_manager SA 异步加载回调。
// 由 SystemAbilityManager 在 LoadSystemAbility 完成后回调，
// 把远端对象 / 加载失败结果回传给 StorageSpaceManagerClient。
class StorageSpaceManagerLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_LOAD_CALLBACK_H
