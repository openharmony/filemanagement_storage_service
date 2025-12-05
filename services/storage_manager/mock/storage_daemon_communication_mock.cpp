/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

int32_t StorageDaemonCommunication::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    return IStorageDaemonCommunicationMock::storageDaemonCommunication->GetFileEncryptStatus(userId, isEncrypted,
        needCheckDirMount);
}

int32_t StorageDaemonCommunication::GetDqBlkSpacesByUids(const std::vector<int32_t> &uids,
    std::vector<NextDqBlk> &dqBlks)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetDqBlkSpacesByUids(uids, dqBlks);
}

int32_t StorageDaemonCommunication::GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs,
    std::vector<DirSpaceInfo> &outDirs)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetDirListSpace(inDirs, outDirs);
}

int32_t StorageDaemonCommunication::SetStopScanFlag(bool stop)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->SetStopScanFlag(stop);
}

int32_t StorageDaemonCommunication::GetAncoSizeData(std::string &outExtraData)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetAncoSizeData(outExtraData);
}
}
}
