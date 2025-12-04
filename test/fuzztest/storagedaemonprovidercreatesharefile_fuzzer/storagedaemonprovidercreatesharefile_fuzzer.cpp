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
#include "storagedaemonprovidercreatesharefile_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sstream>

#include "message_parcel.h"
#include "storage_daemon_stub.h"
#include "storage_daemon.h"
#include "storage_daemon_provider.h"
#include "securec.h"
#include "userdata_dir_info.h"
#include "fuzzer/FuzzedDataProvider.h"

using namespace OHOS::StorageDaemon;

namespace OHOS {

std::shared_ptr<StorageDaemon::StorageDaemonProvider> storageDaemonProvider =
    std::make_shared<StorageDaemon::StorageDaemonProvider>();

uint32_t GetU32Data(const char* ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

void StringVecToRawData(const std::vector<std::string> &stringVec, StorageFileRawData &rawData)
{
    std::stringstream ss;
    uint32_t stringCount = stringVec.size();
    ss.write(reinterpret_cast<const char*>(&stringCount), sizeof(stringCount));

    for (uint32_t i = 0; i < stringCount; ++i) {
        uint32_t strLen = stringVec[i].length();
        ss.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        ss.write(stringVec[i].c_str(), strLen);
    }
    std::string result = ss.str();
    rawData.ownedData = std::move(result);
    rawData.data = rawData.ownedData.data();
    rawData.size = rawData.ownedData.size();
}

bool StorageDaemonProviderCreateShareFileFuzzTest(const uint8_t *data, size_t size)
{
    /* Run your code on data and size */
    if (data == nullptr || size < sizeof(int32_t)) {
        return false;
    }
    uint32_t code = static_cast<uint32_t>(StorageDaemon::IStorageDaemonIpcCode::COMMAND_CREATE_SHARE_FILE);
    FuzzedDataProvider fdp(data, size);
    std::string str = fdp.ConsumeRandomLengthString(10);
    std::vector<std::string> strVec = {str};
    StorageFileRawData rawData;
    StringVecToRawData(strVec, rawData);
    StorageFileRawData fileRawData;
    fileRawData.size = rawData.size;
    fileRawData.RawDataCpy(rawData.data);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageDaemon::StorageDaemonStub::GetDescriptor());
    datas.WriteRawData(fileRawData.data, fileRawData.size);
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    uint32_t flag = fdp.ConsumeIntegral<uint32_t>();
    datas.WriteUint32(tokenId);
    datas.WriteUint32(flag);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageDaemonProvider->OnRemoteRequest(code, datas, reply, option);

    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::StorageDaemonProviderCreateShareFileFuzzTest(data, size);
    return 0;
}
