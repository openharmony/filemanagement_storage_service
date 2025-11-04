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
#include "storagedaemonproxycreatesharefile_fuzzer.h"

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

bool CreateShareFileFuzzTest(sptr<StorageDaemon::IStorageDaemon>& proxy, const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }

    int pos = 0;
    uint32_t tokenId = TypeCast<uint32_t>(data, &pos);
    uint32_t flag = TypeCast<uint32_t>(data + pos, &pos);
    if (pos != sizeof(uint32_t) + sizeof(uint32_t)) {
        return true;
    }
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
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    auto proxy = OHOS::GetStorageDaemonProxy();
    if (proxy != nullptr) {
        OHOS::CreateShareFileFuzzTest(proxy, data, size);
    } else {
        printf("daemon proxy is nullptr\n");
    }

    return 0;
}
