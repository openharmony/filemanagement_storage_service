/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef VOLUME_MANAGER_SERVICE_MOCK_H
#define VOLUME_MANAGER_SERVICE_MOCK_H

#include "gmock/gmock.h"
#include "volume_external.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "volume/volume_manager_service.h"

namespace OHOS {
namespace StorageDaemon {
class VolumeExternalMock : public StorageManager::VolumeExternal {
public:
    static inline std::shared_ptr<VolumeExternalMock> volumeExternMock_ = nullptr;
public:
    VolumeExternalMock() {}
    virtual ~VolumeExternalMock() {}
    MOCK_METHOD0(GetState, int32_t());
};

class MockVolumeManagerService : public VolumeManagerService {
public:
    MOCK_METHOD(int32_t, GetVolumeById, (const std::string &, VolumeExternal &));
};

class MockStorageDaemonCommunication : public StorageDaemonCommunication {
public:
    MOCK_METHOD(int32_t, Encrypt, (const std::string &, const std::string &));
    MOCK_METHOD(int32_t, GetCryptProgressById, (const std::string &, int32_t &));
};
}
}
#endif // VOLUME_MANAGER_SERVICE_MOCK_H