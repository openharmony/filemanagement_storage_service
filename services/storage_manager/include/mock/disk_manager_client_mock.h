/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef DISK_MANAGER_CLIENT_MOCK_H
#define DISK_MANAGER_CLIENT_MOCK_H

#include <gmock/gmock.h>

#include "disk_manager_client.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace DiskManager {

class IDiskManagerClientMock {
public:
    virtual ~IDiskManagerClientMock() = default;
    virtual int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize);
    virtual int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize);

    static inline std::shared_ptr<IDiskManagerClientMock> diskManagerClientMock = nullptr;
};

class DiskManagerClientMock : public IDiskManagerClientMock {
public:
    MOCK_METHOD(int32_t, GetFreeSizeOfVolume, (const std::string &volumeUuid, int64_t &freeSize), (override));
    MOCK_METHOD(int32_t, GetTotalSizeOfVolume, (const std::string &volumeUuid, int64_t &totalSize), (override));
};

} // DiskManager
} // OHOS

#endif // DISK_MANAGER_CLIENT_MOCK_H
