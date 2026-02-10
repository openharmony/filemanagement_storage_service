/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef STORAGESTATUS_N_EXPORTER_H
#define STORAGESTATUS_N_EXPORTER_H

#include <tuple>
#include "n_exporter.h"
#include "storage_statistics_napi.h"

namespace OHOS {
namespace StorageManager {
napi_value GetTotalSizeOfVolume(napi_env env, napi_callback_info info);
napi_value GetFreeSizeOfVolume(napi_env env, napi_callback_info info);
napi_value GetBundleStats(napi_env env, napi_callback_info info);
napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info);
napi_value GetSystemSize(napi_env env, napi_callback_info info);
napi_value GetUserStorageStats(napi_env env, napi_callback_info info);
napi_value GetTotalSize(napi_env env, napi_callback_info info);
napi_value GetFreeSize(napi_env env, napi_callback_info info);
napi_value GetTotalSizeSync(napi_env env, napi_callback_info info);
napi_value GetFreeSizeSync(napi_env env, napi_callback_info info);
napi_value GetSystemDataSize(napi_env env, napi_callback_info info);
napi_value SetExtBundleStats(napi_env env, napi_callback_info info);
napi_value GetExtBundleStats(napi_env env, napi_callback_info info);
napi_value GetAllExtBundleStats(napi_env env, napi_callback_info info);
napi_value ListUserdataDirInfo(napi_env env, napi_callback_info info);

std::tuple<std::string, int32_t, uint32_t> ExtractNameAndIndex(napi_env env, napi_callback_info info);
} // namespace StorageManager
} // namespace OHOS
#endif // STORAGESTATUS_N_EXPORTER_H