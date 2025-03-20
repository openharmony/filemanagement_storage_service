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

#include "hisysevent.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include <chrono>
#include <sstream>
#include <iomanip>

namespace OHOS {
namespace StorageService {
void StorageRadar::ReportActiveUserKey(const std::string &funcName, uint32_t userId, int ret,
    const std::string &keyElxLevel)
{
    RadarParameter param = {
        .orgPkg = "account_mgr",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = keyElxLevel,
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
    enum BizStage stage)
{
    RadarParameter param = {
        .orgPkg = "account_mgr",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_MOUNT_MANAGER,
        .bizStage = stage,
        .keyElxLevel = "ELx",
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportUpdateUserAuth(const std::string &funcName, uint32_t userId, int ret,
    const std::string &keyLevel, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = "account_mgr",
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

void StorageRadar::ReportFbexResult(const std::string &funcName, uint32_t userId, int ret,
    const std::string &keyLevel, const std::string &extraData)
{
    RadarParameter param = {
        .orgPkg = "fbex",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = keyLevel,
        .errorCode = ret,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportIamResult(const std::string &funcName, uint32_t userId, int ret)
{
    RadarParameter param = {
        .orgPkg = "iam",
        .userId = userId,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::ReportHuksResult(const std::string &funcName, int ret)
{
    RadarParameter param = {
        .orgPkg = "huks",
        .userId = DEFAULT_USERID,
        .funcName = funcName,
        .bizScene = BizScene::USER_KEY_ENCRYPTION,
        .bizStage = BizStage::BIZ_STAGE_ACTIVE_USER_KEY,
        .keyElxLevel = "NA",
        .errorCode = ret
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

bool StorageRadar::RecordKillProcessResult(std::string processName, int32_t errcode)
{
    int32_t res = E_OK;
    if (errcode == E_OK) {
        res = HiSysEventWrite(
            STORAGESERVICE_DOAMIN,
            UMOUNT_FAIL_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", DEFAULT_ORGPKGNAME,
            "FUNC", "FindAndKillProcess",
            "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    } else {
        res = HiSysEventWrite(
            STORAGESERVICE_DOAMIN,
            UMOUNT_FAIL_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", DEFAULT_ORGPKGNAME,
            "FUNC", "FindAndKillProcess",
            "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "ERROR_CODE", errcode,
            "PROCESS_NAME", processName);
    }
    if (res != E_OK) {
        LOGE("StorageRadar ERROR, res :%{public}d", res);
        return false;
    }
    return true;
}

bool StorageRadar::RecordFuctionResult(const RadarParameter &parRes)
{
    int32_t res = E_OK;
    if (parRes.errorCode == E_OK) {
        res = HiSysEventWrite(
            STORAGESERVICE_DOAMIN,
            FILE_STORAGE_MANAGER_FAULT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", parRes.orgPkg,
            "USER_ID", parRes.userId,
            "FUNC", parRes.funcName,
            "BIZ_SCENE", static_cast<int32_t>(parRes.bizScene),
            "BIZ_STAGE", static_cast<int32_t>(parRes.bizStage),
            "kEY_ELX_LEVEL", parRes.keyElxLevel,
            "TO_CALL_PKG", "FBEX, HUKS, KEY_RING, BUNDLE_MANAGER, HMDFS",
            "DISK_VOLUME_INFO", "{\"diskId\":\"ab12\", \"volumeId\":\"34cd\", \"fsType\":\"ntfs\"}",
            "FILE_STATUS", parRes.extraData,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START));
    } else {
        res = HiSysEventWrite(
            STORAGESERVICE_DOAMIN,
            FILE_STORAGE_MANAGER_FAULT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::FAULT,
            "ORG_PKG", parRes.orgPkg,
            "USER_ID", parRes.userId,
            "FUNC", parRes.funcName,
            "BIZ_SCENE", static_cast<int32_t>(parRes.bizScene),
            "BIZ_STAGE", static_cast<int32_t>(parRes.bizStage),
            "kEY_ELX_LEVEL", parRes.keyElxLevel,
            "TO_CALL_PKG", "FBEX, HUKS, KEY_RING, BUNDLE_MANAGER, HMDFS",
            "DISK_VOLUME_INFO", "{\"diskId\":\"ab12\", \"volumeId\":\"34cd\", \"fsType\":\"ntfs\"}",
            "FILE_STATUS", parRes.extraData,
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START),
            "ERROR_CODE", parRes.errorCode);
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
    int32_t res = HiSysEventWrite(
        STORAGESERVICE_DOAMIN,
        FILE_STORAGE_MANAGER_STATISTIC,
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "USER_ID", userId,
        "TIME", GetCurrentTime(),
        "KEY_LOAD_SUCC_CNT", radarInfo.keyLoadSuccCount,
        "KEY_LOAD_FAIL_CNT", radarInfo.keyLoadFailCount,
        "KEY_UNLOAD_SUCC_CNT", radarInfo.keyUnloadSuccCount,
        "KEY_UNLOAD_FAIL_CNT", radarInfo.keyUnloadFailCount,
        "USER_ADD_SUCC_CNT", radarInfo.userAddSuccCount,
        "USER_ADD_FAIL_CNT", radarInfo.userAddFailCount,
        "USER_REMOVE_SUCC_CNT", radarInfo.userRemoveSuccCount,
        "USER_REMOVE_FAIL_CNT", radarInfo.userRemoveFailCount,
        "USER_START_SUCC_CNT", radarInfo.userStartSuccCount,
        "USER_START_FAIL_CNT", radarInfo.userStartFailCount,
        "USER_STOP_SUCC_CNT", radarInfo.userStopSuccCount,
        "USER_STOP_FAIL_CNT", radarInfo.userStopFailCount);
    if (res != E_OK) {
        LOGE("StorageRadar ERROR, res :%{public}d", res);
    }
}
} // namespace StorageService
} // namespace OHOS