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

#include "cjstoragestatusservice_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "fuzzer/FuzzedDataProvider.h"
#include "storage_manager_ffi.h"

extern "C" {
int64_t FfiFileStorStatsGetTotalSize(int32_t *errCode);
int64_t FfiFileStorStatsGetFreeSize(int32_t *errCode);
void FfiFileStorStatsGetCurrentBundleStats(OHOS::CJStorageManager::NativeBundleStats *stats, int32_t *errCode);
}

namespace OHOS {
namespace {
void CjStorageStatusServiceFuzzTest(FuzzedDataProvider &provider)
{
    int32_t errCode = 0;
    CJStorageManager::NativeBundleStats stats {};

    if (provider.ConsumeBool()) {
        (void)FfiFileStorStatsGetTotalSize(&errCode);
    }
    if (provider.ConsumeBool()) {
        (void)FfiFileStorStatsGetFreeSize(&errCode);
    }
    if (provider.ConsumeBool()) {
        FfiFileStorStatsGetCurrentBundleStats(&stats, &errCode);
    }
}
} // namespace
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return 0;
    }

    FuzzedDataProvider provider(data, size);
    OHOS::CjStorageStatusServiceFuzzTest(provider);
    return 0;
}
