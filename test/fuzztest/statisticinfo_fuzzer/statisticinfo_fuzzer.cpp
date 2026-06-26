/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "statisticinfo_fuzzer.h"
#include "statistic_info.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <cstddef>
#include <cstdint>
#include <memory>

namespace OHOS {
constexpr size_t MAX_STRING_LEN = 256;

void FuzzNextDqBlk(FuzzedDataProvider &fdp)
{
    Parcel parcel;
    StorageManager::NextDqBlk nextDqBlk;
    nextDqBlk.dqbHardLimit = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbBSoftLimit = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbCurSpace = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbIHardLimit = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbISoftLimit = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbCurInodes = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbBTime = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbITime = fdp.ConsumeIntegral<uint64_t>();
    nextDqBlk.dqbValid = fdp.ConsumeIntegral<uint32_t>();
    nextDqBlk.dqbId = fdp.ConsumeIntegral<uint32_t>();
    nextDqBlk.Marshalling(parcel);
    auto unmarshallingNextDqBlk =
        std::unique_ptr<StorageManager::NextDqBlk>(nextDqBlk.Unmarshalling(parcel));
}

void FuzzDirSpaceInfo(FuzzedDataProvider &fdp)
{
    Parcel parcel;
    StorageManager::DirSpaceInfo dirSpaceInfo;
    dirSpaceInfo.path = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    dirSpaceInfo.uid = fdp.ConsumeIntegral<uint32_t>();
    dirSpaceInfo.size = fdp.ConsumeIntegral<int64_t>();
    dirSpaceInfo.Marshalling(parcel);
    auto unmarshallingDirSpaceInfo =
        std::unique_ptr<StorageManager::DirSpaceInfo>(dirSpaceInfo.Unmarshalling(parcel));
}

void FuzzUidSaInfo(FuzzedDataProvider &fdp)
{
    Parcel parcel;
    StorageManager::UidSaInfo uidSaInfo;
    uidSaInfo.uid = fdp.ConsumeIntegral<int32_t>();
    uidSaInfo.saName = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    uidSaInfo.size = fdp.ConsumeIntegral<int64_t>();
    uidSaInfo.iNodes = fdp.ConsumeIntegral<uint64_t>();
    uidSaInfo.Marshalling(parcel);
    auto unmarshallingUidSaInfo =
        std::unique_ptr<StorageManager::UidSaInfo>(uidSaInfo.Unmarshalling(parcel));
}

void FuzzLargeFileInfo(FuzzedDataProvider &fdp)
{
    Parcel parcel;
    StorageManager::LargeFileInfo largeFileInfo;
    largeFileInfo.path = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    largeFileInfo.size = fdp.ConsumeIntegral<int64_t>();
    largeFileInfo.Marshalling(parcel);
    auto unmarshallingLargeFileInfo =
        std::unique_ptr<StorageManager::LargeFileInfo>(largeFileInfo.Unmarshalling(parcel));
}

void FuzzLargeDirInfo(FuzzedDataProvider &fdp)
{
    Parcel parcel;
    StorageManager::LargeDirInfo largeDirInfo;
    largeDirInfo.path = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    largeDirInfo.totalSize = fdp.ConsumeIntegral<int64_t>();
    largeDirInfo.Marshalling(parcel);
    auto unmarshallingLargeDirInfo =
        std::unique_ptr<StorageManager::LargeDirInfo>(largeDirInfo.Unmarshalling(parcel));
}

bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr) {
        return true;
    }
    FuzzedDataProvider fdp(data, size);
    FuzzNextDqBlk(fdp);
    FuzzDirSpaceInfo(fdp);
    FuzzUidSaInfo(fdp);
    FuzzLargeFileInfo(fdp);
    FuzzLargeDirInfo(fdp);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::FileUtilFuzzTest(data, size);
    return 0;
}
