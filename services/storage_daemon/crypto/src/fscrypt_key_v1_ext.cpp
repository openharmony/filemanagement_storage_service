/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "fscrypt_key_v1_ext.h"

#include <filesystem>

#include "fbex.h"
#include "file_ex.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *NEED_RESTORE_PATH = "/data/service/el0/storage_daemon/sd/latest/need_restore";
constexpr const char *EL4_NEED_RESTORE_PATH = "/data/service/el1/public/storage_daemon/sd/el4/%d/latest/need_restore";
constexpr int NEW_DOUBLE_2_SINGLE_BASE_VERSION = 2;
constexpr uint8_t RECREATE_KEY = 0x6c;
constexpr uint32_t DEFAULT_SINGLE_FIRST_USER_ID = 100;
constexpr uint32_t USER_ID_DIFF = 91;

/* sec_fbe_drv module saved userId, type and IV in ioctl FBEX_IOC_ADD_IV process.
 * In Upgrade Scenario, before upgrade user id 0 and after upgrade  user id 100, ioctl
 * FBEX_IOC_ADD_IV will return error.
 * so, to solve this problem, when el3/el4 ioctl FBEX_IOC_ADD_IV, userId need be mapped
 * in Upgrade Scenario.
 * user id mapped as:
 * src-userId  mapped-userId    diff
 * 0               100          100
 * 10              101           91
 * 11              102           91
 * 12              103           91
 * ...             ...           91
 * 128             219           91
 * 129             220           91
 * ...             ...           91
 */
uint32_t FscryptKeyV1Ext::GetMappedUserId(uint32_t userId, uint32_t type)
{
    LOGD("[L6:FscryptExt] GetMappedUserId: >>> ENTER <<< userId=%{public}u, type=%{public}u", userId, type);
    std::error_code errCode;
    if (!std::filesystem::exists(NEED_RESTORE_PATH, errCode)) {
        LOGE("[L6:FscryptExt] GetMappedUserId: <<< EXIT SUCCESS <<< restore path not exists, return original"
             "userId=%{public}u", userId);
        return userId;
    }
    if (type == TYPE_EL2 || type == TYPE_EL3 || type == TYPE_EL4 || type == TYPE_EL5) {
        if (userId == DEFAULT_SINGLE_FIRST_USER_ID) {
            LOGI("[L6:FscryptExt] GetMappedUserId: <<< EXIT SUCCESS <<< mapped userId 100 to 0");
            return 0;
        }

        if (userId > DEFAULT_SINGLE_FIRST_USER_ID) {
            LOGI("[L6:FscryptExt] GetMappedUserId: <<< EXIT SUCCESS <<< mapped userId %{public}u to %{public}u",
                 userId, userId - USER_ID_DIFF);
            return userId - USER_ID_DIFF;
        }
    }

    LOGI("[L6:FscryptExt] GetMappedUserId: <<< EXIT SUCCESS <<< return original userId=%{public}u", userId);
    return userId;
}

uint32_t FscryptKeyV1Ext::GetMappedDeUserId(uint32_t userId)
{
    LOGD("[L6:FscryptExt] GetMappedDeUserId: >>> ENTER <<< userId=%{public}u", userId);
    if (userId == DEFAULT_SINGLE_FIRST_USER_ID) {
        LOGI("[L6:FscryptExt] GetMappedDeUserId: <<< EXIT SUCCESS <<< mapped userId 100 to 0");
        return 0;
    }

    if (userId > DEFAULT_SINGLE_FIRST_USER_ID) {
        LOGI("[L6:FscryptExt] GetMappedDeUserId: <<< EXIT SUCCESS <<< mapped userId %{public}u to %{public}u",
             userId, userId - USER_ID_DIFF);
        return userId - USER_ID_DIFF;
    }
    LOGI("[L6:FscryptExt] GetMappedDeUserId: <<< EXIT SUCCESS <<< return original userId=%{public}u", userId);
    return userId;
}

