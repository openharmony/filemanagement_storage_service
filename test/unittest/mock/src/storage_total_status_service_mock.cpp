/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "storage/storage_total_status_service.h"
#include "storage_total_status_service_mock.h"

namespace OHOS::StorageManager {
int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    return StorageTotalStatusServiceBase::stss->GetSystemSize(systemSize);
}

int32_t StorageTotalStatusService::GetTotalSize(int64_t &totalSize)
{
    return StorageTotalStatusServiceBase::stss->GetTotalSize(totalSize);
}

int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    return StorageTotalStatusServiceBase::stss->GetFreeSize(freeSize);
}

int32_t StorageTotalStatusService::GetTotalInodes(int64_t &totalInode)
{
    return StorageTotalStatusServiceBase::stss->GetTotalInodes(totalInode);
}

int32_t StorageTotalStatusService::GetFreeInodes(int64_t &freeInode)
{
    return StorageTotalStatusServiceBase::stss->GetFreeInodes(freeInode);
}

int32_t StorageTotalStatusService::GetUsedInodes(int64_t &usedInodes)
{
    return StorageTotalStatusServiceBase::stss->GetUsedInodes(usedInodes);
}
} // OHOS::StorageManager