/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <sys/statvfs.h>
#include "storage/storage_total_status_service.h"
#include "utils/storage_manager_errno.h"

using namespace std;

namespace OHOS {
namespace StorageManager {
StorageTotalStatusService::StorageTotalStatusService() {}
StorageTotalStatusService::~StorageTotalStatusService(){}

int64_t StorageTotalStatusService::GetFreeSizeOfVolume(string volumeUuid) {
    struct statvfs diskInfo;
    int ret = statvfs("/", &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    int64_t freeSize = diskInfo.f_bsize * diskInfo.f_bfree;
    return freeSize;
}

int64_t StorageTotalStatusService::GetTotalSizeOfVolume(string volumeUuid) {
    struct statvfs diskInfo;
    int ret = statvfs("/", &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    int64_t totalSize = diskInfo.f_bsize * diskInfo.f_blocks;
    return totalSize;
}
} // StorageManager
} // OHOS