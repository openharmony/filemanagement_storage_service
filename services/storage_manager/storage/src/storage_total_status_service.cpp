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
#include <cinttypes>
#include "hitrace_meter.h"
#include <mntent.h>
#include <singleton.h>
#include <sys/statvfs.h>
#include <unordered_set>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"

using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
StorageTotalStatusService::StorageTotalStatusService() {}
StorageTotalStatusService::~StorageTotalStatusService() {}

int32_t StorageTotalStatusService::GetSystemSize(int64_t &systemSize)
{
    int64_t roundSize = 0;
    int32_t ret = GetTotalSize(roundSize);
    if (ret != E_OK) {
        LOGE("storage total status service GetTotalSize failed, please check");
        RadarParameter parameterRes = {.orgPkg = DEFAULT_ORGPKGNAME,
                                       .userId = DEFAULT_USERID,
                                       .funcName = "GetTotalSize",
                                       .bizScene = BizScene::SPACE_STATISTICS,
                                       .bizStage = BizStage::BIZ_STAGE_GET_SYSTEM_SIZE,
                                       .keyElxLevel = "EL1",
                                       .errorCode = ret};
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        return ret;
    }
    int64_t totalSize = 0;
    ret = GetSizeOfPath(PATH_DATA, SizeType::TOTAL, totalSize);
    if (ret != E_OK) {
        LOGE("storage total status service GetSizeOfPath failed, please check");
        RadarParameter parameterRes = {.orgPkg = DEFAULT_ORGPKGNAME,
                                       .userId = DEFAULT_USERID,
                                       .funcName = "GetSizeOfPath",
                                       .bizScene = BizScene::SPACE_STATISTICS,
                                       .bizStage = BizStage::BIZ_STAGE_GET_SYSTEM_SIZE,
                                       .keyElxLevel = "EL1",
                                       .errorCode = ret};
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        return ret;
    }
    systemSize = roundSize - totalSize;
    LOGE("StorageTotalStatusService::GetSystemSize success, roundSize=%{public}" PRId64
         ",(/data)totalSize=%{public}" PRId64 ",systemSize=%{public}" PRId64, roundSize, totalSize, systemSize);
    return E_OK;
}

int32_t StorageTotalStatusService::GetTotalSize(int64_t &totalSize)
{
    HITRACE_METER_NAME(HITRACE_TAG_FILEMANAGEMENT, __PRETTY_FUNCTION__);

    int64_t dataSize = 0;
    int32_t ret = GetSizeOfPath(PATH_DATA, SizeType::TOTAL, dataSize);
    if (ret != E_OK) {
        LOGE("GetSizeOfPath of data size failed, please check");
        RadarParameter parameterRes = {.orgPkg = DEFAULT_ORGPKGNAME,
                                       .userId = DEFAULT_USERID,
                                       .funcName = "GetTotalSize GetSizeOfPath for dataSize",
                                       .bizScene = BizScene::SPACE_STATISTICS,
                                       .bizStage = BizStage::BIZ_STAGE_GET_TOTAL_SIZE,
                                       .keyElxLevel = "EL1",
                                       .errorCode = ret};
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        return ret;
    }
    int64_t rootSize = 0;
    ret = GetSizeOfPath(PATH_ROOT, SizeType::TOTAL, rootSize);
    if (ret != E_OK) {
        LOGE("GetSizeOfPath of root size failed, please check");
        RadarParameter parameterRes = {.orgPkg = DEFAULT_ORGPKGNAME,
                                       .userId = DEFAULT_USERID,
                                       .funcName = "GetTotalSize GetSizeOfPath for rootSize",
                                       .bizScene = BizScene::SPACE_STATISTICS,
                                       .bizStage = BizStage::BIZ_STAGE_GET_TOTAL_SIZE,
                                       .keyElxLevel = "EL1",
                                       .errorCode = ret};
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
        return ret;
    }
    totalSize = GetRoundSize(dataSize + rootSize);
    LOGE("StorageTotalStatusService::GetTotalSize success, roundSize=%{public}" PRId64
         ", (/data)totalDataSize=%{public}" PRId64 ", (/)totalRootSize=%{public}" PRId64,
         totalSize, dataSize, rootSize);
    return E_OK;
}

int32_t StorageTotalStatusService::GetFreeSize(int64_t &freeSize)
{
    int32_t ret = GetSizeOfPath(PATH_DATA, SizeType::FREE, freeSize);
    if (ret != E_OK) {
        LOGE("GetFreeSize failed, please check");
        RadarParameter parameterRes = {.orgPkg = DEFAULT_ORGPKGNAME,
                                       .userId = DEFAULT_USERID,
                                       .funcName = "GetFreeSize",
                                       .bizScene = BizScene::SPACE_STATISTICS,
                                       .bizStage = BizStage::BIZ_STAGE_GET_FREE_SIZE,
                                       .keyElxLevel = "EL1",
                                       .errorCode = ret};
        StorageService::StorageRadar::GetInstance().RecordFuctionResult(parameterRes);
    }
    LOGE("StorageTotalStatusService::GetFreeSize success, (/data)freeSize=%{public}" PRId64, freeSize);
    return ret;
}

int32_t StorageTotalStatusService::GetSizeOfPath(const char *path, int32_t type, int64_t &size)
{
    struct statvfs diskInfo;
    int ret = statvfs(path, &diskInfo);
    if (ret != E_OK) {
        return E_ERR;
    }
    std::string typeStr = "";
    if (type == SizeType::TOTAL) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_blocks;
        typeStr = "total space";
    } else if (type == SizeType::FREE) {
        size = (int64_t)diskInfo.f_bsize * (int64_t)diskInfo.f_bfree;
        typeStr = "free space";
    } else {
        size = (int64_t)diskInfo.f_bsize * ((int64_t)diskInfo.f_blocks - (int64_t)diskInfo.f_bfree);
        typeStr = "used space";
    }
    LOGE("StorageStatusService::GetSizeOfPath path is %{public}s, type is %{public}s, size is %{public}." PRId64,
         path, typeStr.c_str(), size);
    return E_OK;
}
} // StorageManager
} // OHOS
