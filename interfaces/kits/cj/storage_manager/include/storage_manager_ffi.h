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

#ifndef STORAGE_MANAGER_FFI_H
#define STORAGE_MANAGER_FFI_H

#include <cstdint>

namespace OHOS {
namespace CJStorageManager {

struct NativeBundleStats {
    int64_t appSize;
    int64_t cacheSize;
    int64_t dataSize;
};

} // namespace CJStorageManager
} // namespace OHOS

#endif // STORAGE_MANAGER_FFI_H