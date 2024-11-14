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
#ifndef STORAGE_DAEMON_BASE_KEY_MOCK_H
#define STORAGE_DAEMON_BASE_KEY_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "key_blob.h"

namespace OHOS {
namespace StorageDaemon {
class IBaseKeyMoc {
public:
    virtual ~IBaseKeyMoc() = default;
public:
    virtual bool InitKey(bool needGenerateKey) = 0;
    virtual bool StoreKey(const UserAuth &auth) = 0;
    virtual bool StoreKey(const UserAuth &auth, bool needGenerateShield) = 0;
    virtual bool ClearKey(const std::string &mnt) = 0;
    virtual bool UpdateKey(const std::string &keypath) = 0;
    virtual bool RestoreKey(const UserAuth &auth) = 0;
    virtual bool UpgradeKeys() = 0;
    virtual bool DecryptKeyBlob(const UserAuth &, const std::string &, KeyBlob &, KeyBlob &) = 0;
    virtual bool EncryptKeyBlob(const UserAuth &, const std::string &, KeyBlob &, KeyBlob &) = 0;
    virtual bool RenameKeyPath(const std::string &keyPath) = 0;
    virtual bool GetOriginKey(KeyBlob &originKey) = 0;
    virtual bool SaveKeyBlob(const KeyBlob &blob, const std::string &path) = 0;
    virtual bool LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size) = 0;
    virtual bool KeyDescIsEmpty() = 0;
    virtual std::string GetKeyDir() = 0;
public:
    static inline std::shared_ptr<IBaseKeyMoc> baseKeyMoc = nullptr;
};

class BaseKeyMoc : public IBaseKeyMoc {
public:
    MOCK_METHOD1(InitKey, bool(bool needGenerateKey));
    MOCK_METHOD2(StoreKey, bool(const UserAuth &auth, bool needGenerateShield));
    MOCK_METHOD1(StoreKey, bool(const UserAuth &auth));
    MOCK_METHOD1(ClearKey, bool(const std::string &mnt));
    MOCK_METHOD1(UpdateKey, bool(const std::string &keypath));
    MOCK_METHOD1(RestoreKey, bool(const UserAuth &auth));
    MOCK_METHOD0(UpgradeKeys, bool());
    MOCK_METHOD0(KeyDescIsEmpty, bool());
    MOCK_METHOD0(GetKeyDir, std::string());
    MOCK_METHOD4(DecryptKeyBlob, bool(const UserAuth &, const std::string &, KeyBlob &, KeyBlob &));
    MOCK_METHOD4(EncryptKeyBlob, bool(const UserAuth &, const std::string &, KeyBlob &, KeyBlob &));
    MOCK_METHOD1(RenameKeyPath, bool(const std::string &keypath));
    MOCK_METHOD1(GetOriginKey, bool(KeyBlob &originKey));
    MOCK_METHOD2(SaveKeyBlob, bool(const KeyBlob &blob, const std::string &path));
    MOCK_METHOD3(LoadKeyBlob, bool(KeyBlob &blob, const std::string &path, const uint32_t size));
};
}
}
#endif