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
#include <libmtp.h>
#include "libmtpcheckspecificdevice_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace OHOS {
constexpr size_t MIN_STORAGE_SIZE = sizeof(uint16_t) * 3 + sizeof(uint32_t) + sizeof(uint64_t) * 3 + sizeof(char *) * 2;
constexpr size_t MIN_EXTENSION_SIZE = sizeof(char *) + sizeof(int) * 2;
constexpr size_t MIN_MTPDEVICE_SIZE = sizeof(uint8_t) * 2 + sizeof(uint32_t) * 8 + sizeof(int);
constexpr size_t MIN_SIZE = std::max({MIN_STORAGE_SIZE, MIN_EXTENSION_SIZE, MIN_MTPDEVICE_SIZE});
constexpr size_t MIN_RAWDEVICE_SIZE = sizeof(uint32_t) * 2 + sizeof(uint16_t) * 2 +
                                      sizeof(uint8_t) + sizeof(int) + sizeof(char *) * 2;
constexpr size_t MIN_FOLDER_SIZE = sizeof(uint32_t) * 3 + sizeof(char *);
constexpr size_t MIN_FILE_SIZE = sizeof(uint32_t) * 3 + sizeof(uint64_t) + sizeof(char *) +
                                 sizeof(time_t) + sizeof(LIBMTP_filetype_t);

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

bool CheckSpecificDeviceFuzzTest(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    if (data == nullptr || size <= sizeof(int) * numPara32) {
        return false;
    }

    int pos = 0;
    int busno = TypeCast<int>(data, &pos);
    int devno = TypeCast<int>(data + pos);

    LIBMTP_Check_Specific_Device(busno, devno);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::CheckSpecificDeviceFuzzTest(data, size);
    return 0;
}