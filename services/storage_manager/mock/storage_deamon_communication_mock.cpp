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

#include "mock/storage_daemon_communication_mock.h"

namespace OHOS {
namespace StorageManager {

int32_t StorageDaemonCommunication::GetDataSizeByPath(const std::string &path, int64_t &size)
{
    return IStorageDaemonCommunicationMock::storageDaemonCommunication->GetDataSizeByPath(path, size);
}

int32_t StorageDaemonCommunication::GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    return IStorageDaemonCommunicationMock::storageDaemonCommunication->GetRmgResourceSize(rgmName, totalSize);
}

int32_t StorageDaemonCommunication::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    return IStorageDaemonCommunicationMock::storageDaemonCommunication->GetFileEncryptStatus(userId, isEncrypted,
        needCheckDirMount);
}

int32_t StorageDaemonCommunication::QueryOccupiedSpaceForSa(std::string &storageStatus,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->QueryOccupiedSpaceForSa(storageStatus,
        bundleNameAndUid);
}
} // namespace StorageManager
}