/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILE_KEYMANAGER_IMPL_H
#define OHOS_FILE_KEYMANAGER_IMPL_H
#include "ohos.file.storageStatistics.impl.hpp"
#include "ohos.file.storageStatistics.proj.hpp"
#include "bundle_stats.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"
#include "storage_stats.h"
#include "taihe/runtime.hpp"

namespace ANI::StorageStatistics {
int64_t GetFreeSizeSync();
int64_t GetTotalSizeSync();

ohos::file::storageStatistics::BundleStats GetCurrentBundleStatsSync();
ohos::file::storageStatistics::StorageStats GetUserStorageStatsSync();
ohos::file::storageStatistics::StorageStats GetUserStorageStatsByidSync(int64_t userId);
}
#endif // OHOS_FILE_KEYMANAGER_IMPL_H
