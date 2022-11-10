/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace StorageDaemon {
constexpr uint32_t USERID_GLOBAL_EL1 = 0;
constexpr uint32_t TYPE_EL1 = 0;
constexpr uint32_t TYPE_EL2 = 1;
constexpr uint32_t TYPE_EL3 = 2;
constexpr uint32_t TYPE_EL4 = 3;
constexpr uint32_t TYPE_GLOBAL_EL1 = 4;

constexpr uint32_t FBEX_IV_SIZE = 64;

class FBEX {
public:
    static bool IsFBEXSupported();
    static int InstallKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, uint8_t flag);
    static int UninstallOrLockUserKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, bool destroy);
    static int LockScreenToKernel(uint32_t userId);
    static int UnlockScreenToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size);
    static bool IsMspReady();
    static int GetStatus();
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_FBEX_X
