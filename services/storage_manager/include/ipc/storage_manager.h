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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_H

#include <cstdint>
#include <string>
#include "istorage_manager.h"
#include "storage_service_constants.h"

namespace OHOS {
namespace StorageManager {
class StorageManager {
public:
    static StorageManager &GetInstance(void)
    {
        static StorageManager instance;
        return instance;
    }
    int32_t PrepareAddUser(int32_t userId, uint32_t flags);
    int32_t RemoveUser(int32_t userId, uint32_t flags);
    int32_t PrepareStartUser(int32_t userId);
    int32_t StopUser(int32_t userId);
    int32_t CompleteAddUser(int32_t userId);

    int32_t GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize);
    int32_t GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize);
    int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex, uint32_t statFlag);
    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetFreeSize(int64_t &freeSize);
    int32_t GetUserStorageStats(StorageStats &storageStats);
    int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats);
    int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag);
    int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, const std::string &type);

    int32_t NotifyVolumeCreated(const VolumeCore& vc);
    int32_t NotifyVolumeMounted(const VolumeInfoStr &volumeInfoStr);
    int32_t NotifyVolumeDamaged(const VolumeInfoStr &volumeInfoStr);
    OHOS::StorageManager::VolumeState UintToState(uint32_t state);
    int32_t NotifyVolumeStateChanged(const std::string &volumeId, uint32_t state);

    int32_t Mount(const std::string &volumeId);
    int32_t Unmount(const std::string &volumeId);

    int32_t TryToFix(const std::string &volumeId);

    int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol);

    int32_t NotifyDiskCreated(const Disk& disk);
    int32_t NotifyDiskDestroyed(const std::string &diskId);
    int32_t Partition(const std::string &diskId, int32_t type);
    int32_t GetAllDisks(std::vector<Disk> &vecOfDisk);

    int32_t GetVolumeByUuid(const std::string &fsUuid, VolumeExternal &vc);
    int32_t GetVolumeById(const std::string &volumeId, VolumeExternal &vc);
    int32_t SetVolumeDescription(const std::string &fsUuid, const std::string &description);
    int32_t Format(const std::string &volumeId, const std::string &fsType);
    int32_t GetDiskById(const std::string &diskId, Disk &disk);
    int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse);

    int32_t NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                              const std::string &uuid);
    int32_t NotifyMtpUnmounted(const std::string &id, const std::string &path, bool isBadRemove);
    
    int32_t IsUsbFuseByType(const std::string &fsType, bool &enabled);

     // fscrypt api
    int32_t DeleteUserKeys(uint32_t userId);
    int32_t EraseAllUserEncryptedKeys();
    int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                            const std::vector<uint8_t> &token,
                            const std::vector<uint8_t> &oldSecret,
                            const std::vector<uint8_t> &newSecret);
    int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                          const std::vector<uint8_t> &newSecret,
                                          uint64_t secureUid,
                                          uint32_t userId,
                                          const std::vector<std::vector<uint8_t>> &plainText);
    int32_t ActiveUserKey(uint32_t userId,
                           const std::vector<uint8_t> &token,
                           const std::vector<uint8_t> &secret);
    int32_t InactiveUserKey(uint32_t userId);
    int32_t UpdateUserPublicDirPolicy(uint32_t userId);
    int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false);
    int32_t LockUserScreen(uint32_t userId);
    int32_t UnlockUserScreen(uint32_t userId,
                              const std::vector<uint8_t> &token,
                              const std::vector<uint8_t> &secret);
    int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus);
    int32_t GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet = false);
    int32_t DeleteAppkey(const std::string &keyId);
    int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);
    int32_t CreateRecoverKey(uint32_t userId,
                              uint32_t userType,
                              const std::vector<uint8_t> &token,
                              const std::vector<uint8_t> &secret);
    int32_t SetRecoverKey(const std::vector<uint8_t> &key);
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key);

     // app file share api
    std::vector<int32_t> CreateShareFile(const StorageFileRawData &uriList,
         uint32_t tokenId, uint32_t flag);
    int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &uriList);

    int32_t SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath,
         int32_t limitSizeMb);
    int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize);

     // dfs service
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
         const std::string &networkId, const std::string &deviceId);
    int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
         const std::string &networkId, const std::string &deviceId);

     // media fuse
    int32_t MountMediaFuse(int32_t userId, int32_t &devFd);
    int32_t UMountMediaFuse(int32_t userId);
     // file mgr fuse
    int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd);
    int32_t UMountFileMgrFuse(int32_t userId, const std::string &path);
    int32_t IsFileOccupied(const std::string &path,
                           const std::vector<std::string> &inputList,
                           std::vector<std::string> &outputList,
                           bool &isOccupy);
    int32_t InactiveUserPublicDirKey(uint32_t userId);
    void ResetUserEventRecord(int32_t userId);
    int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles);
    int32_t UMountDisShareFile(int32_t userId, const std::string &networkId);
    int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback);
    int32_t UnregisterUeceActivationCallback();
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level);
    int32_t CheckUserIdRange(int32_t userId);
    int32_t ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs);
    void NotifyUserChangedEvent(uint32_t userId, StorageService::UserChangedEventType eventType);
    std::mutex mutex_;
private:
    StorageManager() = default;
    ~StorageManager() = default;
};
} // namespace StorageManager
} // namespace OHOS

#endif
