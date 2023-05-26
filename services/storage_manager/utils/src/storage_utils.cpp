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

#include "utils/storage_utils.h"

namespace OHOS {
namespace StorageManager {
int64_t GetRoundSize(int64_t size)
{
    uint64_t val = 1;
    int64_t multple = UNIT;
    int64_t stdMultiple = STD_UNIT;
    while (val * stdMultiple < size) {
        val <<= 1;
        if (val > THRESHOLD) {
            val = 1;
            multple *= UNIT;
            stdMultiple *= STD_UNIT;
        }
    }
    return val * multple;
}
} // namespace STORAGE_Manager
} // namespace OHOS