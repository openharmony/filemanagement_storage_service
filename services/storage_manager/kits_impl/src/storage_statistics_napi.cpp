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

#include "storage_statistics_napi.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "storage_statistics_n_exporter.h"

namespace OHOS {
namespace StorageManager {
/***********************************************
 * Module export and register
 ***********************************************/
napi_value StorageStatisticsExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getTotalSizeOfVolume", GetTotalSizeOfVolume),
        DECLARE_NAPI_FUNCTION("getFreeSizeOfVolume", GetFreeSizeOfVolume),
        DECLARE_NAPI_FUNCTION("getBundleStats", GetBundleStats),
        DECLARE_NAPI_FUNCTION("getCurrentBundleStats", GetCurrentBundleStats),
        DECLARE_NAPI_FUNCTION("getSystemSize", GetSystemSize),
        DECLARE_NAPI_FUNCTION("getUserStorageStats", GetUserStorageStats),
        DECLARE_NAPI_FUNCTION("getTotalSize", GetTotalSize),
        DECLARE_NAPI_FUNCTION("getFreeSize", GetFreeSize),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

NAPI_MODULE(storagestatistics, StorageStatisticsExport)
} // namespace StorageManager
} // namespace OHOS
