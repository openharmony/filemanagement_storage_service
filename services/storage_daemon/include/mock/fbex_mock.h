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
#ifndef STORAGE_DAEMON_FBEX_MOCK_H
#define STORAGE_DAEMON_FBEX_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "fbex.h"

namespace OHOS {
namespace StorageDaemon {
class IFbexMoc {
public:
    virtual ~IFbexMoc() = default;
public:
    virtual bool IsFBEXSupported() = 0;
    virtual int InstallKeyToKernel(uint32_t userId, uint32_t type, KeyBlob &iv,
        uint8_t flag, const KeyBlob &authToken) = 0;
    virtual int UninstallOrLockUserKeyToKernel(uint32_t, uint32_t, uint8_t *, uint32_t, bool) = 0;
    virtual int InstallDoubleDeKeyToKernel(UserIdToFbeStr &userIdToFbe, KeyBlob &iv,
        uint8_t flag, const KeyBlob &authToken) = 0;
    virtual int LockScreenToKernel(uint32_t userId) = 0;
    virtual int UnlockScreenToKernel(uint32_t userId, uint32_t type,
        uint8_t *iv, uint32_t size, const KeyBlob &authToken) = 0;
    virtual int ReadESecretToKernel(UserIdToFbeStr &, uint32_t, KeyBlob &, const KeyBlob &, bool &) = 0;
    virtual int WriteESecretToKernel(UserIdToFbeStr &, uint32_t, uint8_t *, uint32_t length) = 0;
    virtual bool IsMspReady() = 0;
    virtual int GetStatus() = 0;
    virtual int InstallEL5KeyToKernel(uint32_t, uint32_t, uint8_t, bool &, bool &) = 0;
    virtual int DeleteClassEPinCode(uint32_t userIdSingle, uint32_t userIdDouble) = 0;
    virtual int ChangePinCodeClassE(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport) = 0;
    virtual int GenerateAppkey(UserIdToFbeStr &, uint32_t, std::unique_ptr<uint8_t[]> &, uint32_t) = 0;
    virtual int LockUece(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport) = 0;
public:
    static inline std::shared_ptr<IFbexMoc> fbexMoc = nullptr;
};

class FbexMoc : public IFbexMoc {
public:
    MOCK_METHOD0(IsFBEXSupported, bool());
    MOCK_METHOD5(InstallKeyToKernel, int(uint32_t userId, uint32_t type, KeyBlob &iv,
        uint8_t flag, const KeyBlob &authToken));
    MOCK_METHOD5(UninstallOrLockUserKeyToKernel, int(uint32_t, uint32_t, uint8_t *, uint32_t, bool));
    MOCK_METHOD4(InstallDoubleDeKeyToKernel, int(UserIdToFbeStr &, KeyBlob &iv,
        uint8_t flag, const KeyBlob &authToken));
    MOCK_METHOD1(LockScreenToKernel, int(uint32_t userId));
    MOCK_METHOD5(UnlockScreenToKernel, int(uint32_t userId, uint32_t type,
        uint8_t *iv, uint32_t size, const KeyBlob &authToken));
    MOCK_METHOD5(ReadESecretToKernel, int(UserIdToFbeStr &, uint32_t, KeyBlob &, const KeyBlob &, bool &));
    MOCK_METHOD4(WriteESecretToKernel, int(UserIdToFbeStr &, uint32_t, uint8_t *, uint32_t length));
    MOCK_METHOD0(IsMspReady, bool());
    MOCK_METHOD0(GetStatus, int());
    MOCK_METHOD5(InstallEL5KeyToKernel, int(uint32_t, uint32_t, uint8_t, bool &, bool &));
    MOCK_METHOD2(DeleteClassEPinCode, int(uint32_t userIdSingle, uint32_t userIdDouble));
    MOCK_METHOD3(ChangePinCodeClassE, int(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport));
    MOCK_METHOD4(GenerateAppkey, int(UserIdToFbeStr &, uint32_t, std::unique_ptr<uint8_t[]> &, uint32_t));
    MOCK_METHOD3(LockUece, int(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport));
};
}
}
#endif