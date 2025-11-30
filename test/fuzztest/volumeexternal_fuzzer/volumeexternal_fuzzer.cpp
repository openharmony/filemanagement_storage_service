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
#include "volumeexternal_fuzzer.h"
#include "volume_external.h"
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool FileUtilFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return false;
    }
    Parcel parcel;
    StorageManager::VolumeExternal volumeExternal;
    int32_t fsType = *(reinterpret_cast<const int32_t *>(data));
    std::string metaData(reinterpret_cast<const char *>(data), size);
    volumeExternal.SetFlags(fsType);
    volumeExternal.SetFsType(fsType);
    volumeExternal.SetFsUuid(metaData);
    volumeExternal.SetPath(metaData);
    volumeExternal.SetDescription(metaData);
    volumeExternal.GetFlags();
    volumeExternal.GetFsType();
    volumeExternal.GetUuid();
    volumeExternal.GetPath();
    volumeExternal.GetDescription();
    volumeExternal.Reset();
    volumeExternal.Marshalling(parcel);
    auto unmarshallingVolumeExternal =
        std::unique_ptr<StorageManager::VolumeExternal>(volumeExternal.Unmarshalling(parcel));
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
