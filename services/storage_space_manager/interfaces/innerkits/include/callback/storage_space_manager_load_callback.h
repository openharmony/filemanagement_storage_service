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

// Async load callback for storage_space_manager SA.
// Called by SystemAbilityManager after LoadSystemAbility completes,
// passes the remote object / load failure result back to StorageSpaceManagerClient.
class StorageSpaceManagerLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
        const sptr<IRemoteObject> &remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_LOAD_CALLBACK_H