int32_t FscryptKeyV1Ext::ActiveKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    LOGI("[L6:FscryptExt] ActiveKeyExt: >>> ENTER <<< flag=%{public}u, elType=%{public}u", flag, elType);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] ActiveKeyExt: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("FBEX: IS_SUPPORT", startTime);
    LOGI("[L6:FscryptExt] SD_DURATION: FBEX SUPPORT: elType=%{public}d, delay time = %{public}s",
         elType, delay.c_str());
    std::error_code errCode;
    std::string updateVersion;
    int ret = OHOS::LoadStringFromFile(NEED_RESTORE_PATH, updateVersion);
    LOGI("[L6:FscryptExt] ActiveKeyExt: restore version: %{public}s, ret: %{public}d", updateVersion.c_str(), ret);
    if (type_ == TYPE_EL1 && std::filesystem::exists(NEED_RESTORE_PATH, errCode) &&
        std::atoi(updateVersion.c_str()) >= NEW_DOUBLE_2_SINGLE_BASE_VERSION) {
        LOGI("[L6:FscryptExt] ActiveKeyExt: restore path exists, deal double DE");
        ret = ActiveDoubleKeyExt(flag, iv, elType, authToken);
        if (ret != E_OK) {
            LOGE("[L6:FscryptExt] ActiveKeyExt: <<< EXIT FAILED <<< ActiveDoubleKeyExt failed");
        } else {
            LOGI("[L6:FscryptExt] ActiveKeyExt: <<< EXIT SUCCESS <<<");
        }
        return ret;
    }
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] ActiveKeyExt: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, user);
    // iv buffer returns derived keys
    int errNo = FBEX::InstallKeyToKernel(user, type_, iv, static_cast<uint8_t>(flag), authToken);
    if (errNo == FILE_ENCRY_ERROR_EL3_STAUTS_WRONG && elType == TYPE_EL3) {
        if (!std::filesystem::exists(StringPrintf(EL4_NEED_RESTORE_PATH, userId_), errCode)) {
            LOGE("[L6:FscryptExt] ActiveKeyExt: <<< EXIT FAILED <<< need restore does not exist");
            return errNo;
        }
        errNo = FBEX::InstallKeyToKernel(user, type_, iv, static_cast<uint8_t>(RECREATE_KEY), authToken);
        if (errNo == E_OK) {
            LOGE("[L6:FscryptExt] ActiveKeyExt: Recreat iv success, user %{public}d, type %{public}d, flag %{public}u",
                 user, type_, flag);
            elType = type_;
            LOGI("[L6:FscryptExt] ActiveKeyExt: <<< EXIT SUCCESS <<<");
            return E_OK;
        }
        LOGE("[L6:FscryptExt] ActiveKeyExt: <<< EXIT FAILED <<< Recreat failed, user %{public}d, type %{public}d,"
             "flag %{public}u, err:%{public}d",
            user, type_, flag, errNo);
        return errNo;
    }
    if (errNo != 0) {
        if (flag != 0) {
            LOGE("[L6:FscryptExt] ActiveKeyExt: <<< EXIT FAILED <<< First install failed, user %{public}d, type"
                 "%{public}d, flag %{public}u", user, type_, flag);
            return errNo;
        }
        LOGE("[L6:FscryptExt] ActiveKeyExt: Try install again, user %{public}d, type %{public}d, flag %{public}u",
             user, type_, flag);
        errNo = FBEX::InstallKeyToKernel(user, type_, iv, static_cast<uint8_t>(flag), authToken);
        if (errNo != 0) {
            LOGE("[L6:FscryptExt] ActiveKeyExt: <<< EXIT FAILED <<< InstallKeyToKernel failed, user %{public}d, type"
                 "%{public}d, flag %{public}u", user, type_, flag);
            return errNo;
        }
    }
    //Used to associate el3 and el4 kernels.
    elType = type_;
    LOGI("[L6:FscryptExt] ActiveKeyExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::ActiveDoubleKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    LOGI("[L6:FscryptExt] ActiveDoubleKeyExt: >>> ENTER <<< flag=%{public}u", flag);
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedDeUserId(userId_) }, .size = USER_ID_SIZE };
    int32_t errNo = FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv, flag, authToken);
    if (errNo != 0) {
        LOGE("[L6:FscryptExt] ActiveDoubleKeyExt: <<< EXIT FAILED <<< DoubleDeKeyToKernel failed, user %{public}d,"
             "type %{public}d, flag %{public}u", userId_, type_, flag);
        return errNo;
    }
    elType = type_;
    LOGI("[L6:FscryptExt] ActiveDoubleKeyExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::UnlockUserScreenExt(uint32_t flag, uint8_t *iv, uint32_t size, const KeyBlob &authToken)
{
    LOGI("[L6:FscryptExt] UnlockUserScreenExt: >>> ENTER <<< flag=%{public}u", flag);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] UnlockUserScreenExt: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] UnlockUserScreenExt: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, user);
    int32_t ret = FBEX::UnlockScreenToKernel(user, type_, iv, size, authToken);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] UnlockUserScreenExt: <<< EXIT FAILED <<< UnlockScreenToKernel failed, userId"
             "%{public}d, %{public}d", userId_, flag);
        return ret;
    }
    LOGI("[L6:FscryptExt] UnlockUserScreenExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::GenerateAppkey(uint32_t user, uint32_t hashId,
                                        std::unique_ptr<uint8_t[]> &appKey, uint32_t size)
{
    LOGI("[L6:FscryptExt] GenerateAppkey: >>> ENTER <<< userId=%{public}u, hashId=%{public}u", user, hashId);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] GenerateAppkey: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    LOGI("[L6:FscryptExt] GenerateAppkey: map userId %{public}u to %{public}u", userId_, user);
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    auto ret = FBEX::GenerateAppkey(userIdToFbe, hashId, appKey, size);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] GenerateAppkey: <<< EXIT FAILED <<< GenerateAppkey failed, user %{public}d", user);
        return ret;
    }
    LOGI("[L6:FscryptExt] GenerateAppkey: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    LOGI("[L6:FscryptExt] AddClassE: >>> ENTER <<< status=%{public}u", status);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] AddClassE: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] AddClassE: map userId %{public}u to %{public}u", userId_, userIdDouble);
    auto ret = FBEX::InstallEL5KeyToKernel(userId_, userIdDouble, status, isSupport, isNeedEncryptClassE);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] AddClassE: <<< EXIT FAILED <<< AddESecret failed, userId_ %{public}d, status is"
             "%{public}d", userId_, status);
        return ret;
    }
    LOGI("[L6:FscryptExt] AddClassE: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::DeleteClassEPinCode(uint32_t userId)
{
    LOGI("[L6:FscryptExt] DeleteClassEPinCode: >>> ENTER <<< userId=%{public}u", userId);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] DeleteClassEPinCode: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] DeleteClassEPinCode: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdDouble);
    auto ret = FBEX::DeleteClassEPinCode(userId_, userIdDouble);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] DeleteClassEPinCode: <<< EXIT FAILED <<< UninstallOrLockUserKeyForEL5ToKernel failed,"
             "userId_ %{public}d", userId_);
        return ret;
    }
    LOGI("[L6:FscryptExt] DeleteClassEPinCode: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1Ext::ChangePinCodeClassE(uint32_t userId, bool &isFbeSupport)
{
    LOGI("[L6:FscryptExt] ChangePinCodeClassE: >>> ENTER <<< userId=%{public}u", userId);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] ChangePinCodeClassE: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] ChangePinCodeClassE: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdDouble);
    auto ret = FBEX::ChangePinCodeClassE(userId_, userIdDouble, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] ChangePinCodeClassE: <<< EXIT FAILED <<< ChangePinCodeClassE failed, userId_ %{public}d",
             userId_);
        return ret;
    }
    LOGI("[L6:FscryptExt] ChangePinCodeClassE: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1Ext::UpdateClassEBackUp(uint32_t userId)
{
    LOGI("[L6:FscryptExt] UpdateClassEBackUp: >>> ENTER <<< userId=%{public}u", userId);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] UpdateClassEBackUp: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] UpdateClassEBackUp: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdDouble);
    auto ret = FBEX::UpdateClassEBackUp(userId_, userIdDouble);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] UpdateClassEBackUp: <<< EXIT FAILED <<< UpdateClassEBackUp failed, userId_ %{public}d",
             userId_);
        return ret;
    }
    LOGI("[L6:FscryptExt] UpdateClassEBackUp: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return E_OK;
}

