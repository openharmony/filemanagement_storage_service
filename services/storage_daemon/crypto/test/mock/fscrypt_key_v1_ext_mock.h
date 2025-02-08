/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_FSCRYPT_KEY_V1_EXT_MOCK_H
#define STORAGE_DAEMON_FSCRYPT_KEY_V1_EXT_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "fscrypt_key_v1_ext.h"

namespace OHOS {
namespace StorageDaemon {
class IFscryptKeyV1Ext {
public:
    virtual ~IFscryptKeyV1Ext() = default;

    virtual int32_t ActiveKeyExt(uint32_t, uint8_t*, uint32_t, uint32_t &) = 0;
    virtual int32_t ActiveDoubleKeyExt(uint32_t, uint8_t*, uint32_t, uint32_t &) = 0;
    virtual int32_t InactiveKeyExt(uint32_t) = 0;
    virtual int32_t LockUserScreenExt(uint32_t, uint32_t &) = 0;
    virtual int32_t UnlockUserScreenExt(uint32_t, uint8_t*, uint32_t) = 0;
    virtual int32_t AddClassE(bool &, bool &, uint32_t) = 0;
    virtual int32_t DeleteClassEPinCode(uint32_t) = 0;
    virtual int32_t ChangePinCodeClassE(uint32_t, bool &) = 0;
    virtual int32_t ReadClassE(uint32_t, std::unique_ptr<uint8_t[]>&, uint32_t, bool&) = 0;
    virtual int32_t WriteClassE(uint32_t, uint8_t*, uint32_t) = 0;
    virtual int32_t GenerateAppkey(uint32_t, uint32_t, std::unique_ptr<uint8_t[]>&, uint32_t) = 0;
    virtual int32_t LockUeceExt(bool&) = 0;
public:
    static inline std::shared_ptr<IFscryptKeyV1Ext> fscryptKeyV1ExtMock = nullptr;
};

class FscryptKeyV1ExtMock : public IFscryptKeyV1Ext {
public:
    MOCK_METHOD(int32_t, ActiveKeyExt, (uint32_t, uint8_t*, uint32_t, uint32_t &));
    MOCK_METHOD(int32_t, ActiveDoubleKeyExt, (uint32_t, uint8_t*, uint32_t, uint32_t &));
    MOCK_METHOD(int32_t, InactiveKeyExt, (uint32_t));
    MOCK_METHOD(int32_t, LockUserScreenExt, (uint32_t, uint32_t &));
    MOCK_METHOD(int32_t, UnlockUserScreenExt, (uint32_t, uint8_t*, uint32_t));
    MOCK_METHOD(int32_t, AddClassE, (bool &, bool &, uint32_t));
    MOCK_METHOD(int32_t, DeleteClassEPinCode, (uint32_t));
    MOCK_METHOD(int32_t, ChangePinCodeClassE, (uint32_t, bool &));
    MOCK_METHOD(int32_t, ReadClassE, (uint32_t, (std::unique_ptr<uint8_t[]>&), uint32_t, bool&));
    MOCK_METHOD(int32_t, WriteClassE, (uint32_t, uint8_t*, uint32_t));
    MOCK_METHOD(int32_t, GenerateAppkey, (uint32_t, uint32_t, (std::unique_ptr<uint8_t[]>&), uint32_t));
    MOCK_METHOD(int32_t, LockUeceExt, (bool&));
};
}
}
#endif