/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "disk_fuzzer.h"
#include "disk.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    Parcel parcel;
    StorageManager::Disk disk;
    int32_t flag = *(reinterpret_cast<const int32_t *>(data));
    disk.GetDiskId();
    disk.GetSizeBytes();
    disk.GetSysPath();
    disk.GetVendor();
    disk.GetFlag();
    disk.SetFlag(flag);
    disk.Marshalling(parcel);
    auto unmarshallingDisk = std::unique_ptr<OHOS::StorageManager::Disk>(disk.Unmarshalling(parcel));
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
