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
#ifndef STORAGE_DAEMON_FSCRYT_KEY_V2_MOCK_H
#define STORAGE_DAEMON_FSCRYT_KEY_V2_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "fscrypt_key_v2.h"

namespace OHOS {
namespace StorageDaemon {
class IFscryptKeyV2Moc {
public:
    virtual ~IFscryptKeyV2Moc() = default;
public:
    virtual bool ActiveKey(uint32_t flag, const std::string &mnt) = 0;
    virtual int32_t GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId) = 0;
    virtual int32_t DeleteAppkey(const std::string KeyId) = 0;
    virtual bool InactiveKey(uint32_t flag, const std::string &mnt) = 0;
    virtual bool LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt) = 0;
    virtual bool LockUece(bool &isFbeSupport) = 0;
    virtual bool UnlockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt) = 0;
    virtual int32_t AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status) = 0;
    virtual int32_t DeleteClassEPinCode(uint32_t user) = 0;
    virtual int32_t ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId) = 0;
    virtual bool DecryptClassE(const UserAuth &auth, bool &isSupport,
                               bool &eBufferStatue, uint32_t user, uint32_t status) = 0;
    virtual bool EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status) = 0;
public:
    static inline std::shared_ptr<IFscryptKeyV2Moc> fscryptKeyV2Moc = nullptr;
};

class FscryptKeyV2Moc : public IFscryptKeyV2Moc {
public:
    MOCK_METHOD2(ActiveKey, bool(uint32_t flag, const std::string &mnt));
    MOCK_METHOD3(GenerateAppkey, bool(uint32_t userId, uint32_t appUid, std::string &keyId));
    MOCK_METHOD1(DeleteAppkey, bool(const std::string KeyId));
    MOCK_METHOD2(InactiveKey, bool(uint32_t flag, const std::string &mnt));
    MOCK_METHOD3(LockUserScreen, bool(uint32_t flag, uint32_t sdpClass, const std::string &mnt));
    MOCK_METHOD1(LockUece, bool(bool &isFbeSupport));
    MOCK_METHOD3(UnlockUserScreen, bool(uint32_t flag, uint32_t sdpClass, const std::string &mnt));
    MOCK_METHOD3(AddClassE, bool(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status));
    MOCK_METHOD1(DeleteClassEPinCode, bool(uint32_t user));
    MOCK_METHOD2(ChangePinCodeClassE, bool(bool &isFbeSupport, uint32_t userId));
    MOCK_METHOD5(DecryptClassE, bool(const UserAuth &auth, bool &isSupport,
                                     bool &eBufferStatue, uint32_t user, uint32_t status));
    MOCK_METHOD4(EncryptClassE, bool(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status));
};
}
}
#endif