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

#include "storage_file_raw_data.h"
#include "ipc_types.h"
#include "securec.h"

namespace OHOS {
namespace StorageManager {
int32_t StorageFileRawData::RawDataCpy(const void* readdata)
{
    if (readdata == nullptr || size == 0 || size > StorageFileRawData::MAX_RAW_DATA_SIZE) {
        return ERR_INVALID_DATA;
    }
    void* newData = malloc(size);
    isMalloc = true;
    if (newData == nullptr) {
        isMalloc = false;
        return ERR_INVALID_DATA;
    }
    if (memcpy_s(newData, size, readdata, size) != EOK) {
        free(newData);
        isMalloc = false;
        return ERR_INVALID_DATA;
    }
    if (data != nullptr) {
        free(const_cast<void*>(data));
        data = nullptr;
    }
    data = newData;
    return ERR_NONE;
}

StorageFileRawData::~StorageFileRawData()
{
    if (data != nullptr && isMalloc) {
        free(const_cast<void*>(data));
        isMalloc = false;
        data = nullptr;
    }
}
} // namespace StorageManager
} // namespace OHOS