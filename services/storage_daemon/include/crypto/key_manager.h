/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
#define STORAGE_DAEMON_CRYPTO_KEYMANAGER_H

#include "base_key.h"
#include "crypto_delay_handler.h"
#include "ipc/storage_daemon.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *USER_EL1_DIR = "/data/service/el1/public/storage_daemon/sd/el1";
constexpr const char *USER_EL2_DIR = "/data/service/el1/public/storage_daemon/sd/el2";
constexpr const char *USER_EL3_DIR = "/data/service/el1/public/storage_daemon/sd/el3";
constexpr const char *USER_EL4_DIR = "/data/service/el1/public/storage_daemon/sd/el4";
constexpr const char *USER_EL5_DIR = "/data/service/el1/public/storage_daemon/sd/el5";
constexpr const char *UECE_DIR = "data/app/el5";
constexpr const char *RESTORE_DIR = "/latest/need_restore";
constexpr const char *FSCRYPT_VERSION_DIR = "/fscrypt_version";

constexpr const char *NATO_EL2_DIR = "/data/service/el1/public/storage_daemon/sd/el2_NATO";
constexpr const char *NATO_EL3_DIR = "/data/service/el1/public/storage_daemon/sd/el3_NATO";
constexpr const char *NATO_EL4_DIR = "/data/service/el1/public/storage_daemon/sd/el4_NATO";

constexpr const char *MAINTAIN_USER_EL1_DIR = "/mnt/data_old/service/el1/public/storage_daemon/sd/el1";
constexpr const char *MAINTAIN_USER_EL2_DIR = "/mnt/data_old/service/el1/public/storage_daemon/sd/el2";
constexpr const char *MAINTAIN_USER_EL3_DIR = "/mnt/data_old/service/el1/public/storage_daemon/sd/el3";
constexpr const char *MAINTAIN_USER_EL4_DIR = "/mnt/data_old/service/el1/public/storage_daemon/sd/el4";
constexpr const char *MAINTAIN_USER_EL5_DIR = "/mnt/data_old/service/el1/public/storage_daemon/sd/el5";

class KeyManager {
public:
    static KeyManager &GetInstance(void)
    {
        static KeyManager instance;
        return instance;
    }
    int InitGlobalDeviceKey(void);
    int InitGlobalUserKeys(void);
    int GenerateUserKeys(unsigned int user, uint32_t flags);
    int DeleteUserKeys(unsigned int user);
    int EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList);

#ifdef USER_CRYPTO_MIGRATE_KEY
    int UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                       bool needGenerateShield = true);
    int UpdateCeEceSeceUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                                KeyType type, bool needGenerateShield);
#else
    int UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret);
    std::string CheckSecretStatus(struct UserTokenSecret &userTokenSecret);
    void HandleEl2Error(int ret, unsigned int user, const std::string &secretInfo,
                        const std::string &reportPrefix, const std::string &level);
    int UpdateCeEceSeceUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret,
                                KeyType type);

#endif
    int UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                     const std::vector<uint8_t> &newSecret,
                                     uint64_t secureUid,
                                     uint32_t userId,
                                     const std::vector<std::vector<uint8_t>> &plainText);
    int ActiveCeSceSeceUserKey(unsigned int user, KeyType type, const std::vector<uint8_t> &token,
                               const std::vector<uint8_t> &secret);
    int InActiveUserKey(unsigned int user);
    int SetDirectoryElPolicy(unsigned int user, KeyType type,
                             const std::vector<FileList> &vec);
    int UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false);
    int UpdateCeEceSeceKeyContext(uint32_t userId, KeyType type);
    int UpdateClassEBackUp(uint32_t userId);
    int getElxKeyPath(unsigned int user, KeyType type, std::string &elxKeyPath);
    int LockUserScreen(uint32_t user);
    int UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token,
                         const std::vector<uint8_t> &secret);
    int GetLockScreenStatus(uint32_t user, bool &lockScreenStatus);
    int GenerateAppkey(uint32_t user, uint32_t hashId, std::string &keyId, bool needReSet = false);
    int DeleteAppkey(uint32_t user, const std::string keyId);
    int GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
                         const std::vector<uint8_t> &secret);
    int SetRecoverKey(const std::vector<uint8_t> &key);
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key);
#ifdef EL5_FILEKEY_MANAGER
    int RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback);
    int UnregisterUeceActivationCallback();
#endif
    int NotifyUeceActivation(uint32_t userId, int32_t resultCode, bool needGetAllAppKey);
#ifdef USER_CRYPTO_MIGRATE_KEY
    int RestoreUserKey(uint32_t userId, KeyType type);
    int32_t ClearAppCloneUserNeedRestore(unsigned int userId, std::string elNeedRestorePath);
