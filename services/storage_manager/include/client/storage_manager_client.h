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

#ifndef STORAGE_MANAGER_CILENT_H
#define STORAGE_MANAGER_CILENT_H

#include "storage_manager_proxy.h"
#include "iuece_activation_callback.h"
#include "storage_service_constants.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerClient {
public:
    static int32_t PrepareAddUser(uint32_t userId, uint32_t flags);
    static int32_t RemoveUser(uint32_t userId, uint32_t flags);
    static int32_t DeleteUserKeys(uint32_t userId);
    static int32_t EraseAllUserEncryptedKeys();
    static int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &oldSecret,
                                  const std::vector<uint8_t> &newSecret);
    static int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                const std::vector<uint8_t> &newSecret,
                                                uint64_t secureUid,
                                                uint32_t userId,
                                                const std::vector<std::vector<uint8_t>> &plainText);
    static int32_t ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret);
    static int32_t InactiveUserKey(uint32_t userId);
    static int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false);
    static int32_t LockUserScreen(uint32_t userId);
    static int32_t UnlockUserScreen(uint32_t userId,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret);
    static int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus);
    static int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    static int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath,
        StorageService::EncryptionLevel level);
    static int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    static int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    static int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);
    static int32_t RegisterUeceActivationCallback(const sptr<StorageManager::IUeceActivationCallback> &ueceCallback);
    static int32_t UnregisterUeceActivationCallback();
private:
    static sptr<IStorageManager> GetStorageManagerProxy(void);
};
}
}
#endif
