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

#include "utils/storage_radar.h"
#include "utils/string_utils.h"

#include "hisysevent.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include <iomanip>

namespace OHOS {
namespace StorageService {
constexpr const char *FILE_STORAGE_MANAGER_FAULT_BEHAVIOR  = "FILE_STORAGE_MANAGER_FAULT";
constexpr const char *FILE_STORAGE_MANAGER_STATISTIC = "FILE_STORAGE_MANAGER_STATISTIC";
constexpr char STORAGESERVICE_DOAMIN[] = "FILEMANAGEMENT";
constexpr uint8_t INDEX = 3;
constexpr uint32_t MS_1000 = 1000;
constexpr int32_t GLOBAL_USER_ID = 0;
constexpr int32_t PARAMS_LEN = 12;
constexpr int32_t STAGE_FAIL_INDEX = 9;
constexpr int32_t ERROR_CODE_INDEX = 11;

constexpr const char *TAG_PREFIX = " WARNING: DELAY > ";
constexpr const char *TAG_UNIT_SUFFIX = " ms.";

void StorageRadar::ReportActiveUserKey(const std::string &funcName, uint32_t userId, int ret,
                                       const std::string &keyLevel)
{
    RadarParameter param = {
        .orgPkg = "os_account",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = keyLevel,
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportGetStorageStatus(const std::string &funcName, uint32_t userId, int ret,
                                          const std::string &orgPkg)
{
    RadarParameter param = {
        .orgPkg = orgPkg,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::SPACE_STATISTICS,
        .bizStage = BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS,
        .keyElxLevel = "NA",
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportVolumeOperation(const std::string &funcName, int ret)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::EXTERNAL_VOLUME_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportUserKeyResult(const std::string &funcName, uint32_t userId, int ret,
    const std::string &keyLevel, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = "init",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_INIT_GLOBAL_KEY,
        .keyElxLevel = keyLevel,
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportUserManager(const std::string &funcName, uint32_t userId, int ret,
    const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_MOUNT_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_USER_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportSaSizeResult(const std::string &funcName, int ret, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = GLOBAL_USER_ID,
        .funcName = funcName,
        .bizScene = BizScene::SPACE_STATISTICS,
        .bizStage = BizStage::BIZ_STAGE_GET_SYSTEM_SIZE,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportSpaceRadar(const std::string &funcName, int ret, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = GLOBAL_USER_ID,
        .funcName = funcName,
        .bizScene = BizScene::SPACE_STATISTICS,
        .bizStage = BizStage::BIZ_STAGE_GET_USER_STORAGE_STATS,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportUpdateUserAuth(const std::string &funcName, uint32_t userId, int ret,
    const std::string &keyLevel, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = "os_account",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_UPDATE_USER_AUTH,
        .keyElxLevel = keyLevel,
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportFbexResult(const std::string &funcName, uint32_t userId, int ret, const std::string &keyLevel,
    const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = keyLevel,
        .errorCode = ret,
        .extraData = extraData,
        .toCallPkg = "fbex"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportIamResult(const std::string &funcName, uint32_t userId, int ret)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .toCallPkg = "iam"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportHuksResult(const std::string &funcName, int ret)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .toCallPkg = "huks"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportMtpResult(const std::string &funcName, int ret, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::MTP_DEVICE_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_MTPFS_MTP_DEVICE,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData,
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportKeyRingResult(const std::string &funcName, int ret, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportOsAccountResult(const std::string &funcName, int32_t ret, unsigned int userId)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .toCallPkg = "os_account"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportEl5KeyMgrResult(const std::string &funcName, int32_t ret, unsigned int userId)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_UNLOCK_USER_SCREEN,
        .keyElxLevel = "EL5",
        .errorCode = ret,
        .toCallPkg = "el5_file_key_manager"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportTEEClientResult(const std::string &funcName, int32_t ret, unsigned int userId,
    const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_UNLOCK_USER_SCREEN,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData,
        .toCallPkg = "tee_client"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportCommonResult(const std::string &funcName, int32_t ret, unsigned int userId,
    const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_NOT_PERMISSION,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData,
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportBundleMgrResult(const std::string &funcName, int32_t ret, unsigned int userId,
    const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::SPACE_STATISTICS,
        .bizStage = BizStage::BIZ_STAGE_GET_BUNDLE_STATS,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData,
        .toCallPkg = "bundle_mgr"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportStorageUsage(enum BizStage stage, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = "CheckAndCleanCache",
        .bizScene = BizScene::STORAGE_USAGE_MANAGER,
        .bizStage = stage,
        .keyElxLevel = "NA",
        .errorCode = E_STORAGE_USAGE_NOT_ENOUGH,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

bool StorageRadar::RecordFuctionResult(const RadarParameter &parRes)
{
    int32_t res = E_OK;
    const char* DISK_VOLUME_INFO_STR = "{\"diskId\":\"ab12\", \"volumeId\":\"34cd\", \"fsType\":\"ntfs\"}";
    HiSysEventParam params[PARAMS_LEN] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)parRes.orgPkg.c_str() }, .arraySize = 0, },
        {.name = "USER_ID", .t = HISYSEVENT_INT32, .v = { .i32 = parRes.userId }, .arraySize = 0, },
        {.name = "FUNC", .t = HISYSEVENT_STRING, .v = { .s = (char *)parRes.funcName.c_str() }, .arraySize = 0, },
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32, .v = { .i32 = static_cast<int32_t>(parRes.bizScene) },
            .arraySize = 0, },
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v = { .i32 = static_cast<int32_t>(parRes.bizStage) },
            .arraySize = 0, },
        {.name = "kEY_ELX_LEVEL", .t = HISYSEVENT_STRING, .v = { .s = (char *)parRes.keyElxLevel.c_str() },
            .arraySize = 0, },
        {.name = "TO_CALL_PKG", .t = HISYSEVENT_STRING, .v = { .s = (char *)parRes.toCallPkg.c_str() },
            .arraySize = 0, },
        {.name = "DISK_VOLUME_INFO", .t = HISYSEVENT_STRING, .v = { .s = (char *)DISK_VOLUME_INFO_STR },
            .arraySize = 0, },
        {.name = "FILE_STATUS", .t = HISYSEVENT_STRING, .v = { .s = (char *)parRes.extraData.c_str() },
            .arraySize = 0, },
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v = { .i32 = static_cast<int32_t>(StageRes::STAGE_SUCC) },
            .arraySize = 0, },
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v = { .i32 = static_cast<int32_t>(BizState::BIZ_STATE_START) },
            .arraySize = 0, },
        {},
    };
    size_t len = 11;
    if (parRes.errorCode == E_OK) {
        res = OH_HiSysEvent_Write(STORAGESERVICE_DOAMIN, FILE_STORAGE_MANAGER_FAULT_BEHAVIOR,
            HISYSEVENT_BEHAVIOR, params, len);
    } else {
        const int32_t stageFail = static_cast<int32_t>(StageRes::STAGE_FAIL);
        params[STAGE_FAIL_INDEX].v.i32 = stageFail;
        params[ERROR_CODE_INDEX] = {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v = { .i32 = parRes.errorCode },
            .arraySize = 0 };
        len++;
        res = OH_HiSysEvent_Write(STORAGESERVICE_DOAMIN, FILE_STORAGE_MANAGER_FAULT_BEHAVIOR,
            HISYSEVENT_FAULT, params, len);
    }
    if (res != E_OK) {
        LOGE("StorageRadar ERROR, res :%{public}d", res);
        return false;
    }
    return true;
}

static std::string GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    std::stringstream strTime;
    strTime << (std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S:")) << (std::setfill('0'))
            << (std::setw(INDEX)) << (ms.count() % MS_1000);
    return strTime.str();
}

void StorageRadar::ReportStatistics(uint32_t userId, StorageDaemon::RadarStatisticInfo radarInfo)
{
    std::string timeStr = GetCurrentTime();
    HiSysEventParam params[] = {
        {.name = "USER_ID", .t = HISYSEVENT_UINT32, .v = { .ui32 = userId }, .arraySize = 0, },
        {.name = "TIME", .t = HISYSEVENT_STRING, .v = { .s = (char *)timeStr.c_str() }, .arraySize = 0, },
        {.name = "KEY_LOAD_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.keyLoadSuccCount },
            .arraySize = 0, },
        {.name = "KEY_LOAD_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.keyLoadFailCount },
            .arraySize = 0, },
        {.name = "KEY_UNLOAD_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.keyUnloadSuccCount },
            .arraySize = 0, },
        {.name = "KEY_UNLOAD_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.keyUnloadFailCount },
            .arraySize = 0, },
        {.name = "USER_ADD_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userAddSuccCount },
            .arraySize = 0, },
        {.name = "USER_ADD_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userAddFailCount },
            .arraySize = 0, },
        {.name = "USER_REMOVE_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userRemoveSuccCount },
            .arraySize = 0, },
        {.name = "USER_REMOVE_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userRemoveFailCount },
            .arraySize = 0, },
        {.name = "USER_START_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userStartSuccCount },
            .arraySize = 0, },
        {.name = "USER_START_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userStartFailCount },
            .arraySize = 0, },
        {.name = "USER_STOP_SUCC_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userStopSuccCount },
            .arraySize = 0, },
        {.name = "USER_STOP_FAIL_CNT", .t = HISYSEVENT_UINT64, .v = { .ui64 = radarInfo.userStopFailCount },
            .arraySize = 0, },
    };
    size_t len = sizeof(params) / sizeof(params[0]);
    int32_t res = OH_HiSysEvent_Write(STORAGESERVICE_DOAMIN, FILE_STORAGE_MANAGER_STATISTIC,
        HISYSEVENT_STATISTIC, params, len);
    if (res != E_OK) {
        LOGE("StorageRadar ERROR, res :%{public}d", res);
    }
}

int64_t StorageRadar::RecordCurrentTime()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
           std::chrono::system_clock::now().time_since_epoch()
           ).count();
}

std::string StorageRadar::ReportDuration(const std::string &funcName, int64_t startTime,
                                         int64_t delay_threshold, uint32_t userId)
{
    auto duration = RecordCurrentTime() - startTime;
    std::string ret = std::to_string(duration) + TAG_UNIT_SUFFIX;
    if (duration <= delay_threshold) {
        return ret;
    }
    std::string tag = TAG_PREFIX + std::to_string(delay_threshold) + TAG_UNIT_SUFFIX;
    std::string extraData = ret + tag;
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .keyElxLevel = "NA",
        .errorCode = E_DURATION_EXCEED_THRESH,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
    return ret;
}

void StorageRadar::ReportSetQuotaByBaseline(const std::string &funcName, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::SPACE_STATISTICS,
        .bizStage = BizStage::BIZ_STAGE_SET_BUNDLE_QUOTA,
        .keyElxLevel = "NA",
        .errorCode = E_SET_QUOTA_UID_FAILED,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportFucBehavior(const std::string &funcName,
                                     uint32_t userId,
                                     const std::string &extraData,
                                     int32_t ret)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::STORAGE_START,
        .bizStage = BizStage::BIZ_STAGE_SA_START,
        .keyElxLevel = "NA",
        .errorCode = ret,
        .extraData = extraData,
        .toCallPkg = "NA"
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}
} // namespace StorageService
} // namespace OHOS