#endif
    std::string GetKeyDirByUserAndType(unsigned int user, KeyType type);
    std::string GetKeyDirByType(KeyType type);
    int GenerateUserKeyByType(unsigned int user, KeyType type,
                              const std::vector<uint8_t> &token,
                              const std::vector<uint8_t> &secret);
    int TryToFixUserCeEceSeceKey(unsigned int userId, KeyType type,
                                 const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret);
    int TryToFixUeceKey(unsigned int userId,
                        const std::vector<uint8_t> &token,
                        const std::vector<uint8_t> &secret);
    int ActiveElxUserKey4Nato(unsigned int user, KeyType type, const KeyBlob &authToken);
    std::string GetNatoNeedRestorePath(uint32_t userId, KeyType type);
    // userElKeys_ function
    void SaveUserElKey(unsigned int user, KeyType type, std::shared_ptr<BaseKey> elKey);
    void DeleteElKey(unsigned int user, KeyType type);
    bool HasElkey(uint32_t userId, KeyType type);
    friend class KeyManagerExt;
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, StorageService::EncryptionLevel level);
    bool IsDirRecursivelyEmpty(const char* dirPath);

private:
    KeyManager()
    {
        hasGlobalDeviceKey_ = false;
        getLockStatusTime_[0] = 0;
        getLockStatusTime_[1] = 0;
    }
    ~KeyManager() {}
    KeyManager(const KeyManager &) = delete;
    KeyManager& operator= (const KeyManager &) = delete;
    KeyManager(KeyManager &&) = delete;
    KeyManager& operator= (KeyManager &&) = delete;
    int GenerateAndInstallDeviceKey(const std::string &dir);
    int RestoreDeviceKey(const std::string &dir);
    int DeleteGlobalDeviceKey(const std::string &dir);
    int GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int GenerateAndInstallEl5Key(uint32_t userId, const std::string &dir, const UserAuth &auth);
    int RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int LoadAllUsersEl1Key(void);
    int InitUserElkeyStorageDir(void);
    int DoDeleteUserKeys(unsigned int user);
    int DoDeleteUserCeEceSeceKeys(unsigned int user, const std::string userDir, KeyType type);
    int UpgradeKeys(const std::vector<FileList> &dirInfo);
    int UpdateESecret(unsigned int user, struct UserTokenSecret &tokenSecret);
    bool ResetESecret(unsigned int user, std::shared_ptr<BaseKey> &elKey);
    std::shared_ptr<BaseKey> GetBaseKey(const std::string& dir);
    std::shared_ptr<BaseKey> GetUserElKey(unsigned int user, KeyType type, bool isSave = true);
    bool IsNeedClearKeyFile(std::string file);
    void ClearKeyFilesForPath(const std::string &path);
    bool CheckDir(KeyType type, std::string keyDir, unsigned int user);
    int ActiveUece(unsigned int user,
                   std::shared_ptr<BaseKey> elKey,
                   const std::vector<uint8_t> &token,
                   const std::vector<uint8_t> &secret);
    void ProcUpgradeKey(const std::vector<FileList> &dirInfo);
    int GenerateElxAndInstallUserKey(unsigned int user);
    int ActiveUeceUserKey(unsigned int user,
                          const std::vector<uint8_t> &token,
                          const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey);
    int ActiveElXUserKey(unsigned int user,
                         const std::vector<uint8_t> &token, KeyType keyType,
                         const std::vector<uint8_t> &secret, std::shared_ptr<BaseKey> elKey);
    int InactiveUserElKey(unsigned int user, KeyType type);
    int CheckAndDeleteEmptyEl5Directory(std::string keyDir, unsigned int user);
    bool GetUserDelayHandler(uint32_t userId, std::shared_ptr<DelayHandler> &delayHandler);
    bool IsUeceSupport();
    int IsUeceSupportWithErrno();
    bool IsUserCeDecrypt(uint32_t userId);
    int32_t UnlockEceSece(uint32_t user, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret);
    int32_t UnlockUece(uint32_t user, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret);
    int CheckUserPinProtect(unsigned int userId, const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret);
    bool IsWorkDirExist(std::string type, int32_t userId);
    int GenerateIntegrityDirs(int32_t userId, KeyType type);
    int CheckAndFixUserKeyDirectory(unsigned int user);
    int DoChangerPinCodeClassE(unsigned int user, std::shared_ptr<BaseKey> &el5Key);
    bool HashElxActived(unsigned int user, KeyType type);
    bool IsAppCloneUser(unsigned int user);
    int CheckNeedRestoreVersion(unsigned int user, KeyType type);
    int GenerateAppkeyWithRecover(uint32_t userId, uint32_t hashId, std::string &keyId);
    int UpdateClassEBackUpFix(uint32_t userId);

#ifdef RECOVER_KEY_TEE_ENVIRONMENT
    int32_t FileBasedEncryptfsMount();
    int32_t InstallEmptyUserKeyForRecovery(uint32_t userId);
#endif
    using KeyMap = std::map<KeyType, std::shared_ptr<BaseKey>>;
    std::map<unsigned int, KeyMap> userElKeys_;
    std::map<unsigned int, std::shared_ptr<DelayHandler>> userLockScreenTask_;
    std::shared_ptr<BaseKey> globalEl1Key_ { nullptr };
    std::map<unsigned int, bool> userPinProtect;
    std::map<unsigned int, bool> saveLockScreenStatus;
    std::map<unsigned int, bool> saveESecretStatus;
    std::mutex keyMutex_;
    int64_t getLockStatusTime_[2];
    bool hasGlobalDeviceKey_;

#ifdef EL5_FILEKEY_MANAGER
    std::mutex ueceMutex_;
    sptr<StorageManager::IUeceActivationCallback> ueceCallback_ = nullptr;
#endif
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
