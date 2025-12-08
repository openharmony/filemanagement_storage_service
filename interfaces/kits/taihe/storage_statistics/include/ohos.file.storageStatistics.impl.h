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
int64_t GetFreeSizeOfVolumeSync(::taihe::string_view volumeUuid);
int64_t GetSystemSizeSync();
int64_t GetTotalSizeOfVolumeSync(::taihe::string_view volumeUuid);
int64_t GetFreeSizeAsync();
int64_t GetTotalSizeAsync();

ohos::file::storageStatistics::BundleStats GetCurrentBundleStatsSync();
ohos::file::storageStatistics::StorageStats GetUserStorageStatsSync();
ohos::file::storageStatistics::StorageStats GetUserStorageStatsByidSync(int64_t userId);
void SetExtBundleStatsSync(int32_t userId, ohos::file::storageStatistics::ExtBundleStats stats);
ohos::file::storageStatistics::ExtBundleStats GetExtBundleStatsSync(int32_t userId, taihe::string_view businessName);
taihe::array<ohos::file::storageStatistics::ExtBundleStats> GetAllExtBundleStatsSync(int32_t userId);
taihe::array<ohos::file::storageStatistics::UserdataDirInfo> ListUserdataDirInfoSync();
::ohos::file::storageStatistics::BundleStats GetBundleStatsSync(::taihe::string_view packageName,
    ::taihe::optional_view<int32_t> index);
}
#endif // OHOS_FILE_KEYMANAGER_IMPL_H
