/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storagetotalstatusservice_fuzzer.h"
#include "storage/storage_total_status_service.h"
#include "storage_service_log.h"
namespace OHOS {
namespace StorageManager {
bool StorageTotalStatusServiceFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return false;
    }
    std::shared_ptr<StorageTotalStatusService> service = DelayedSingleton<StorageTotalStatusService>::GetInstance();
    int64_t result = service->GetTotalSize();
    service->GetSystemSize();
    service->GetFreeSize();
    // You can add other interfaces of class StorageTotalStatusService here.
    if (result >= 0) {
        LOGI("Storage total status service fuzz test of interface StorageTotalStatusService::GetTotalSize success!");
        return true;
    }
    LOGE("Storage total status service fuzz test of interface StorageTotalStatusService::GetTotalSize failed!");
    return false;
}
} // namespace StorageManager
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::StorageTotalStatusServiceFuzzTest(data, size);
    return 0;
}
