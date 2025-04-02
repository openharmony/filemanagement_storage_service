/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_RECOVERY_MANAGER_MOCK_H
#define STORAGE_DAEMON_RECOVERY_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "recover_manager.h"

namespace OHOS {
namespace StorageDaemon {
class IRecoveryManager {
public:
    virtual ~IRecoveryManager() = default;
    virtual int CreateRecoverKey(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret, const std::vector<KeyBlob> &originIv) = 0;
    virtual int SetRecoverKey(const std::vector<uint8_t> &key) = 0;
    virtual int32_t ResetSecretWithRecoveryKey() = 0;
public:
    static inline std::shared_ptr<IRecoveryManager> recoveryMgrMock = nullptr;
};

class RecoveryMgrMock : public IRecoveryManager {
public:
    MOCK_METHOD5(CreateRecoverKey, int(uint32_t userId, uint32_t userType, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret, const std::vector<KeyBlob> &originIv));
    MOCK_METHOD1(SetRecoverKey, int(const std::vector<uint8_t> &key));
    MOCK_METHOD0(ResetSecretWithRecoveryKey, int32_t());
};
}
}
#endif