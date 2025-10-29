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
#include "storagedaemonproxy_fuzzer.h"

#include <vector>
#include <map>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "storage_daemon_proxy.h"
#include "ipc/storage_daemon_provider.h"

namespace OHOS {
using namespace std;
constexpr int PARAM_COUNT = 5;
template<typename T>
T TypeCast(const uint8_t *data, int *pos)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

sptr<StorageDaemon::IStorageDaemon> GetStorageDaemonProxy()
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        printf("samgr empty error");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_DAEMON_ID);
    if (object == nullptr) {
        printf("storage daemon client samgr ablity empty error");
        return nullptr;
    }

    return iface_cast<StorageDaemon::IStorageDaemon>(object);
}

bool QueryOccupiedSpaceForSaFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int64_t) * PARAM_COUNT) {
        return true;
    }
    string storageStats(reinterpret_cast<const char *>(data), size);
    int pos = 0;
    int len = (size - pos) / 2;
    int32_t key = TypeCast<int32_t>(data + pos, &pos);
    string value(reinterpret_cast<const char *>(data + pos + len), len);
    map<int32_t, std::string> bundleNameAndUid {{key, value}};
    proxy->QueryOccupiedSpaceForSa(storageStats, bundleNameAndUid);
    return true;
}

bool DeleteUserKeysFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    uint32_t userId = TypeCast<uint32_t>(data, nullptr);
    proxy->DeleteUserKeys(userId);
    return true;
}

bool UpdateUserAuthFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint64_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);
    int len = (size - pos) / 3;
    vector<uint8_t> token;
    vector<uint8_t> oldSecret;
    vector<uint8_t> newSecret;
    for (int i = 0; i < len; i++) {
        token.emplace_back(data[pos + i]);
        oldSecret.emplace_back(data[pos + len + i]);
        newSecret.emplace_back(data[pos + len + len + i]);
    }
    proxy->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    return true;
}

bool UpdateUseAuthWithRecoveryKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint64_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos, &pos);
    int len = (size - pos) / 3;
    vector<uint8_t> authToken;
    vector<uint8_t> newSecret;
    vector<uint8_t> plain;
    for (int i = 0; i < len; i++) {
        authToken.emplace_back(data[pos + i]);
        newSecret.emplace_back(data[pos + len + i]);
        plain.emplace_back(data[pos + len + len + i]);
    }
    std::vector<std::vector<uint8_t>> plainText { plain };
    proxy->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
    return true;
}

bool ActiveUserKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    int len = (size - pos) / 2;
    vector<uint8_t> token;
    vector<uint8_t> secret;
    for (int i = 0; i < len; i++) {
        token.emplace_back(data[pos + i]);
        secret.emplace_back(data[pos + len + i]);
    }
    proxy->ActiveUserKey(userId, token, secret);
    proxy->InactiveUserKey(userId);
    proxy->MountCryptoPathAgain(userId);
    proxy->LockUserScreen(userId);
    return true;
}

bool UpdateKeyContextFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(bool)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    bool needRemoveTmpKey = TypeCast<bool>(data + pos, &pos);
    proxy->UpdateKeyContext(userId, needRemoveTmpKey);
    return true;
}

bool UnlockUserScreenFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    int len = (size - pos) / 2;
    vector<uint8_t> token;
    vector<uint8_t> secret;
    for (int i = 0; i < len; i++) {
        token.emplace_back(data[pos + i]);
        secret.emplace_back(data[pos + len + i]);
    }
    proxy->UnlockUserScreen(userId, token, secret);
    return true;
}

bool GetLockScreenStatusFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(bool)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    bool lockScreenStatus = TypeCast<bool>(data + pos, &pos);
    proxy->GetLockScreenStatus(userId, lockScreenStatus);
    return true;
}

bool GenerateAppkeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t) + sizeof(bool)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t hashId = TypeCast<uint32_t>(data + pos, &pos);
    bool needReSet = TypeCast<bool>(data + pos, &pos);
    string keyId(reinterpret_cast<const char *>(data + pos), size - pos);
    proxy->GenerateAppkey(userId, hashId, keyId, needReSet);
    proxy->DeleteAppkey(userId, keyId);
    return true;
}

