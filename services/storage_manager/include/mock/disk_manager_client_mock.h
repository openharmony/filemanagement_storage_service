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
    virtual int32_t Mount(const std::string &volumeId);
    virtual int32_t Unmount(const std::string &volumeId);
    virtual int32_t Format(const std::string &volumeId, const std::string &fsType);
    virtual int32_t SetVolumeDescription(const std::string &fsUuid, const std::string &description);
    virtual int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol);
    virtual int32_t GetVolumeByUuid(const std::string &uuid, VolumeExternal &vc);
    virtual int32_t GetVolumeById(const std::string &volumeId, VolumeExternal &vc);
    virtual int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize);
    virtual int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize);
    virtual int32_t GetAllDisks(std::vector<Disk> &vecOfDisk);
    virtual int32_t GetDiskById(const std::string &diskId, Disk &disk);
    virtual int32_t Partition(const std::string &diskId, int32_t type);
    virtual int32_t GetPartitionTable(const std::string &diskId, PartitionTableInfo &out);
    virtual int32_t CreatePartition(const std::string &diskId, const PartitionParams &params);
    virtual int32_t DeletePartition(const std::string &diskId, int32_t partitionNum);
    virtual int32_t FormatPartition(const std::string &diskId, int32_t partitionNum, const FormatParams &params);
    virtual int32_t ResetProxy();
    virtual int32_t NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                                     const std::string &uuid, const std::string &fsType);
    virtual int32_t NotifyMtpUnmounted(const std::string &id, const bool isBadRemove);
    virtual int32_t OnBlockDiskUevent(const std::string &rawUeventMsg);

    static inline std::shared_ptr<IDiskManagerClientMock> diskManagerClientMock = nullptr;
};

class DiskManagerClientMock : public IDiskManagerClientMock {
public:
    MOCK_METHOD(int32_t, Mount, (const std::string &volumeId), (override));
    MOCK_METHOD(int32_t, Unmount, (const std::string &volumeId), (override));
    MOCK_METHOD(int32_t, Format, (const std::string &volumeId, const std::string &fsType), (override));
    MOCK_METHOD(int32_t, SetVolumeDescription, (const std::string &fsUuid, const std::string &description),
                (override));
    MOCK_METHOD(int32_t, GetAllVolumes, (std::vector<VolumeExternal> &vecOfVol), (override));
    MOCK_METHOD(int32_t, GetVolumeByUuid, (const std::string &uuid, VolumeExternal &vc), (override));
    MOCK_METHOD(int32_t, GetVolumeById, (const std::string &volumeId, VolumeExternal &vc), (override));
    MOCK_METHOD(int32_t, GetFreeSizeOfVolume, (const std::string &volumeUuid, int64_t &freeSize), (override));
    MOCK_METHOD(int32_t, GetTotalSizeOfVolume, (const std::string &volumeUuid, int64_t &totalSize), (override));
    MOCK_METHOD(int32_t, GetAllDisks, (std::vector<Disk> &vecOfDisk), (override));
    MOCK_METHOD(int32_t, GetDiskById, (const std::string &diskId, Disk &disk), (override));
    MOCK_METHOD(int32_t, Partition, (const std::string &diskId, int32_t type), (override));
    MOCK_METHOD(int32_t, GetPartitionTable, (const std::string &diskId, PartitionTableInfo &out), (override));
    MOCK_METHOD(int32_t, CreatePartition,
                (const std::string &diskId, const PartitionParams &params), (override));
    MOCK_METHOD(int32_t, DeletePartition,
                (const std::string &diskId, int32_t partitionNum), (override));
    MOCK_METHOD(int32_t, FormatPartition,
                (const std::string &diskId, int32_t partitionNum, const FormatParams &params), (override));
    MOCK_METHOD(int32_t, ResetProxy, (), (override));
    MOCK_METHOD(int32_t, NotifyMtpMounted,
                (const std::string &id, const std::string &path, const std::string &desc,
                 const std::string &uuid, const std::string &fsType), (override));
    MOCK_METHOD(int32_t, NotifyMtpUnmounted, (const std::string &id, const bool isBadRemove), (override));
    MOCK_METHOD(int32_t, OnBlockDiskUevent, (const std::string &rawUeventMsg), (override));
};

} // DiskManager
} // OHOS

#endif // DISK_MANAGER_CLIENT_MOCK_H
