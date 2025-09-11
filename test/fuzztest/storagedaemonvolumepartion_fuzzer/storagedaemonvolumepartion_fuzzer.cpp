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
#include "storagedaemonvolumepartion_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "accesstoken_kit.h"
#include "message_parcel.h"
#include "securec.h"
#include "storage_daemon.h"
#include "storage_daemon_stub.h"
#include "user_manager.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr uint8_t MAX_CALL_TRANSACTION = 32;

std::shared_ptr<StorageDaemon::StorageDaemon> storageDaemon = std::make_shared<StorageDaemon::StorageDaemon>();
StorageDaemon::UserManager &userManager = StorageDaemon::UserManager::GetInstance();
uint32_t GetU32Data(const uint8_t *ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool StorageDaemonFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return true;
    }

    uint32_t code = GetU32Data(data);
    if (code == 0 ||
        code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE) ||
        code % MAX_CALL_TRANSACTION == static_cast<int32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE)) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageDaemon->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool HandlePartitionFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageDaemon->HandlePartition(datas, reply);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::StorageDaemonFuzzTest(data, size);
    OHOS::HandlePartitionFuzzTest(data, size);
    return 0;
}
