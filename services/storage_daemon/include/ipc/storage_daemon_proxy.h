/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_H

#include "iremote_proxy.h"
#include "ipc/istorage_daemon.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonProxy : public IRemoteProxy<IStorageDaemon> {
public:
    StorageDaemonProxy(const sptr<IRemoteObject> &impl);
    virtual int32_t Shutdown() override;

    virtual int32_t Mount(std::string volId, uint32_t flags) override;
    virtual int32_t UMount(std::string volId) override;
    virtual int32_t Check(std::string volId) override;
    virtual int32_t Format(std::string volId, std::string fsType) override;
    virtual int32_t Partition(std::string diskId, int32_t type) override;
    virtual int32_t SetVolumeDescription(std::string volId, std::string description) override;

    virtual int32_t StartUser(int32_t userId) override;
    virtual int32_t StopUser(int32_t userId) override;
    virtual int32_t PrepareUserDirs(int32_t userId, uint32_t flags) override;
    virtual int32_t DestroyUserDirs(int32_t userId, uint32_t flags) override;

    // fscrypt api
    virtual int32_t InitGlobalKey(void) override;
    virtual int32_t InitGlobalUserKeys(void) override;
    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) override;
    virtual int32_t DeleteUserKeys(uint32_t userId) override;
    virtual int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override;
    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override;
    virtual int32_t InactiveUserKey(uint32_t userId) override;
    virtual int32_t UpdateKeyContext(uint32_t userId) override;
    virtual int32_t LockUserScreen(uint32_t userId) override;
    virtual int32_t UnlockUserScreen(uint32_t userId) override;
    virtual int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus) override;
    virtual int32_t MountCryptoPathAgain(uint32_t userId) override;
    virtual int32_t GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId) override;
    virtual int32_t DeleteAppkey(uint32_t userId, const std::string keyId) override;

    // app file share api
    virtual std::vector<int32_t> CreateShareFile(const std::vector<std::string> &uriList,
                                                uint32_t tokenId, uint32_t flag) override;
    virtual int32_t DeleteShareFile(uint32_t tokenId, const std::vector<std::string> &uriList) override;

    virtual int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb) override;
    virtual int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size) override;
    virtual int32_t UpdateMemoryPara(int32_t size, int32_t &oldSize) override;
    virtual int32_t GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
        const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes) override;
    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) override;
private:
    static inline BrokerDelegator<StorageDaemonProxy> delegator_;
    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_PROXY_H