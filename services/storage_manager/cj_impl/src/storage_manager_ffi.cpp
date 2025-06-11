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

#include "storage_manager_ffi.h"

#include "bundle_stats.h"
#include "cj_common_ffi.h"
#include "native/ffi_remote_data.h"
#include "storage_manager_impl.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace CJStorageManager {

extern "C" {
FFI_EXPORT void FfiFileStorStatsGetCurrentBundleStats(NativeBundleStats *stats, int32_t *errCode)
{
    if (stats == nullptr) {
        *errCode = E_NOOBJECT;
        LOGE("FfiFileStorStatsGetCurrentBundleStats check failed");
        return;
    }
    StorageManager::BundleStats bundleStats = {};
    uint32_t statFlag = 0;
    auto service = DelayedSingleton<CjStorageStatusService>::GetInstance();
    if (service == nullptr) {
        *errCode = E_IPCSS;
        return;
    }
    int32_t errNum = service->GetCurrentBundleStats(bundleStats, statFlag);
    if (errNum != E_OK) {
        *errCode = Convert2CjErrNum(errNum);
        return;
    }
    *errCode = E_OK;
    stats->appSize = bundleStats.appSize_;
    stats->cacheSize = bundleStats.cacheSize_;
    stats->dataSize = bundleStats.dataSize_;
    return;
}

FFI_EXPORT int64_t FfiFileStorStatsGetTotalSize(int32_t *errCode)
{
    int64_t totalSize = 0;
    auto service = DelayedSingleton<CjStorageStatusService>::GetInstance();
    if (service == nullptr) {
        *errCode = E_IPCSS;
        return 0;
    }
    int32_t errNum = service->GetTotalSize(totalSize);
    if (errNum != E_OK) {
        *errCode = Convert2CjErrNum(errNum);
        return 0;
    }
    *errCode = E_OK;
    return totalSize;
}

FFI_EXPORT int64_t FfiFileStorStatsGetFreeSize(int32_t *errCode)
{
    int64_t freeSize = 0;
    auto service = DelayedSingleton<CjStorageStatusService>::GetInstance();
    if (service == nullptr) {
        *errCode = E_IPCSS;
        return 0;
    }
    int32_t errNum = service->GetFreeSize(freeSize);
    if (errNum != E_OK) {
        *errCode = Convert2CjErrNum(errNum);
        return 0;
    }
    *errCode = E_OK;
    return freeSize;
}
}

} // namespace CJStorageManager
} // namespace OHOS