/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef MOCK_STORAGE_MANAGER_STUB_H
#define MOCK_STORAGE_MANAGER_STUB_H

#include "gmock/gmock.h"

#include "ipc/storage_manager_stub.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerStubMock : public StorageManagerStub {
public:
    MOCK_METHOD2(PrepareAddUser, int32_t(int32_t, uint32_t));
    MOCK_METHOD2(RemoveUser, int32_t(int32_t, uint32_t));
    MOCK_METHOD1(PrepareStartUser, int32_t(int32_t));
    MOCK_METHOD1(StopUser, int32_t(int32_t));
    MOCK_METHOD1(CompleteAddUser, int32_t(int32_t));

    MOCK_METHOD2(GetFreeSizeOfVolume, int32_t(std::string, int64_t &));
    MOCK_METHOD2(GetTotalSizeOfVolume, int32_t(std::string, int64_t &));
    MOCK_METHOD4(GetBundleStats, int32_t(std::string, BundleStats &, int32_t, uint32_t));
    MOCK_METHOD1(GetSystemSize, int32_t(int64_t &));
    MOCK_METHOD1(GetTotalSize, int32_t(int64_t &));
    MOCK_METHOD1(GetFreeSize, int32_t(int64_t &));
    MOCK_METHOD1(GetUserStorageStats, int32_t(StorageStats &));
    MOCK_METHOD2(GetUserStorageStats, int32_t(int32_t, StorageStats &));
    MOCK_METHOD3(GetUserStorageStatsByType, int32_t(int32_t, StorageStats &, std::string));
    MOCK_METHOD2(GetCurrentBundleStats, int32_t(BundleStats &, uint32_t));

    MOCK_METHOD1(NotifyVolumeCreated, int32_t(VolumeCore));
    MOCK_METHOD5(NotifyVolumeMounted, int32_t(std::string, int32_t, std::string, std::string, std::string));
    MOCK_METHOD2(NotifyVolumeStateChanged, int32_t(std::string, VolumeState));
    MOCK_METHOD1(Mount, int32_t(std::string));
    MOCK_METHOD1(Unmount, int32_t(std::string));
    MOCK_METHOD1(GetAllVolumes, int32_t(std::vector<VolumeExternal> &));
    MOCK_METHOD1(NotifyDiskCreated, int32_t(Disk));
    MOCK_METHOD1(NotifyDiskDestroyed, int32_t(std::string));
    MOCK_METHOD2(Partition, int32_t(std::string, int32_t));
    MOCK_METHOD1(GetAllDisks, int32_t(std::vector<Disk> &));
    MOCK_METHOD2(GetVolumeByUuid, int32_t(std::string, VolumeExternal &));
    MOCK_METHOD2(GetVolumeById, int32_t(std::string, VolumeExternal &));
    MOCK_METHOD2(SetVolumeDescription, int32_t(std::string, std::string));
    MOCK_METHOD2(Format, int32_t(std::string, std::string));
    MOCK_METHOD2(GetDiskById, int32_t(std::string, Disk &));
    MOCK_METHOD2(QueryUsbIsInUse, int32_t(const std::string &, bool &));

    MOCK_METHOD2(GenerateUserKeys, int32_t(uint32_t, uint32_t));
    MOCK_METHOD1(DeleteUserKeys, int32_t(uint32_t));
    MOCK_METHOD5(UpdateUserAuth, int32_t(uint32_t, uint64_t, const std::vector<uint8_t> &,
        const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD5(UpdateUseAuthWithRecoveryKey, int32_t(const std::vector<uint8_t> &,
        const std::vector<uint8_t> &, uint64_t, uint32_t, std::vector<std::vector<uint8_t>> &));
    MOCK_METHOD3(ActiveUserKey, int32_t(uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD1(InactiveUserKey, int32_t(uint32_t));
    MOCK_METHOD2(UpdateKeyContext, int32_t(uint32_t, bool));
    MOCK_METHOD3(CreateShareFile, std::vector<int32_t> (const std::vector<std::string> &, uint32_t, uint32_t));
    MOCK_METHOD2(DeleteShareFile, int32_t (uint32_t, const std::vector<std::string> &));
    MOCK_METHOD1(LockUserScreen, int32_t(uint32_t));
    MOCK_METHOD3(UnlockUserScreen, int32_t(uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD2(GetLockScreenStatus, int32_t(uint32_t, bool &));
    MOCK_METHOD4(MountDfsDocs, int32_t(int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD4(UMountDfsDocs, int32_t(int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD2(UpdateMemoryPara, int32_t (int32_t, int32_t &));
    MOCK_METHOD5(GetBundleStatsForIncrease, int32_t(uint32_t, const std::vector<std::string> &,
        const std::vector<int64_t> &, std::vector<int64_t> &, std::vector<int64_t> &));
    MOCK_METHOD4(GenerateAppkey, int32_t(uint32_t, uint32_t, std::string &, bool));
    MOCK_METHOD4(SetBundleQuota, int32_t(const std::string &, int32_t, const std::string &, int32_t));
    MOCK_METHOD1(DeleteAppkey, int32_t(const std::string keyId));
    MOCK_METHOD3(GetFileEncryptStatus, int32_t(uint32_t, bool &, bool));
    MOCK_METHOD2(GetUserNeedActiveStatus, int32_t(uint32_t, bool &));
    MOCK_METHOD4(CreateRecoverKey, int32_t(uint32_t, uint32_t, const std::vector<uint8_t> &,
        const std::vector<uint8_t> &));
    MOCK_METHOD1(SetRecoverKey, int32_t(const std::vector<uint8_t> &));
    MOCK_METHOD3(ResetSecretWithRecoveryKey, int32_t(uint32_t, uint32_t, const std::vector<uint8_t> &));
    MOCK_METHOD4(NotifyMtpMounted, int32_t(const std::string &, const std::string &, const std::string &,
        const std::string &));
    MOCK_METHOD3(NotifyMtpUnmounted, int32_t(const std::string &, const std::string &, const bool));

    MOCK_METHOD2(MountMediaFuse, int32_t (int32_t, int32_t &));
    MOCK_METHOD1(UMountMediaFuse, int32_t (int32_t));
    MOCK_METHOD3(MountFileMgrFuse, int32_t (int32_t, const std::string &, int32_t &));
    MOCK_METHOD2(UMountFileMgrFuse, int32_t (int32_t, const std::string &));
    MOCK_METHOD4(IsFileOccupied, int32_t (const std::string &, const std::vector<std::string> &,
        std::vector<std::string> &, bool &));
};
}  // namespace StorageManager
}  // namespace OHOS
#endif /* MOCK_STORAGE_MANAGER_STUB_H */
