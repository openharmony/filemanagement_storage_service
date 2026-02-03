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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_MOCK_H
#define OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_MOCK_H

#include <gmock/gmock.h>

#include "storage/storage_total_status_service.h"

namespace OHOS::StorageManager {
class StorageTotalStatusServiceBase {
public:
    virtual ~StorageTotalStatusServiceBase() = default;
public:
    virtual int32_t GetSystemSize(int64_t&) = 0;
    virtual int32_t GetTotalSize(int64_t&) = 0;
    virtual int32_t GetFreeSize(int64_t&) = 0;
    virtual int32_t GetTotalInodes(int64_t&) = 0;
    virtual int32_t GetFreeInodes(int64_t&) = 0;
public:
    static inline std::shared_ptr<StorageTotalStatusServiceBase> stss = nullptr;
};

class StorageTotalStatusServiceMock : public StorageTotalStatusServiceBase {
public:
    MOCK_METHOD(int32_t, GetSystemSize, (int64_t&));
    MOCK_METHOD(int32_t, GetTotalSize, (int64_t&));
    MOCK_METHOD(int32_t, GetFreeSize, (int64_t&));
    MOCK_METHOD(int32_t, GetTotalInodes, (int64_t&));
    MOCK_METHOD(int32_t, GetFreeInodes, (int64_t&));
};
} // OHOS::StorageManager
#endif // OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_MOCK_H

