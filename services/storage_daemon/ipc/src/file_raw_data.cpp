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
#include "ipc/file_raw_data.h"
#include "storage_service_log.h"
#include <cstdlib>
#include <cstring>

namespace OHOS {
namespace StorageDaemon {

FileRawData::FileRawData(uint32_t size, const void *data) : size(size), data(data) {}

int32_t FileRawData::RawDataCpy(const void *rawData)
{
    if (rawData == nullptr) {
        LOGE("null rawData");
        return E_ERR;
    }
    if (size == 0 || size > MAX_IPC_RAW_DATA_SIZE) {
        LOGE("size invalid: %u", size);
        return E_ERR;
    }
    void *buffer = nullptr;
    size_t dataSize = static_cast<size_t>(size);
    buffer = malloc(dataSize);
    if (buffer == nullptr) {
        LOGE("malloc buffer failed");
        return E_ERR;
    }
    if (memcpy_s(buffer, dataSize, rawData, dataSize) != E_OK) {
        free(buffer);
        LOGE("memcpy failed");
        return E_ERR;
    }
    data = buffer;
    return E_OK;
}

FileRawData::~FileRawData()
{
    if (data != nullptr) {
        free(const_cast<void*>(data));
        data = nullptr;
    }
}

} // namespace StorageDaemon
} // namespace OHOS