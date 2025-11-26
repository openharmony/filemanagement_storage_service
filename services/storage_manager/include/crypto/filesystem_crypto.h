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

#ifndef OHOS_STORAGE_MANAGER_FILE_SYSTEM_CRYPTO_H
#define OHOS_STORAGE_MANAGER_FILE_SYSTEM_CRYPTO_H

#include <singleton.h>
#include "iuece_activation_callback.h"
#include "storage_service_constants.h"

namespace OHOS {
namespace StorageManager {
class FileSystemCrypto final : public NoCopyable {
public:
    static FileSystemCrypto &GetInstance(void)
    {
        static FileSystemCrypto instance;
        return instance;
    }
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
    int32_t DeleteAppkey(const std::string keyId);
    int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);
    int32_t CreateRecoverKey(uint32_t userId,
                             uint32_t userType,
                             const std::vector<uint8_t> &token,
                             const std::vector<uint8_t> &secret);
    int32_t SetRecoverKey(const std::vector<uint8_t> &key);
    int32_t ResetSecretWithRecoveryKey(uint32_t userId, uint32_t rkType, const std::vector<uint8_t> &key);
    int32_t InactiveUserPublicDirKey(uint32_t userId);
    int32_t RegisterUeceActivationCallback(const sptr<IUeceActivationCallback> &ueceCallback);
    int32_t UnregisterUeceActivationCallback();
    int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level);
private:
    int32_t CheckUserIdRange(int32_t userId);
    FileSystemCrypto();
    ~FileSystemCrypto();
};
} // StorageManager
} // OHOS
#endif // OHOS_STORAGE_MANAGER_FILE_SYSTEM_CRYPTO_H