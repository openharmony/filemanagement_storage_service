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
#ifndef STORAGE_MANAGER_STORAGE_UTILS_H
#define STORAGE_MANAGER_STORAGE_UTILS_H

#include <iostream>

namespace OHOS {
namespace StorageManager {
static const int64_t UNIT = 1000;
static const int64_t THRESHOLD = 512;
static const int64_t ONE_GB = 1000000000;
int64_t GetRoundSize(int64_t size);
std::string GetAnonyString(const std::string &value);
int32_t IsUserIdValid(int32_t userId);
} // namespace STORAGE_MANAGER
} // namespace OHOS

#endif // STORAGE_MANAGER_STORAGE_UTILS_H