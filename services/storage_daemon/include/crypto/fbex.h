/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef STORAGE_DAEMON_CRYPTO_FBEX_X
#define STORAGE_DAEMON_CRYPTO_FBEX_X

#include <cstdint>
#include <memory>
#include <string>

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t USERID_GLOBAL_EL1 = 0;
constexpr uint32_t TYPE_EL1 = 0;
constexpr uint32_t TYPE_EL2 = 1;
constexpr uint32_t TYPE_EL3 = 3;
constexpr uint32_t TYPE_EL4 = 2;
constexpr uint32_t TYPE_EL5 = 6;
constexpr uint32_t TYPE_GLOBAL_EL1 = 4;

constexpr uint32_t FBEX_IV_SIZE = 64;
constexpr uint32_t FBEX_KEYID_SIZE = 64;
constexpr uint32_t FBEX_E_BUFFER_SIZE = 64;
const uint32_t FBEX_UNSUPPORT_CODE = 0xfbe30203;
const uint32_t UNLOCK_STATUS = 0x2;
const int STORAGE_UNSUPPORT_CODE = 0;
const int SINGLE_ID_INDEX = 0;
const int DOUBLE_ID_INDEX = 1;
const int USER_ID_SIZE = 2;

struct UserIdToFbeStr {
    uint32_t userIds[USER_ID_SIZE];
    int size = USER_ID_SIZE;
};

class FBEX {
public:
    static bool IsFBEXSupported();
    static int InstallKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, uint8_t flag);
    static int UninstallOrLockUserKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, bool destroy);
    static int LockScreenToKernel(uint32_t userId);
    static int UnlockScreenToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size);
    static int ReadESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status,
                                   uint8_t *eBuffer, uint32_t length, bool &isFbeSupport);
    static int WriteESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status, uint8_t *eBuffer, uint32_t length);
    static bool IsMspReady();
    static int GetStatus();
    static int UnlockSendSecret(uint32_t status,uint32_t bufferSize);
    static int InstallEL5KeyToKernel(uint32_t userIdSingle, uint32_t userIdDouble, uint8_t flag,
                                     bool &isSupport, bool &isNeedEncryptClassE);
    static int DeleteClassEPinCode(uint32_t userIdSingle, uint32_t userIdDouble);
    static int ChangePinCodeClassE(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport);
    static int GenerateAppkey(UserIdToFbeStr &userIdToFbe, uint32_t hashId, std::unique_ptr<uint8_t[]> &keyId,
                              uint32_t size);
    static int LockUece(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport);
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_FBEX_X