int32_t FscryptKeyV1Ext::ReadClassE(uint32_t status, KeyBlob &classEBuffer, const KeyBlob &authToken,
                                    bool &isFbeSupport)
{
    LOGI("[L6:FscryptExt] ReadClassE: >>> ENTER <<< status=%{public}u", status);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] ReadClassE: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    LOGI("[L6:FscryptExt] ReadClassE: type_: %{public}u, userId %{public}u to %{public}u",
         type_, userId_, userIdToFbe.userIds[DOUBLE_ID_INDEX]);
    auto ret = FBEX::ReadESecretToKernel(userIdToFbe, status, classEBuffer, authToken, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] ReadClassE: <<< EXIT FAILED <<< ReadESecret failed, user %{public}d, status: %{public}d",
             userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
        return ret;
    }
    LOGI("[L6:FscryptExt] ReadClassE: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::WriteClassE(uint32_t status, uint8_t *classEBuffer, uint32_t length)
{
    LOGI("[L6:FscryptExt] WriteClassE: >>> ENTER <<< status=%{public}u", status);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] WriteClassE: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    LOGI("[L6:FscryptExt] WriteClassE: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdToFbe.userIds[DOUBLE_ID_INDEX]);
    auto ret = FBEX::WriteESecretToKernel(userIdToFbe, status, classEBuffer, length);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] WriteClassE: <<< EXIT FAILED <<< WriteESecret failed,user %{public}d, status:"
             "%{public}d", userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
        return ret;
    }
    LOGI("[L6:FscryptExt] WriteClassE: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::InactiveKeyExt(uint32_t flag)
{
    LOGI("[L6:FscryptExt] InactiveKeyExt: >>> ENTER <<< flag=%{public}u", flag);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] InactiveKeyExt: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }

    bool destroy = !!flag;
