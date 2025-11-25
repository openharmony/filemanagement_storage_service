/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_MOCK_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_MOCK_H

#include "iremote_proxy.h"
#include "istorage_manager.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerProxy : public IRemoteProxy<IStorageManager> {
public:
    explicit StorageManagerProxy(const sptr<IRemoteObject>& impl)
        : IRemoteProxy<IStorageManager>(impl) {}
    ~StorageManagerProxy() override {}

    int32_t PrepareAddUser(int32_t userId, uint32_t flags) override;
    int32_t RemoveUser(int32_t userId, uint32_t flags) override;
    int32_t PrepareStartUser(int32_t userId) override;
    int32_t StopUser(int32_t userId) override;
    int32_t CompleteAddUser(int32_t userId) override;
    int32_t GetFreeSizeOfVolume(const std::string& volumeUuid, int64_t &freeSize) override;
    int32_t GetTotalSizeOfVolume(const std::string& volumeUuid, int64_t &totalSize) override;
    int32_t GetBundleStats(const std::string &pkgName, BundleStats &bundleStats, int32_t appIndex,
                           uint32_t statFlag) override;
    int32_t GetSystemSize(int64_t &systemSize) override;
    int32_t GetTotalSize(int64_t &totalSize) override;
    int32_t GetFreeSize(int64_t &freeSize) override;
    int32_t GetUserStorageStats(StorageStats &storageStats) override;
    int32_t GetUserStorageStats(int32_t userId, StorageStats &storageStats) override;
    int32_t GetUserStorageStatsByType(int32_t userId, StorageStats &storageStats, const std::string& type) override;
    int32_t GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag) override;
    int32_t NotifyVolumeCreated(const VolumeCore& vc) override;
    int32_t NotifyVolumeMounted(const VolumeInfoStr &volumeInfoStr) override;
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t type) override;
    int32_t NotifyVolumeStateChanged(const std::string& volumeId, uint32_t state) override;
    int32_t NotifyVolumeDamaged(const VolumeInfoStr &volumeInfoStr) override;
    int32_t Mount(const std::string& volumeId) override;
    int32_t Unmount(const std::string& volumeId) override;
    int32_t TryToFix(const std::string& volumeId) override;
    int32_t GetAllVolumes(std::vector<VolumeExternal> &vecOfVol) override;
    int32_t NotifyDiskCreated(const Disk& disk) override;
    int32_t NotifyDiskDestroyed(const std::string& diskId) override;
    int32_t Partition(const std::string& diskId, int32_t type) override;
    int32_t GetAllDisks(std::vector<Disk> &vecOfDisk) override;
    int32_t GetVolumeByUuid(const std::string& fsUuid, VolumeExternal &vc) override;
    int32_t GetVolumeById(const std::string& volumeId, VolumeExternal &vc) override;
    int32_t SetVolumeDescription(const std::string& fsUuid, const std::string& description) override;
    int32_t Format(const std::string& volumeId, const std::string& fsType) override;
    int32_t GetDiskById(const std::string& diskId, Disk &disk) override;
    int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse) override;

    // fscrypt api
    int32_t DeleteUserKeys(uint32_t userId) override;
    int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                           const std::vector<uint8_t> &token,
                           const std::vector<uint8_t> &oldSecret,
                           const std::vector<uint8_t> &newSecret) override;
    int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                         const std::vector<uint8_t> &newSecret,
                                         uint64_t secureUid,
                                         uint32_t userId,
                                         const std::vector<std::vector<uint8_t>> &plainText) override;
    int32_t ActiveUserKey(uint32_t userId,
                          const std::vector<uint8_t> &token,
                          const std::vector<uint8_t> &secret) override;
    int32_t InactiveUserKey(uint32_t userId) override;
    int32_t UpdateUserPublicDirPolicy(uint32_t userId) override;
    int32_t LockUserScreen(uint32_t userId) override;
    int32_t UnlockUserScreen(uint32_t userId,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret) override;
    int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) override;
    int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false) override;
    int32_t GenerateAppkey(uint32_t hashId, uint32_t userId, std::string &keyId, bool needReSet = false) override;
    int32_t DeleteAppkey(const std::string& keyId) override;
    int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false) override;
    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive) override;
    int32_t CreateRecoverKey(uint32_t userId,
                             uint32_t userType,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret) override;
    int32_t SetRecoverKey(const std::vector<uint8_t> &key) override;
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key) override;
    int32_t NotifyMtpMounted(const std::string &id, const std::string &path, const std::string &desc,
                             const std::string &uuid) override;
    int32_t NotifyMtpUnmounted(const std::string &id, const std::string &path, bool isBadRemove) override;
    int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback>& callback) override;
    int32_t UnregisterUeceActivationCallback() override;
    virtual int32_t CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid) override;
    virtual int32_t DeleteUserDir(const std::string &path) override;

    // app file share api
    int32_t CreateShareFile(const StorageFileRawData &rawData,
                            uint32_t tokenId,
                            uint32_t flag,
                            std::vector<int32_t> &funcResult) override;
    int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData) override;

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid, const std::string &bundleDataDirPath,
        int32_t limitSizeMb) override;
    int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) override;

    // dfs service
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override;

    int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override;
    int32_t MountMediaFuse(int32_t userId, int32_t &devFd) override;
    int32_t UMountMediaFuse(int32_t userId) override;
    // file mgr fuse
    int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd) override;
    int32_t UMountFileMgrFuse(int32_t userId, const std::string &path) override;
    // file lock
    int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy) override;
    int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles) override;
    int32_t UMountDisShareFile(int32_t userId, const std::string &networkId) override;
    int32_t InactiveUserPublicDirKey(uint32_t userId) override;
    int32_t NotifyUserChangedEvent(uint32_t userId, uint32_t eventType) override;
    int32_t SetExtBundleStats(uint32_t userId, const std::string &businessName, uint64_t businessSize) override;
    int32_t GetExtBundleStats(uint32_t userId, const std::string &businessName, uint64_t &businessSize) override;
private:
    static inline BrokerDelegator<StorageManagerProxy> delegator_;
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MANAGER_PROXY_MOCK_H
