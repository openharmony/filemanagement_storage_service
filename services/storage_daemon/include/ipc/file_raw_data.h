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

#ifndef OHOS_STORAGE_DAEMON_FILE_RAW_DATA_H
#define OHOS_STORAGE_DAEMON_FILE_RAW_DATA_H

#include "securec.h"
#include "storage_service_errno.h"
#include <cstring>
namespace OHOS {
namespace StorageDaemon {

/**
 * @brief Maximum size for IPC capacity.
 */
constexpr uint32_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024; // 128MB

/**
 * @class FileRawData
 * @brief Represents raw data used in IPC communication.
 */
class FileRawData {
public:
    FileRawData() : data(nullptr), size(0) {};
    ~FileRawData();

    /**
     * @brief Copies raw data into the internal data. Fixed method used by the IDL tool
     *
     * @param rawData Pointer to the raw data to be copied.
     * @return int32_t Returns 0 on success, or an error code on failure.
     */
    int32_t RawDataCpy(const void *rawData);

public:
    const void *data; // Fixed variable used by the IDL tool
    uint32_t size; // Fixed variable used by the IDL tool
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // OHOS_STORAGE_DAEMON_FILE_RAW_DATA_H