bool CreateRecoverKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t userType = TypeCast<uint32_t>(data + pos, &pos);
    int len = (size - pos) / 2;
    vector<uint8_t> token;
    vector<uint8_t> secret;
    for (int i = 0; i < len; i++) {
        token.emplace_back(data[pos + i]);
        secret.emplace_back(data[pos + len + i]);
    }
    proxy->CreateRecoverKey(userId, userType, token, secret);
    return true;
}

bool SetRecoverKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return true;
    }

    vector<uint8_t> key;
    for (size_t i = 0; i < size; i++) {
        key.emplace_back(data[i]);
    }
    proxy->SetRecoverKey(key);
    return true;
}

bool CreateShareFileFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t tokenId = TypeCast<uint32_t>(data, &pos);
    uint32_t flag = TypeCast<uint32_t>(data + pos, &pos);
    unsigned int len = (size - pos) / 2;
    vector<int32_t> funcResult;
    StorageManager::StorageFileRawData uriList;
    uriList.ownedData = string(reinterpret_cast<const char *>(data + pos + len), len);
    for (unsigned int i = 0; i + sizeof(int32_t) <= len; i += sizeof(int32_t)) {
        funcResult.emplace_back(TypeCast<int32_t>(data + pos + i, nullptr));
    }
    if (proxy->CreateShareFile(uriList, tokenId, flag, funcResult) == 0) {
        proxy->DeleteShareFile(tokenId, uriList);
    }
    return true;
}

bool SetBundleQuotaFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return true;
    }

    int pos = 0;
    int32_t uid = TypeCast<int32_t>(data, &pos);
    int32_t limitSizeMb = TypeCast<int32_t>(data + pos, &pos);
    int len = (size - pos) / 2;
    string bundleName(reinterpret_cast<const char *>(data + pos), len);
    string bundleDataDirPath(reinterpret_cast<const char *>(data + pos + len), len);
    proxy->SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    return true;
}

[[maybe_unused]]bool GetOccupiedSpaceFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data,
    size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return true;
    }

    int pos = 0;
    int32_t idType = TypeCast<int32_t>(data, &pos);
    int32_t id = TypeCast<int32_t>(data + pos, &pos);
    int64_t sz = 0;
    proxy->GetOccupiedSpace(idType, id, sz);
    return true;
}

bool UpdateMemoryParaFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(int32_t)) {
        return true;
    }

    int pos = 0;
    int32_t sz = TypeCast<int32_t>(data, &pos);
    int32_t oldSize = TypeCast<uint32_t>(data + pos, &pos);
    proxy->UpdateMemoryPara(sz, oldSize);
    return true;
}

bool MountDfsDocsFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return true;
    }

    int pos = 0;
    int32_t userId = TypeCast<int32_t>(data, &pos);
    int len = (size - pos) / 3;
    string relativePath(reinterpret_cast<const char *>(data + pos), len);
    string networkId(reinterpret_cast<const char *>(data + pos + len), len);
    string deviceId(reinterpret_cast<const char *>(data + pos + len + len), len);
    if (proxy->MountDfsDocs(userId, relativePath, networkId, deviceId) == 0) {
        proxy->UMountDfsDocs(userId, relativePath, networkId, deviceId);
    }
    return true;
}

bool GetFileEncryptStatusFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t) + sizeof(bool) + sizeof(bool)) {
        return true;
    }

    int pos = 0;
    int32_t userId = TypeCast<int32_t>(data, &pos);
    bool isEncrypted = TypeCast<bool>(data + pos, &pos);
    bool needCheckDirMount = TypeCast<bool>(data + pos, &pos);
    proxy->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    return true;
}

bool MountMediaFuseFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int32_t)) {
        return true;
    }

    int32_t userId = TypeCast<int32_t>(data, nullptr);
    int devFd = 0;
    proxy->MountMediaFuse(userId, devFd);
    proxy->UMountMediaFuse(userId);
    return true;
}

bool GetUserNeedActiveStatusFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(bool)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    bool needActive = TypeCast<bool>(data + pos, &pos);
    proxy->GetUserNeedActiveStatus(userId, needActive);
    return true;
}

bool MountFileMgrFuseFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    string path(reinterpret_cast<const char *>(data + pos), size - pos);
    int fuseFd = 0;
    if (proxy->MountFileMgrFuse(userId, path, fuseFd) == 0) {
        proxy->UMountFileMgrFuse(userId, path);
    }
    return true;
}

