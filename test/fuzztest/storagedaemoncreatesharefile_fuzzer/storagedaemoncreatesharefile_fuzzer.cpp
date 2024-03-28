/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "storagedaemoncreatesharefile_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "message_parcel.h"
#include "mount_argument_utils.h"
#include "storage_daemon_ipc_interface_code.h"
#include "storage_daemon_stub.h"
#include "storage_daemon.h"
#include "securec.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {
constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

std::shared_ptr<StorageDaemon::StorageDaemon> storageDaemon =
    std::make_shared<StorageDaemon::StorageDaemon>();

uint32_t GetU32Data(const char* ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool StorageDaemonCreateShareFileFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    uint32_t code = static_cast<int32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemonStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageDaemon->OnRemoteRequest(code, datas, reply, option);

    return true;
}

bool MountArgumentUtilsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return false;
    }
    struct Utils::MountArgument argument = {};
    argument.GetFullSrc();
    argument.GetFullDst();
    argument.GetShareSrc();
    argument.GetShareDst();
    argument.GetUserIdPara();
    argument.GetCommFullPath();
    argument.GetCloudFullPath();
    argument.GetCachePath();
    argument.GetCtrlPath();
    argument.OptionsToString();
    argument.GetFullCloud();
    argument.GetFullMediaCloud();
    argument.GetCloudDocsPath();
    argument.GetLocalDocsPath();
    argument.GetFlags();
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
    OHOS::StorageDaemonCreateShareFileFuzzTest(move(str), size);
    OHOS::MountArgumentUtilsFuzzTest(move(str), size);

    return 0;
}
