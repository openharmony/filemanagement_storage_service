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
 
void StorageRadar::ReportFbexResult(const std::string &funcName, uint32_t userId, int ret, const std::string &keyLevel,
    const std::string &extraData)
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

void StorageRadar::RecordMountFail(const std::string &extraData, int32_t errcode)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = "Mount",
        .bizScene = BizScene::USER_MOUNT_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_USER_MOUNT,
        .keyElxLevel = "NA",
        .errorCode = errcode,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::RecordUMountFail(const std::string &extraData, int32_t errcode)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = "Mount",
        .bizScene = BizScene::USER_UMOUNT_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_USER_UMOUNT,
        .keyElxLevel = "NA",
        .errorCode = errcode,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

void StorageRadar::RecordPrepareDirFail(const std::string &extraData, int32_t errcode)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = "Mount",
        .bizScene = BizScene::USER_DIR_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_USER_DIR,
        .keyElxLevel = "NA",
        .errorCode = errcode,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
}

bool StorageRadar::RecordFindProcess(const std::string &extraData, int32_t errcode)
{
    RadarParameter param = {
        .orgPkg = DEFAULT_ORGPKGNAME,
        .userId = DEFAULT_USERID,
        .funcName = "FindProcess",
        .bizScene = BizScene::PROCESS_MANAGER,
        .bizStage = BizStage::BIZ_STAGE_FIND_PROCESS,
        .keyElxLevel = "NA",
        .errorCode = errcode,
        .extraData = extraData
    };
    StorageRadar::GetInstance().RecordFuctionResult(param);
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
} // namespace StorageService
} // namespace OHOS