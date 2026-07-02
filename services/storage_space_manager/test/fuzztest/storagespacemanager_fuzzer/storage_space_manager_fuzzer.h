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

#ifndef STORAGE_SPACE_MANAGER_FUZZER_H
#define STORAGE_SPACE_MANAGER_FUZZER_H

#include <cstdint>
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace StorageSpaceManager {

// Fuzz function declarations
void FuzzGetTotalSize(FuzzedDataProvider& provider);
void FuzzGetSystemSize(FuzzedDataProvider& provider);
void FuzzGetFreeSize(FuzzedDataProvider& provider);
void FuzzGetTotalInodes(FuzzedDataProvider& provider);
void FuzzGetFreeInodes(FuzzedDataProvider& provider);
void FuzzCleanBundleCache(FuzzedDataProvider& provider);
void FuzzInvalidParameters(FuzzedDataProvider& provider);
void FuzzRapidSequentialCalls(FuzzedDataProvider& provider);
void FuzzAlternatingCalls(FuzzedDataProvider& provider);
void FuzzConcurrentAccess(FuzzedDataProvider& provider);
void FuzzBoundaryValues(FuzzedDataProvider& provider);
void FuzzRandomDataPatterns(FuzzedDataProvider& provider);

// Initialize the StorageSpaceManager client
bool InitializeClient();

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // STORAGE_SPACE_MANAGER_FUZZER_H
