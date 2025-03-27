/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_ENUM_H
#define OHOS_STORAGE_DAEMON_ENUM_H

namespace OHOS {
namespace StorageDaemon {
class IStorageDaemonEnum {
public:
    enum {
        CRYPTO_FLAG_EL1 = 1,
        CRYPTO_FLAG_EL2 = 2,
        CRYPTO_FLAG_EL3 = 4,
        CRYPTO_FLAG_EL4 = 8,
        CRYPTO_FLAG_EL5 = 16,
    };
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // OHOS_STORAGE_DAEMON_ENUM_H