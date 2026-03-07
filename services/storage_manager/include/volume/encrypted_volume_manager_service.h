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

#ifndef OHOS_STORAGE_MANAGER_ENCRYPTED_VOLUME_MANAGER_SERVICE_H
#define OHOS_STORAGE_MANAGER_ENCRYPTED_VOLUME_MANAGER_SERVICE_H

#include "volume_external.h"
#include "storage_rl_map.h"

namespace OHOS {
namespace StorageManager {
class EncryptedVolumeManagerService {
public:
    static EncryptedVolumeManagerService &GetInstance(void)
    {
        static EncryptedVolumeManagerService instance;
        return instance;
    }
    //disk crypt api
    int32_t Encrypt(const std::string &volumeId, const std::string &pazzword);
    int32_t GetCryptProgressById(const std::string &volumeId, int32_t &progress);
    int32_t GetCryptUuidById(const std::string &volumeId, std::string &uuid);
    int32_t BindRecoverKeyToPasswd(const std::string &volumeId,
                                const std::string &pazzword,
                                const std::string &recoverKey);
    int32_t UpdateCryptPasswd(const std::string &volumeId,
                            const std::string &pazzword,
                            const std::string &newPazzword);
    int32_t ResetCryptPasswd(const std::string &volumeId,
                            const std::string &recoverKey,
                            const std::string &newPazzword);
    int32_t VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword);
    int32_t Unlock(const std::string &volumeId, const std::string &pazzword);
    int32_t Decrypt(const std::string &volumeId, const std::string &pazzword);

private:
    EncryptedVolumeManagerService();
    ~EncryptedVolumeManagerService();
    EncryptedVolumeManagerService(const EncryptedVolumeManagerService &) = delete;
    EncryptedVolumeManagerService &operator=(const EncryptedVolumeManagerService &) = delete;
    EncryptedVolumeManagerService(const EncryptedVolumeManagerService &&) = delete;
    EncryptedVolumeManagerService &operator=(const EncryptedVolumeManagerService &&) = delete;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_ENCRYPTED_VOLUME_MANAGER_SERVICE_H
