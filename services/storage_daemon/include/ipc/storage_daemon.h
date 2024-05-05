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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H

#include <mutex>
#include <vector>
#include "ipc/storage_daemon_stub.h"
#include "system_ability_status_change_stub.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageDaemon {

struct UserTokenSecret {
    const std::vector<uint8_t> token;
    const std::vector<uint8_t> oldSecret;
    const std::vector<uint8_t> newSecret;
    uint64_t secureUid;
};
class StorageDaemon : public StorageDaemonStub {
public:
    StorageDaemon() = default;
    virtual ~StorageDaemon() = default;

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

    // fscrypt api, add fs mutex in KeyManager
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
    virtual int32_t GetLockScreenStatus(uint32_t user, bool &lockScreenStatus) override;
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
    class SystemAbilityStatusChangeListener : public OHOS::SystemAbilityStatusChangeStub {
    public:
        SystemAbilityStatusChangeListener() = default;
        ~SystemAbilityStatusChangeListener() = default;
        void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
        void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    };

private:
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string GetNeedRestoreFilePath(int32_t userId, const std::string &user_dir);
    std::string GetNeedRestoreFilePathByType(int32_t userId, KeyType type);
    int32_t PrepareUserDirsAndUpdateUserAuth(uint32_t userId, KeyType type,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret);
    int32_t RestoreUserKey(int32_t userId, uint32_t flags);
    bool IsNeedRestorePathExist(uint32_t userId, bool needCheckEl1);
    int32_t RestoreUserOneKey(int32_t userId, KeyType type);
#endif
    int32_t GetCryptoFlag(KeyType type, uint32_t &flags);
    int32_t GenerateKeyAndPrepareUserDirs(uint32_t userId, KeyType type,
                                          const std::vector<uint8_t> &token,
                                          const std::vector<uint8_t> &secret);
    int32_t ActiveUserKeyAndPrepare(uint32_t userId, KeyType type,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret);
    int32_t RestoreconElX(uint32_t userId);
    void AncoInitCryptKey();
    void AncoActiveCryptKey(uint32_t userId);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H
