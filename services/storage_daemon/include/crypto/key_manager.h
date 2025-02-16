/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <map>
#include <memory>
#include <mutex>

#include "base_key.h"
#include "crypto_delay_handler.h"
#include "key_blob.h"
#include "ipc/storage_daemon.h"
#include "storage_service_constant.h"
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

class KeyManager {
public:
    static KeyManager *GetInstance(void)
    {
        static KeyManager instance;
        return &instance;
    }
    int InitGlobalDeviceKey(void);
    int InitGlobalUserKeys(void);
    int GenerateUserKeys(unsigned int user, uint32_t flags);
    int DeleteUserKeys(unsigned int user);

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
    int getEceSeceKeyPath(unsigned int user, KeyType type, std::string &eceSeceKeyPath);
    int LockUserScreen(uint32_t user);
    int UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token,
                         const std::vector<uint8_t> &secret);
    int GetLockScreenStatus(uint32_t user, bool &lockScreenStatus);
    int GenerateAppkey(uint32_t user, uint32_t hashId, std::string &keyId);
    int DeleteAppkey(uint32_t user, const std::string keyId);
    int UnlockUserAppKeys(uint32_t userId, bool needGetAllAppKey);
    int GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
                         const std::vector<uint8_t> &secret);
    int SetRecoverKey(const std::vector<uint8_t> &key);
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
private:
    KeyManager()
    {
        hasGlobalDeviceKey_ = false;
    }
    ~KeyManager() {}
    int GenerateAndInstallDeviceKey(const std::string &dir);
    int RestoreDeviceKey(const std::string &dir);
    int GenerateAndInstallUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int GenerateAndInstallEl5Key(uint32_t userId, const std::string &dir, const UserAuth &auth);
    int RestoreUserKey(uint32_t userId, const std::string &dir, const UserAuth &auth, KeyType type);
    int LoadAllUsersEl1Key(void);
    int InitUserElkeyStorageDir(void);
    bool HasElkey(uint32_t userId, KeyType type);
    int DoDeleteUserKeys(unsigned int user);
    int DoDeleteUserCeEceSeceKeys(unsigned int user, const std::string userDir,
                                  std::map<unsigned int, std::shared_ptr<BaseKey>> &userElKey_);
    int UpgradeKeys(const std::vector<FileList> &dirInfo);
    int UpdateESecret(unsigned int user, struct UserTokenSecret &tokenSecret);
    bool ResetESecret(unsigned int user, std::shared_ptr<BaseKey> &elKey);
    std::shared_ptr<BaseKey> GetBaseKey(const std::string& dir);
    std::shared_ptr<BaseKey> GetUserElKey(unsigned int user, KeyType type);
    void SaveUserElKey(unsigned int user, KeyType type, std::shared_ptr<BaseKey> elKey);
    bool IsNeedClearKeyFile(std::string file);
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
    int InactiveUserElKey(unsigned int user, std::map<unsigned int, std::shared_ptr<BaseKey>> &userElxKey_);
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
    bool HashElxActived(unsigned int user, KeyType type);
    bool HasElxDesc(std::map<unsigned int, std::shared_ptr<BaseKey>> &userElKey_, KeyType type, unsigned int user);
    bool IsAppCloneUser(unsigned int user);
    int CheckNeedRestoreVersion(unsigned int user, KeyType type);
#ifdef EL5_FILEKEY_MANAGER
    int GenerateAndLoadAppKeyInfo(uint32_t userId, const std::vector<std::pair<int, std::string>> &keyInfo);
#endif

    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl1Key_;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl2Key_;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl3Key_;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl4Key_;
    std::map<unsigned int, std::shared_ptr<BaseKey>> userEl5Key_;
    std::map<unsigned int, std::shared_ptr<DelayHandler>> userLockScreenTask_;
    std::shared_ptr<BaseKey> globalEl1Key_ { nullptr };
    std::map<unsigned int, bool> userPinProtect;
    std::map<unsigned int, bool> saveLockScreenStatus;
    std::map<unsigned int, bool> saveESecretStatus;
    std::mutex keyMutex_;
    bool hasGlobalDeviceKey_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYMANAGER_H
