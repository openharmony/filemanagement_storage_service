/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "filesystemcryptoupdateuserauth_fuzzer.h"
#include "crypto/filesystem_crypto.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "vector"

namespace OHOS {
namespace StorageManager {

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

auto &fileSystem = FileSystemCrypto::GetInstance();

bool UpdateUserAuthFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= sizeof(uint32_t) + sizeof(uint64_t)) {
        return false;
    }

    int pos = 0;
    uint32_t userId = TypeCast<uint32_t>(data, &pos);
    uint64_t secureUid = TypeCast<uint64_t>(data + pos);

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    token.push_back(*data);
    oldSecret.push_back(*data);
    newSecret.push_back(*data);

    int32_t result = fileSystem.UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    if (result != E_OK) {
        LOGI("file system crypto fuzz test of interface FileSystemCrypto::UpdateUserAuth failed!");
        return false;
    }
    return true;
}
} // namespace StorageManager
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::StorageManager::UpdateUserAuthFuzzTest(data, size);
    return 0;
}
