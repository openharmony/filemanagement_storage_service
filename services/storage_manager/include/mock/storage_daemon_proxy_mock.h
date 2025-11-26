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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_MOCK_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_MOCK_H

#include "iremote_proxy.h"
#include "istorage_daemon.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonProxy : public IRemoteProxy<IStorageDaemon> {
public:
    StorageDaemonProxy(const sptr<IRemoteObject> &impl);
    virtual int32_t Shutdown() override;

    virtual int32_t Mount(const std::string &volId, uint32_t flags) override;
    virtual int32_t UMount(const std::string &volId) override;
    virtual int32_t TryToFix(const std::string &volId, uint32_t flags) override;
    virtual int32_t Check(const std::string &volId) override;
    virtual int32_t Format(const std::string &volId, const std::string &fsType) override;
    virtual int32_t Partition(const std::string &diskId, int32_t type) override;
    virtual int32_t SetVolumeDescription(const std::string &volId, const std::string &description) override;
    virtual int32_t QueryUsbIsInUse(const std::string &diskPath, bool &isInUse) override;

    virtual int32_t StartUser(int32_t userId) override;
    virtual int32_t StopUser(int32_t userId) override;
    virtual int32_t PrepareUserDirs(int32_t userId, uint32_t flags) override;
    virtual int32_t DestroyUserDirs(int32_t userId, uint32_t flags) override;
    virtual int32_t CompleteAddUser(int32_t userId) override;

    // fscrypt api
    virtual int32_t InitGlobalKey(void) override;
    virtual int32_t InitGlobalUserKeys(void) override;
    virtual int32_t DeleteUserKeys(uint32_t userId) override;
    virtual int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override;
    virtual int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                 const std::vector<uint8_t> &newSecret,
                                                 uint64_t secureUid,
                                                 uint32_t userId,
                                                 const std::vector<std::vector<uint8_t>> &plainText) override;
    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override;
    virtual int32_t InactiveUserKey(uint32_t userId) override;
    virtual int32_t UpdateUserPublicDirPolicy(uint32_t userId) override;
    virtual int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false) override;
    virtual int32_t MountCryptoPathAgain(uint32_t userId) override;
    virtual int32_t LockUserScreen(uint32_t userId) override;
    virtual int32_t UnlockUserScreen(uint32_t userId,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override;
    virtual int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) override;
    virtual int32_t GenerateAppkey(uint32_t userId, uint32_t hashId,
                                   std::string &keyId, bool needReSet = false) override;
    virtual int32_t DeleteAppkey(uint32_t userId, const std::string &keyId) override;
    virtual int32_t CreateRecoverKey(uint32_t userId,
                                     uint32_t userType,
                                     const std::vector<uint8_t> &token,
                                     const std::vector<uint8_t> &secret) override;
    virtual int32_t SetRecoverKey(const std::vector<uint8_t> &key) override;
    virtual int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType,
                                               const std::vector<uint8_t> &key) override;

    // app file share api
    virtual int32_t CreateShareFile(const StorageFileRawData &rawData,
                                    uint32_t tokenId,
                                    uint32_t flag,
                                    std::vector<int32_t> &funcResult) override;
    virtual int32_t DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData) override;

    virtual int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb) override;
    virtual int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size) override;
    virtual int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) override;
    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override;
    virtual int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override;
    virtual int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false) override;
    virtual int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive) override;
    virtual int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t type) override;

    virtual int32_t MountMediaFuse(int32_t userId, int32_t &devFd) override;
    virtual int32_t UMountMediaFuse(int32_t userId) override;
    // file mgr fuse
    virtual int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd) override;
    virtual int32_t UMountFileMgrFuse(int32_t userId, const std::string &path) override;
    // file lock
    virtual int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy) override;
    virtual int32_t MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles) override;
    virtual int32_t UMountDisShareFile(int32_t userId, const std::string &networkId) override;
    virtual int32_t InactiveUserPublicDirKey(uint32_t userId) override;
    virtual int32_t QueryOccupiedSpaceForSa(const std::string &storageStatus,
        const std::map<int32_t, std::string> &bundleNameAndUid) override;
    virtual int32_t MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd) override;
    virtual int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback>& callback);
    virtual int32_t UnregisterUeceActivationCallback();
    virtual int32_t CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid) override;
    virtual int32_t DeleteUserDir(const std::string &path) override;
    virtual int32_t GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks) override;
    virtual int32_t GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs,
        std::vector<DirSpaceInfo> &outDirs) override;
    virtual int32_t SetStopScanFlg(bool stop) override;
    virtual int32_t GetAncoSizeData(std::string &outExtraData) override;
private:
    static inline BrokerDelegator<StorageDaemonProxy> delegator_;
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_MOCK_H