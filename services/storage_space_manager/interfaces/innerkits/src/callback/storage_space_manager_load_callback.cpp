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

#include "callback/storage_space_manager_load_callback.h"

#include <iremote_object.h>

#include "storage_space_manager_hilog.h"
#include "storage_space_manager_client.h"

namespace OHOS {
namespace StorageSpaceManager {

void StorageSpaceManagerLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject)
{
    LOGI("OnLoadSystemAbilitySuccess, systemAbilityId=%{public}d", systemAbilityId);
    DelayedSingleton<StorageSpaceManagerClient>::GetInstance()->LoadSystemAbilitySuccess(remoteObject);
}

void StorageSpaceManagerLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGE("OnLoadSystemAbilityFail, systemAbilityId=%{public}d", systemAbilityId);
    DelayedSingleton<StorageSpaceManagerClient>::GetInstance()->LoadSystemAbilityFail();
}

} // namespace StorageSpaceManager
} // namespace OHOS
