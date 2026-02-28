/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_KEY_MANAGER_MOCK_H
#define STORAGE_DAEMON_KEY_MANAGER_MOCK_H

#include <gmock/gmock.h>

#include "crypto/key_manager.h"
#include "storage_service_constant.h"

namespace OHOS {
namespace StorageDaemon {
class IKeyManagerMock {
public:
    virtual ~IKeyManagerMock() = default;
public:
    virtual int RestoreUserKey(uint32_t userId, KeyType type) = 0;
    virtual int32_t GenerateUserKeys(unsigned int user, uint32_t flags) = 0;
    virtual int32_t DeleteUserKeys(unsigned int user) = 0;
    virtual int32_t EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList) = 0;
    virtual int32_t InitGlobalUserKeys() = 0;
    virtual int32_t InitGlobalDeviceKey() = 0;
    virtual int UpdateUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret) = 0;
    virtual int ActiveCeSceSeceUserKey(unsigned int user, KeyType type, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret) = 0;
    virtual int UpdateCeEceSeceUserAuth(unsigned int user, struct UserTokenSecret &userTokenSecret, KeyType type) = 0;
    virtual int UpdateCeEceSeceKeyContext(uint32_t userId, KeyType type) = 0;
    virtual int ActiveElxUserKey4Nato(unsigned int user, KeyType type, const KeyBlob &authToken) = 0;
    virtual std::string GetNatoNeedRestorePath(uint32_t userId, KeyType type) = 0;
    virtual std::string GetKeyDirByUserAndType(unsigned int user, KeyType type) = 0;
    virtual int GenerateUserKeyByType(unsigned int user, KeyType type,
        const std::vector<uint8_t> &token, const std::vector<uint8_t> &secret) = 0;
    virtual int NotifyUeceActivation(uint32_t userId, int32_t resultCode, bool needGetAllAppKey) = 0;
    virtual int32_t InActiveUserKey(unsigned int user) = 0;
    virtual int32_t LockUserScreen(uint32_t user) = 0;
    virtual int32_t UnlockUserScreen(uint32_t user, const std::vector<uint8_t> &token,
        const std::vector<uint8_t> &secret) = 0;
    virtual int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet) = 0;
    virtual int32_t DeleteAppkey(uint32_t userId, const std::string keyId) = 0;
    virtual int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey) = 0;
    virtual int GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount) = 0;
    virtual int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath,
        StorageService::EncryptionLevel level) = 0;
    virtual bool IsDirRecursivelyEmpty(const char* dirPath) = 0;
    virtual bool GetSecureUid(uint32_t userId, uint64_t &secureUid) = 0;
    virtual int UpdateUserAuthByKeyType(unsigned int user,
        struct UserTokenSecret &userTokenSecret, KeyType keyType) = 0;
    virtual int UpdateKeyContextByKeyType(uint32_t userId, KeyType keyType) = 0;

public:
    static inline std::shared_ptr<IKeyManagerMock> iKeyManagerMock_ = nullptr;
};

class KeyManagerMock : public IKeyManagerMock {
public:
    MOCK_METHOD(int, RestoreUserKey, (uint32_t, KeyType));
    MOCK_METHOD(int32_t, GenerateUserKeys, (unsigned int, uint32_t));
    MOCK_METHOD(int32_t, DeleteUserKeys, (unsigned int));
    MOCK_METHOD(int32_t, EraseAllUserEncryptedKeys, (const std::vector<int32_t> &));
    MOCK_METHOD(int32_t, InitGlobalUserKeys, ());
    MOCK_METHOD(int32_t, InitGlobalDeviceKey, ());
    MOCK_METHOD(int, UpdateUserAuth, (unsigned int, struct UserTokenSecret &));
    MOCK_METHOD(int, ActiveCeSceSeceUserKey, (unsigned int, KeyType,
        const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD(int, UpdateCeEceSeceUserAuth, (unsigned int, struct UserTokenSecret &, KeyType));
    MOCK_METHOD(int, UpdateCeEceSeceKeyContext, (unsigned int, KeyType));
    MOCK_METHOD(int, ActiveElxUserKey4Nato, (unsigned int, KeyType, const KeyBlob &));
    MOCK_METHOD(std::string, GetNatoNeedRestorePath, (uint32_t, KeyType));
    MOCK_METHOD(std::string, GetKeyDirByUserAndType, (unsigned int, KeyType));
    MOCK_METHOD(int, GenerateUserKeyByType, (unsigned int, KeyType,
        const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD(int, NotifyUeceActivation, (uint32_t, int32_t, bool));
    MOCK_METHOD(int32_t, InActiveUserKey, (unsigned int));
    MOCK_METHOD(int32_t, LockUserScreen, (uint32_t));
    MOCK_METHOD(int32_t, UnlockUserScreen, (uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD(int32_t, GenerateAppkey, (uint32_t, uint32_t, std::string &, bool));
    MOCK_METHOD(int32_t, DeleteAppkey, (uint32_t, const std::string));
    MOCK_METHOD(int32_t, UpdateKeyContext, (uint32_t, bool));
    MOCK_METHOD(int, GetFileEncryptStatus, (uint32_t, bool &, bool));
    MOCK_METHOD(int32_t, SetDirEncryptionPolicy, (uint32_t, const std::string &, StorageService::EncryptionLevel));
    MOCK_METHOD(bool, IsDirRecursivelyEmpty, (const char *));
    MOCK_METHOD(bool, GetSecureUid, (uint32_t, uint64_t &));
    MOCK_METHOD(int, UpdateUserAuthByKeyType, (unsigned int, struct UserTokenSecret &, KeyType));
    MOCK_METHOD(int, UpdateKeyContextByKeyType, (uint32_t, KeyType));
};
}
}
#endif // STORAGE_DAEMON_KEY_MANAGER_MOCK_H