bool QueryUsbIsInUseFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    bool isInUse = false;
    string diskPath(reinterpret_cast<const char *>(data), size);
    proxy->QueryUsbIsInUse(diskPath, isInUse);
    return true;
}

bool ResetSecretWithRecoveryKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint32_t rkType = TypeCast<uint32_t>(data + pos, &pos);
    int len = (size - pos);
    vector<uint8_t> key;
    for (int i = 0; i < len; i++) {
        key.emplace_back(data[i + pos]);
    }
    proxy->ResetSecretWithRecoveryKey(userId, rkType, key);
    return true;
}

bool IsFileOccupiedFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    int len = size / 2;
    string path(reinterpret_cast<const char *>(data), len);
    vector<std::string> inputList { string(reinterpret_cast<const char *>(data + len), len) };
    vector<std::string> outputList;
    bool isOccupy;
    proxy->IsFileOccupied(path, inputList, outputList, isOccupy);
    return true;
}

bool MountDisShareFileFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(int)) {
        return true;
    }

    int pos = 0;
    int userId = TypeCast<int>(data, &pos);
    int len = (size - pos) / 2;
    string key(reinterpret_cast<const char *>(data + pos), len);
    string value(reinterpret_cast<const char *>(data + pos + len), len);
    map<string, string> shareFiles {{key, value}};
    proxy->MountDisShareFile(userId, shareFiles);
    return true;
}

[[maybe_unused]]bool UMountDisShareFileFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data,
    size_t size)
{
    if (data == nullptr || size < sizeof(int)) {
        return true;
    }

    int pos = 0;
    int userId = TypeCast<int>(data, &pos);
    int len = (size - pos);
    string networkId(reinterpret_cast<const char *>(data + pos), len);
    proxy->UMountDisShareFile(userId, networkId);
    return true;
}

bool TryToFixFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t flags = TypeCast<uint32_t>(data, &pos);
    int len = (size - pos);
    string volId(reinterpret_cast<const char *>(data + pos), len);
    proxy->TryToFix(volId, flags);
    return true;
}

bool InactiveUserPublicDirKeyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    uint32_t userId = TypeCast<uint32_t>(data, nullptr);
    proxy->InactiveUserPublicDirKey(userId);
    return true;
}

void StorageDaemonProxyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    DeleteUserKeysFuzzTest(proxy, data, size);
    UpdateUserAuthFuzzTest(proxy, data, size);
    UpdateUseAuthWithRecoveryKeyFuzzTest(proxy, data, size);
    ActiveUserKeyFuzzTest(proxy, data, size);
    UpdateKeyContextFuzzTest(proxy, data, size);
    UnlockUserScreenFuzzTest(proxy, data, size);
    GenerateAppkeyFuzzTest(proxy, data, size);
    CreateRecoverKeyFuzzTest(proxy, data, size);
    SetRecoverKeyFuzzTest(proxy, data, size);
    CreateShareFileFuzzTest(proxy, data, size);
    SetBundleQuotaFuzzTest(proxy, data, size);
    UpdateMemoryParaFuzzTest(proxy, data, size);
    MountDfsDocsFuzzTest(proxy, data, size);
    GetFileEncryptStatusFuzzTest(proxy, data, size);
    MountMediaFuseFuzzTest(proxy, data, size);
    GetUserNeedActiveStatusFuzzTest(proxy, data, size);
    MountFileMgrFuseFuzzTest(proxy, data, size);
    QueryUsbIsInUseFuzzTest(proxy, data, size);
    ResetSecretWithRecoveryKeyFuzzTest(proxy, data, size);
    IsFileOccupiedFuzzTest(proxy, data, size);
    MountDisShareFileFuzzTest(proxy, data, size);
    TryToFixFuzzTest(proxy, data, size);
    InactiveUserPublicDirKeyFuzzTest(proxy, data, size);
    QueryOccupiedSpaceForSaFuzzTest(proxy, data, size);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    auto proxy = OHOS::GetStorageDaemonProxy();
    if (proxy != nullptr) {
        OHOS::StorageDaemonProxyFuzzTest(proxy, data, size);
    } else {
        printf("daemon proxy is nullptr");
    }

    return 0;
}
