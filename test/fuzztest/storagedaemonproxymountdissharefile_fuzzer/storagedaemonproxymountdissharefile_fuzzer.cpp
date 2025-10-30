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
#include "storagedaemonproxymountdissharefile_fuzzer.h"

#include <vector>
#include <map>

#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "storage_daemon_proxy.h"
#include "ipc/storage_daemon_provider.h"

namespace OHOS {
using namespace std;
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

void StorageDaemonProxyFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    MountDisShareFileFuzzTest(proxy, data, size);
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
