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
bool Storage::AddNewUser(int32_t errcode)
{
    int32_t res = E_OK;
    if (errcode == E_OK) {
        res = HisysEventWrite(
            STORAGESERVICE_DOAMIN,
            ADD_NEW_USER_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "FUNC", "PrepareUserDirs",
            "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
            "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    } else {
        res = HisysEventWrite(
                STORAGESERVICE_DOAMIN,
                ADD_NEW_USER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "FUNC", "PrepareUserDirs",
                "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
                "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
                "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END)
                "ERROR_CODE", errcode);
    }
    return true;
}

bool Storage::ActiveCurrentUser(int32_t errcode)
{
    int32_t res = E_OK;
    if (errcode == E_OK) {
        res = HisysEventWrite(
                STORAGESERVICE_DOAMIN,
                ACTIVE_CURRENT_USER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "FUNC", "ActiveUserKey",
                "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
                "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
                "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    } else {
        res = HisysEventWrite(
                STORAGESERVICE_DOAMIN,
                ACTIVE_CURRENT_USER_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "FUNC", "ActiveUserKey",
                "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
                "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
                "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "ERROR_CODE", errcode);
    }
    return true;
}

bool Storage::UmountFail(std::string processName, int32_t errcode)
{
    int32_t res = E_OK;
    if (errcode == E_OK) {
        res = HisysEventWrite(
                STORAGESERVICE_DOAMIN,
                UMOUNT_FAIL_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "FUNC", "FindAndKillProcess",
                "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
                "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCC),
                "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END));
    } else {
        res = HisysEventWrite(
                STORAGESERVICE_DOAMIN,
                UMOUNT_FAIL_BEHAVIOR,
                HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
                "ORG_PKG", ORGPKGNAME,
                "FUNC", "FindAndKillProcess",
                "BIZ_SCENE", static_cast<int32_t>(BizScene::STORAGE_START),
                "BIZ_STAGE", static_cast<int32_t>(BizStage::BIZ_STAGE_SA_START),
                "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
                "BIZ_SCENE", static_cast<int32_t>(BizState::BIZ_STATE_END),
                "ERROR_CODE", errcode,
                "PROCESS_NAME", processName);
    }
    return true;
}

} // namespace StorageService
}// namespace OHOS