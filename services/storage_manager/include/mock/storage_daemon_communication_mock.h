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

#ifndef STORAGE_DAEMON_COMMUNICATION_MOCK_H
#define STORAGE_DAEMON_COMMUNICATION_MOCK_H

#include <gmock/gmock.h>

#include "storage_daemon_communication/storage_daemon_communication.h"

namespace OHOS {
namespace StorageManager {

class IStorageDaemonCommunicationMock {
public:
    virtual ~IStorageDaemonCommunicationMock() = default;
    virtual int32_t GetDataSizeByPath(const std::string &path, int64_t &size);
    virtual int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize);
    virtual int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    virtual int32_t QueryOccupiedSpaceForSa(std::string &storageStatus,
        const std::map<int32_t, std::string> &bundleNameAndUid);
    virtual int32_t GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks);
    virtual int32_t GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs, std::vector<DirSpaceInfo> &outDirs);
    virtual int32_t SetStopScanFlag(bool stop = false);
    virtual int32_t GetAncoSizeData(std::string &outExtraData);
    virtual int32_t UMountCryptoPathAgain(uint32_t userId, const std::string &bundleName);
    static inline std::shared_ptr<IStorageDaemonCommunicationMock> storageDaemonCommunication = nullptr;
};
class StorageDaemonCommunicationMock : public IStorageDaemonCommunicationMock {
public:
    MOCK_METHOD(int32_t, GetDataSizeByPath, (const std::string &, int64_t &), (override));
    MOCK_METHOD(int32_t, GetRmgResourceSize, (const std::string &, uint64_t &), (override));
    MOCK_METHOD(int32_t, GetFileEncryptStatus, (uint32_t, bool &, bool), (override));
    MOCK_METHOD(int32_t, QueryOccupiedSpaceForSa, (std::string &, (const std::map<int32_t, std::string> &)),
        (override));
    MOCK_METHOD(int32_t, GetDqBlkSpacesByUids, (const std::vector<int32_t> &, std::vector<NextDqBlk> &));
    MOCK_METHOD(int32_t, GetDirListSpace, (const std::vector<DirSpaceInfo> &, std::vector<DirSpaceInfo> &));
    MOCK_METHOD(int32_t, SetStopScanFlag, (bool));
    MOCK_METHOD(int32_t, GetAncoSizeData, (std::string &));
    MOCK_METHOD(int32_t, UMountCryptoPathAgain, (uint32_t, const std::string &));
};
} // StorageManager
} // OHOS

#endif // STORAGE_DAEMON_COMMUNICATION_MOCK_H