#ifdef EL2_INACTIVE_SPECIAL_HANDLING
    if (!destroy) {
        LOGI("[L6:FscryptExt] InactiveKeyExt: no need to inactive");
#else
    if ((type_ == TYPE_EL1) && !destroy) {
        LOGI("[L6:FscryptExt] InactiveKeyExt: Is el1, no need to inactive");
#endif
        return E_OK;
    }
    uint8_t buf[FBEX_IV_SIZE] = {0};
    buf[0] = 0xfb; // fitst byte const to kernel
    buf[1] = 0x30; // second byte const to kernel

    // When double update single, el5 use single user id, like: 100  101  102 ...
    // el1-el4 use double id, like 0 10 11 12 ...
    uint32_t user = type_ == TYPE_EL5 ? userId_ : GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] InactiveKeyExt: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, user);
    int errNo = FBEX::UninstallOrLockUserKeyToKernel(user, type_, buf, FBEX_IV_SIZE, destroy);
    if (errNo != 0) {
        LOGE("[L6:FscryptExt] InactiveKeyExt: <<< EXIT FAILED <<< UninstallOrLockUserKeyToKernel failed, userId"
             "%{public}d, type %{public}d, destroy %{public}u",
             userId_, type_, destroy);
        return errNo;
    }
    LOGI("[L6:FscryptExt] InactiveKeyExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::LockUserScreenExt(uint32_t flag, uint32_t &elType)
{
    LOGI("[L6:FscryptExt] LockUserScreenExt: >>> ENTER <<< flag=%{public}u", flag);
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] LockUserScreenExt: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] LockUserScreenExt: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, user);
    int32_t ret = FBEX::LockScreenToKernel(user);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] LockUserScreenExt: <<< EXIT FAILED <<< LockScreenToKernel failed, userId %{public}d,"
             "error code: %{public}d", user, ret);
        return ret;
    }
    //Used to associate el3 and el4 kernels.
    elType = type_;
    LOGI("[L6:FscryptExt] LockUserScreenExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t FscryptKeyV1Ext::LockUeceExt(bool &isFbeSupport)
{
    LOGD("[L6:FscryptExt] LockUeceExt: >>> ENTER <<<");
    if (!FBEX::IsFBEXSupported()) {
        LOGI("[L6:FscryptExt] LockUeceExt: <<< EXIT SUCCESS <<< FBEX not supported, return E_OK");
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("[L6:FscryptExt] LockUeceExt: type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdDouble);
    int32_t ret = FBEX::LockUece(userId_, userIdDouble, isFbeSupport);
    if (ret != E_OK) {
        LOGE("[L6:FscryptExt] LockUeceExt: <<< EXIT FAILED <<< LockUeceExt failed");
        return ret;
    }
    LOGI("[L6:FscryptExt] LockUeceExt: <<< EXIT SUCCESS <<<");
    return E_OK;
}

uint32_t FscryptKeyV1Ext::GetUserIdFromDir()
{
    LOGD("[L6:FscryptExt] GetUserIdFromDir: >>> ENTER <<<");
    int userId = USERID_GLOBAL_EL1; // default to global el1

    // fscrypt key dir is like `/data/foo/bar/el1/100`
    auto slashIndex = dir_.rfind('/');
    if (slashIndex != std::string::npos) {
        std::string last = dir_.substr(slashIndex + 1);
        (void)OHOS::StrToInt(last, userId);
    }

    LOGI("[L6:FscryptExt] GetUserIdFromDir: <<< EXIT SUCCESS <<< dir_: %{public}s, get userId is %{public}d",
         dir_.c_str(), userId);
    return static_cast<uint32_t>(userId);
}

uint32_t FscryptKeyV1Ext::GetTypeFromDir()
{
    LOGD("[L6:FscryptExt] GetTypeFromDir: >>> ENTER <<<");
    static const std::vector<std::pair<std::string, uint32_t>> typeStrs = {
        {"el1", TYPE_EL1},
        {"el2", TYPE_EL2},
        {"el3", TYPE_EL3},
        {"el4", TYPE_EL4},
        {"el5", TYPE_EL5},
    };
    uint32_t type = TYPE_GLOBAL_EL1; // default to global el1

    // fscrypt key dir is like `/data/foo/bar/el1/100`
    auto slashIndex = dir_.rfind('/');
    if (slashIndex == std::string::npos) {
        LOGE("[L6:FscryptExt] GetTypeFromDir: <<< EXIT FAILED <<< bad dir %{public}s", dir_.c_str());
        return type;
    }

    if (slashIndex == 0) {
        LOGE("[L6:FscryptExt] GetTypeFromDir: <<< EXIT FAILED <<< bad dir %{public}s", dir_.c_str());
        return type;
    }

    slashIndex = dir_.rfind('/', slashIndex - 1);
    if (slashIndex == std::string::npos) {
        LOGE("[L6:FscryptExt] GetTypeFromDir: <<< EXIT FAILED <<< bad dir %{public}s", dir_.c_str());
        return type;
    }

    std::string el = dir_.substr(slashIndex + 1); // el string is like `el1/100`
    for (const auto &it : typeStrs) {
        if (el.find(it.first) != std::string::npos) {
            type = it.second;
            break;
        }
    }
    LOGI("[L6:FscryptExt] GetTypeFromDir: <<< EXIT SUCCESS <<< el string is %{public}s, parse type %{public}d",
         el.c_str(), type);
    return type;
}
} // namespace StorageDaemon
} // namespace OHOS
