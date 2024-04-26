/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storagedaemon_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "storage_daemon_ipc_interface_code.h"
#include "storage_daemon_stub.h"
#include "storage_daemon.h"
#include "securec.h"
#include "user_manager.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr uint8_t MAX_CALL_TRANSACTION = 32;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t FOO_MAX_LEN = 1024;

std::shared_ptr<StorageDaemon::StorageDaemon> storageDaemon =
    std::make_shared<StorageDaemon::StorageDaemon>();
std::shared_ptr<StorageDaemon::UserManager> userManager =
        StorageDaemon::UserManager::GetInstance();
uint32_t GetU32Data(const char* ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool StorageDaemonFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    uint32_t code = GetU32Data(data.get());
    if (code == 0
        || code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE)
        || code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE)) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageDaemon->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool UserManagerFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }

    int32_t userId = *(reinterpret_cast<const int32_t *>(data));
    uint32_t flag = *(reinterpret_cast<const uint32_t *>(data));
    userManager->PrepareUserDirs(userId, flag);
    userManager->DestroyUserDirs(userId, flag);
    userManager->StartUser(userId);
    userManager->StopUser(userId);
    userManager->CreateBundleDataDir(flag);

    return true;
    }
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }
    OHOS::StorageDaemonFuzzTest(move(str), size);
    OHOS::UserManagerFuzzTest(data, size);

    return 0;
}
