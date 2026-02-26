/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

int32_t StorageDaemonCommunication::GetDataSizeByPath(const std::string &path, int64_t &size)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetDataSizeByPath(path, size);
}

int32_t StorageDaemonCommunication::GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetRmgResourceSize(rgmName, totalSize);
}

int32_t StorageDaemonCommunication::QueryOccupiedSpaceForSa(std::string &storageStatus,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->QueryOccupiedSpaceForSa(storageStatus,
        bundleNameAndUid);
}

int32_t StorageDaemonCommunication::ClearSecondMountPoint(uint32_t userId, const std::string &bundleName)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->ClearSecondMountPoint(userId, bundleName);
}

int32_t StorageDaemonCommunication::GetSystemDataSize(int64_t &otherUidSizeSum)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetSystemDataSize(otherUidSizeSum);
}

int32_t StorageDaemonCommunication::GetDirListSpaceByPaths(const std::vector<std::string> &paths,
    const std::vector<int32_t> &uids, std::vector<DirSpaceInfo> &resultDirs)
{
    return StorageDaemonCommunicationMock::storageDaemonCommunication->GetDirListSpaceByPaths(paths, uids, resultDirs);
}
}
}