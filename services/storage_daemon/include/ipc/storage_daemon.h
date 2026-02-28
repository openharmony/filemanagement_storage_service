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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H

#include "system_ability_status_change_stub.h"
#include "storage_service_constant.h"
#include "storage_service_constants.h"
#include "storage_daemon_provider.h"
namespace OHOS {
namespace StorageDaemon {

struct UserTokenSecret {
    const std::vector<uint8_t> token;
    const std::vector<uint8_t> oldSecret;
    const std::vector<uint8_t> newSecret;
    uint64_t secureUid;
};
class StorageDaemon {
public:

    static StorageDaemon &GetInstance(void)
    {
        static StorageDaemon instance;
        return instance;
    }
    int32_t PrepareUserDirs(int32_t userId, uint32_t flags);
    int32_t DestroyUserDirs(int32_t userId, uint32_t flags);
    int32_t CompleteAddUser(int32_t userId);

    // fscrypt api, add fs mutex in KeyManager
    int32_t InitGlobalKey(void);
    int32_t InitGlobalUserKeys(void);
    int32_t EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList);
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
    int32_t GetLockScreenStatus(uint32_t user, bool &lockScreenStatus);
    int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet = false);
    int32_t DeleteAppkey(uint32_t userId, const std::string &keyId);
    int32_t CreateRecoverKey(uint32_t userId,
                             uint32_t userType,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret);
    int32_t SetRecoverKey(const std::vector<uint8_t> &key);
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key);
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level);

    // app file share api
    int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);

    // file lock
    int32_t IsFileOccupied(const std::string &path,
                           const std::vector<std::string> &inputList,
                           std::vector<std::string> &outputList,
                           bool &isOccupy);

    void SetPriority();

    int32_t InactiveUserPublicDirKey(uint32_t userId);
    int32_t RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback);
    int32_t UnregisterUeceActivationCallback();
    bool IsDirRecursivelyEmpty(const char* dirPath);
private:
    StorageDaemon() = default;
    ~StorageDaemon() = default;
    StorageDaemon(const StorageDaemon &) = delete;
    StorageDaemon& operator=(const StorageDaemon &) = delete;
    StorageDaemon(StorageDaemon &&) = delete;
    StorageDaemon& operator=(StorageDaemon &&) = delete;
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string GetNeedRestoreFilePath(int32_t userId, const std::string &user_dir);
    std::string GetNeedRestoreFilePathByType(int32_t userId, KeyType type);
    std::string GetNeedRestoreVersion(uint32_t userId, KeyType type);
    int32_t PrepareUserDirsAndUpdateUserAuth(uint32_t userId, KeyType type,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret);
    int32_t PrepareUserDirsAndUpdateUserAuthOld(uint32_t userId, KeyType type,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret);
    int32_t PrepareUserDirsAndUpdateUserAuthVx(uint32_t userId, KeyType type,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret,
                                             const std::string needRestoreVersion);
    int32_t PrepareUserDirsAndUpdateAuth4Nato(uint32_t userId, KeyType type, const std::vector<uint8_t> &token);
    int32_t PrepareUeceDir(uint32_t userId);
    int32_t RestoreUserKey(int32_t userId, uint32_t flags);
    bool IsNeedRestorePathExist(uint32_t userId, bool needCheckEl1);
    int32_t RestoreOneUserKey(int32_t userId, KeyType type);
#endif
    int32_t IsDirPathSupport(const std::string &dirPath);
    StorageService::EncryptionLevel UintToKeyType(uint32_t type);
    int32_t GetCryptoFlag(KeyType type, uint32_t &flags);
    int32_t GenerateKeyAndPrepareUserDirs(uint32_t userId, KeyType type,
                                          const std::vector<uint8_t> &token,
                                          const std::vector<uint8_t> &secret);
    int32_t ActiveUserKeyAndPrepare(uint32_t userId, KeyType type,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret);
    int32_t ActiveUserKeyAndPrepareElX(uint32_t userId,
                                       const std::vector<uint8_t> &token,
                                       const std::vector<uint8_t> &secret);
    int32_t RestoreconElX(uint32_t userId);
    void ActiveAppCloneUserKey();
    void SetDeleteFlag4KeyFiles();
    int32_t ActiveUserKey4Nato(uint32_t userId, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret);
    int32_t ActiveUserKey4Update(uint32_t userId, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret);
    int32_t ActiveUserKey4Single(uint32_t userId, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret);
    void ClearNatoRestoreKey(uint32_t userId, KeyType type, bool isClearAll);
    void ClearAllNatoRestoreKey(uint32_t userId, bool isClearAll);
    void ClearKeyDirInfo(const std::string &path);
    void ClearKeyDir(const std::string &path);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_H
