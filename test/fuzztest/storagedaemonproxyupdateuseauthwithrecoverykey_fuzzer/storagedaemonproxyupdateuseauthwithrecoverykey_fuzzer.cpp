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
#include "storagedaemonproxyupdateuseauthwithrecoverykey_fuzzer.h"

#include <vector>
#include <map>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "securec.h"
#include "storage_daemon_proxy.h"
#include "ipc/storage_daemon_provider.h"

namespace OHOS {
using namespace std;
template<typename T>
T TypeCast(const uint8_t *data, int *pos)
{
    T value{};
    if (data == nullptr) {
        printf("data is nullptr\n");
        return value;
    }
    auto ret = memcpy_s(&value, sizeof(T), data, sizeof(T));
    if (ret != 0) {
        printf("memcpy_s failed, ret: %d\n", ret);
        return value;
    }
    if (pos) {
        *pos += sizeof(T);
    }
    return value;
}

sptr<StorageDaemon::IStorageDaemon> GetStorageDaemonProxy()
{
    auto samgr = OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        printf("samgr empty error\n");
        return nullptr;
    }

    sptr<IRemoteObject> object = samgr->GetSystemAbility(OHOS::STORAGE_MANAGER_DAEMON_ID);
    if (object == nullptr) {
        printf("storage daemon client samgr ablity empty error\n");
        return nullptr;
    }

    return iface_cast<StorageDaemon::IStorageDaemon>(object);
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
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    auto proxy = OHOS::GetStorageDaemonProxy();
    if (proxy != nullptr) {
        OHOS::UpdateUseAuthWithRecoveryKeyFuzzTest(proxy, data, size);
    } else {
        printf("daemon proxy is nullptr\n");
    }

    return 0;
}
