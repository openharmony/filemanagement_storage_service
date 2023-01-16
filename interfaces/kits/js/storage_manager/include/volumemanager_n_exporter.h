/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VOLUMEMANAGER_N_EXPORTER_H
#define VOLUMEMANAGER_N_EXPORTER_H

#include "n_exporter.h"
#include "storage_manager_connect.h"

namespace OHOS {
namespace StorageManager {
namespace ModuleVolumeManager {
napi_value GetAllVolumes(napi_env env, napi_callback_info info);
napi_value Mount(napi_env env, napi_callback_info info);
napi_value Unmount(napi_env env, napi_callback_info info);
napi_value GetVolumeByUuid(napi_env env, napi_callback_info info);
napi_value GetVolumeById(napi_env env, napi_callback_info info);
napi_value SetVolumeDescription(napi_env env, napi_callback_info info);
napi_value Format(napi_env env, napi_callback_info info);
napi_value Partition(napi_env env, napi_callback_info info);
} // namespace ModuleVolumeManager
} // namespace StorageManager
} // namespace OHOS
#endif // VOLUMEMANAGER_N_EXPORTER_H