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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H
#define OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H

#include <singleton.h>
#include "istorage_daemon.h"
#include "iuece_activation_callback.h"

namespace OHOS {
namespace StorageManager {
class StorageDaemonCommunication : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageDaemonCommunication);

public:
    int32_t Connect();

    int32_t PrepareAddUser(int32_t userId, uint32_t flags);
    int32_t RemoveUser(int32_t userId, uint32_t flags);
    int32_t PrepareStartUser(int32_t userId);
    int32_t StopUser(int32_t userId);
    int32_t CompleteAddUser(int32_t userId);

    int32_t Mount(std::string volumeId, int32_t flag);
    int32_t Unmount(std::string volumeId);
    int32_t TryToFix(std::string volumeId, int32_t flag);
    int32_t Check(std::string volumeId);
    int32_t Partition(std::string diskId, int32_t type);
    int32_t Format(std::string volumeId, std::string type);
    int32_t SetVolumeDescription(std::string volumeId, std::string description);
    int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse);

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
    int32_t MountCryptoPathAgain(int32_t userId);
    int32_t LockUserScreen(uint32_t userId);
    int32_t UnlockUserScreen(uint32_t userId,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret);
    int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);
    int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus);
    int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet = false);
    int32_t DeleteAppkey(uint32_t userId, const std::string keyId);
    int32_t CreateRecoverKey(uint32_t userId,
                             uint32_t userType,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret);
    int32_t SetRecoverKey(const std::vector<uint8_t> &key);
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key);

    int32_t ResetSdProxy();
    void ForceLockUserScreen();

    // app file share api
    std::vector<int32_t> CreateShareFile(const StorageFileRawData &rawData, uint32_t tokenId, uint32_t flag);
    int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &uriList);

    int32_t SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath,
        int32_t limitSizeMb);
    int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size);
    int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize);

    // dfs service
    int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);

    // media fuse mount
    int32_t MountMediaFuse(int32_t userId, int32_t &devFd);
    int32_t UMountMediaFuse(int32_t userId);
    // file mgr fuse
    int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd);
    int32_t UMountFileMgrFuse(int32_t userId, const std::string &path);
    // file lock
    int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy);
    int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles);
    int32_t UMountDisShareFile(int32_t userId, const std::string &networkId);
    int32_t InactiveUserPublicDirKey(uint32_t userId);
    int32_t QueryOccupiedSpaceForSa(std::string &storageStatus,
        const std::map<int32_t, std::string> &bundleNameAndUid);
    int32_t MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd);

    // el5 filekey manager
    int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback);
    int32_t UnregisterUeceActivationCallback();
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level);

    int32_t CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid);
    int32_t DeleteUserDir(const std::string &path);

    // stats radar
    int32_t GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks);
    int32_t GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs, std::vector<DirSpaceInfo> &outDirs);
    int32_t SetStopScanFlag(bool stop = false);
    int32_t GetAncoSizeData(std::string &outExtraData);

    // stats api
    int32_t GetDataSizeByPath(const std::string &path, int64_t &size);
    int32_t GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize);

private:
    sptr<OHOS::StorageDaemon::IStorageDaemon> storageDaemon_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::mutex mutex_;
};

class SdDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    SdDeathRecipient() = default;
    virtual ~SdDeathRecipient() = default;

    virtual void OnRemoteDied(const wptr<IRemoteObject> &object);
};
} // StorageManager
} // OHOS

#endif
