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

#ifndef OHOS_STORAGE_SERVICE_CONSTANTS_H
#define OHOS_STORAGE_SERVICE_CONSTANTS_H

namespace OHOS {
namespace StorageService {
enum EncryptionLevel {
    EL1_SYS_KEY = 0,
    EL1_USER_KEY = 1,
    EL2_USER_KEY = 2,
    EL3_USER_KEY = 3,
    EL4_USER_KEY = 4,
};

enum UserChangedEventType {
    EVENT_USER_UNLOCKED = 0,
    EVENT_USER_SWITCHED = 1,
};
}
} // OHOS

#endif // OHOS_STORAGE_POLICE_CONSTANT_H
