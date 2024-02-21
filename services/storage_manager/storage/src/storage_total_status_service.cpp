/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "storage/storage_total_status_service.h"

#include <cstdlib>
#include <cstring>
#include "hitrace_meter.h"
#include <mntent.h>
#include <singleton.h>
#include <sys/statvfs.h>
#include <unordered_set>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_utils.h"

namespace OHOS {
namespace StorageManager {
StorageTotalStatusService::StorageTotalStatusService() {}
StorageTotalStatusService::~StorageTotalStatusService() {}

int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    int64_t roundSize = 0;
    int32_t ret = GetTotalSize(roundSize);
    if (ret != E_OK) {
        return ret;
    }
    int64_t totalSize = 0;
    ret = GetSizeOfPath(PATH_DATA, SizeType::TOTAL, totalSize);
    if (ret != E_OK) {
        return ret;
    }
    systemSize = roundSize - totalSize;
    return E_OK;
}

int32_t StorageTotalStatusService::GetTotalSize(int64_t &totalSize)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);
    int64_t dataSize = 0;
    int32_t ret = GetSizeOfPath(PATH_DATA, SizeType::TOTAL, dataSize);
    if (ret != E_OK) {
        return ret;
    }
    int64_t rootSize = 0;
    ret = GetSizeOfPath(PATH_ROOT, SizeType::TOTAL, rootSize);
    if (ret != E_OK) {
        return ret;
    }
    totalSize = GetRoundSize(dataSize + rootSize);
    return E_OK;
}

int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    return GetSizeOfPath(PATH_DATA, SizeType::FREE, freeSize);
}

int32_t StorageTotalStatusService::GetSizeOfPath(const char *path, int32_t type, int64_t &size)
{
    struct statvfs diskInfo;
    int ret = statvfs(path, &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    if (type == SizeType::TOTAL) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
    } else if (type == SizeType::FREE) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
    } else {
        size = (int64_t)diskInfo.f_bsize * ((int64_t)diskInfo.f_blocks - (int64_t)diskInfo.f_bfree);
    }
    return E_OK;
}
} // StorageManager
} // OHOS
