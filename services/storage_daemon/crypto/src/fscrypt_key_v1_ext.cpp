/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace StorageDaemon {
constexpr const char *NEED_RESTORE_PATH = "/data/service/el0/storage_daemon/sd/latest/need_restore";
constexpr int NEW_DOUBLE_2_SINGLE_BASE_VERSION = 2;
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
    std::error_code errCode;
    if (!std::filesystem::exists(NEED_RESTORE_PATH, errCode)) {
        LOGE("restore path not exists, errCode = %{public}d", errCode.value());
        return userId;
    }
    if (type == TYPE_EL2 || type == TYPE_EL3 || type == TYPE_EL4 || type == TYPE_EL5) {
        if (userId == DEFAULT_SINGLE_FIRST_USER_ID) {
            return 0;
        }

        if (userId > DEFAULT_SINGLE_FIRST_USER_ID) {
            return userId - USER_ID_DIFF;
        }
    }

    return userId;
}

uint32_t FscryptKeyV1Ext::GetMappedDeUserId(uint32_t userId)
{
    if (userId == DEFAULT_SINGLE_FIRST_USER_ID) {
        return 0;
    }

    if (userId > DEFAULT_SINGLE_FIRST_USER_ID) {
        return userId - USER_ID_DIFF;
    }
    return userId;
}

int32_t FscryptKeyV1Ext::ActiveKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("FBEX: IS_SUPPORT", startTime);
    LOGI("enter. SD_DURATION: FBEX SUPPORT: elType=%{public}d, delay time = %{public}s", elType, delay.c_str());
    std::error_code errCode;
    std::string updateVersion;
    int ret = OHOS::LoadStringFromFile(NEED_RESTORE_PATH, updateVersion);
    LOGI("restore version: %{public}s, ret: %{public}d", updateVersion.c_str(), ret);
    if (type_ == TYPE_EL1 && std::filesystem::exists(NEED_RESTORE_PATH, errCode) &&
        std::atoi(updateVersion.c_str()) >= NEW_DOUBLE_2_SINGLE_BASE_VERSION) {
        LOGI("restore path exists, deal double DE, errCode = %{public}d", errCode.value());
        return ActiveDoubleKeyExt(flag, iv, elType, authToken);
    }
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    // iv buffer returns derived keys
    int errNo = FBEX::InstallKeyToKernel(user, type_, iv, static_cast<uint8_t>(flag), authToken);
    if (errNo != 0) {
        if (flag != 0) {
            LOGE("New User InstallKeyToKernel failed, user %{public}d, type %{public}d, flag %{public}u", user, type_,
                 flag);
            return errNo;
        }
        LOGE("InstallKeyToKernel first failed, user %{public}d, type %{public}d, flag %{public}u", user, type_, flag);
        errNo = FBEX::InstallKeyToKernel(user, type_, iv, static_cast<uint8_t>(flag), authToken);
        if (errNo != 0) {
            LOGE("InstallKeyToKernel failed, user %{public}d, type %{public}d, flag %{public}u", user, type_, flag);
            return errNo;
        }
    }

    //Used to associate el3 and el4 kernels.
    elType = type_;
    return E_OK;
}

int32_t FscryptKeyV1Ext::ActiveDoubleKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    LOGI("enter");
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedDeUserId(userId_) }, .size = USER_ID_SIZE };
    int32_t errNo = FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv, flag, authToken);
    if (errNo != 0) {
        LOGE("DoubleDeKeyToKernel failed, user %{public}d, type %{public}d, flag %{public}u", userId_, type_, flag);
        return errNo;
    }
    elType = type_;
    return E_OK;
}

int32_t FscryptKeyV1Ext::UnlockUserScreenExt(uint32_t flag, uint8_t *iv, uint32_t size, const KeyBlob &authToken)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    int32_t ret = FBEX::UnlockScreenToKernel(user, type_, iv, size, authToken);
    if (ret != E_OK) {
        LOGE("UnlockScreenToKernel failed, userId %{public}d, %{public}d", userId_, flag);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::GenerateAppkey(uint32_t user, uint32_t hashId,
                                        std::unique_ptr<uint8_t[]> &appKey, uint32_t size)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    LOGI("map userId %{public}u to %{public}u", userId_, user);
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    auto ret = FBEX::GenerateAppkey(userIdToFbe, hashId, appKey, size);
    if (ret != E_OK) {
        LOGE("GenerateAppkey failed, user %{public}d", user);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("map userId %{public}u to %{public}u", userId_, userIdDouble);
    auto ret = FBEX::InstallEL5KeyToKernel(userId_, userIdDouble, status, isSupport, isNeedEncryptClassE);
    if (ret != E_OK) {
        LOGE("AddESecret failed, userId_ %{public}d, status is %{public}d", userId_, status);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::DeleteClassEPinCode(uint32_t userId)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, userIdDouble);
    auto ret = FBEX::DeleteClassEPinCode(userId_, userIdDouble);
    if (ret != E_OK) {
        LOGE("UninstallOrLockUserKeyForEL5ToKernel failed, userId_ %{public}d", userId_);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::ChangePinCodeClassE(uint32_t userId, bool &isFbeSupport)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, userIdDouble);
    auto ret = FBEX::ChangePinCodeClassE(userId_, userIdDouble, isFbeSupport);
    if (ret != E_OK) {
        LOGE("ChangePinCodeClassE failed, userId_ %{public}d", userId_);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::UpdateClassEBackUp(uint32_t userId)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, userIdDouble);
    auto ret = FBEX::UpdateClassEBackUp(userId_, userIdDouble);
    if (ret != E_OK) {
        LOGE("UpdateClassEBackUp failed, userId_ %{public}d", userId_);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::ReadClassE(uint32_t status, KeyBlob &classEBuffer, const KeyBlob &authToken,
                                    bool &isFbeSupport)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    LOGI("type_: %{public}u, userId %{public}u to %{public}u", type_, userId_, userIdToFbe.userIds[DOUBLE_ID_INDEX]);
    auto ret = FBEX::ReadESecretToKernel(userIdToFbe, status, classEBuffer, authToken, isFbeSupport);
    if (ret != E_OK) {
        LOGE("ReadESecret failed, user %{public}d, status: %{public}d", userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::WriteClassE(uint32_t status, uint8_t *classEBuffer, uint32_t length)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    // 0--single id, 1--double id
    UserIdToFbeStr userIdToFbe = { .userIds = { userId_, GetMappedUserId(userId_, type_) }, .size = USER_ID_SIZE };
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u",
         type_, userId_, userIdToFbe.userIds[DOUBLE_ID_INDEX]);
    auto ret = FBEX::WriteESecretToKernel(userIdToFbe, status, classEBuffer, length);
    if (ret != E_OK) {
        LOGE("WriteESecret failed,user %{public}d, status: %{public}d", userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
        return ret;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::InactiveKeyExt(uint32_t flag)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }

    LOGI("enter");
    bool destroy = !!flag;
#ifdef EL2_INACTIVE_SPECIAL_HANDLING
    if (!destroy) {
        LOGI("no need to inactive");
#else
    if ((type_ == TYPE_EL1) && !destroy) {
        LOGI("Is el1, no need to inactive");
#endif
        return E_OK;
    }
    uint8_t buf[FBEX_IV_SIZE] = {0};
    buf[0] = 0xfb; // fitst byte const to kernel
    buf[1] = 0x30; // second byte const to kernel

    // When double update single, el5 use single user id, like: 100  101  102 ...
    // el1-el4 use double id, like 0 10 11 12 ...
    uint32_t user = type_ == TYPE_EL5 ? userId_ : GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    int errNo = FBEX::UninstallOrLockUserKeyToKernel(user, type_, buf, FBEX_IV_SIZE, destroy);
    if (errNo != 0) {
        LOGE("UninstallOrLockUserKeyToKernel failed, userId %{public}d, type %{public}d, destroy %{public}u",
             userId_, type_, destroy);
        return errNo;
    }
    return E_OK;
}

int32_t FscryptKeyV1Ext::LockUserScreenExt(uint32_t flag, uint32_t &elType)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGI("enter");
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    int32_t ret = FBEX::LockScreenToKernel(user);
    if (ret != E_OK) {
        LOGE("LockScreenToKernel failed, userId %{public}d, error code: %{public}d", user, ret);
        return ret;
    }
    //Used to associate el3 and el4 kernels.
    elType = type_;
    return E_OK;
}

int32_t FscryptKeyV1Ext::LockUeceExt(bool &isFbeSupport)
{
    if (!FBEX::IsFBEXSupported()) {
        return E_OK;
    }
    LOGD("FscryptKeyV1Ext::LockUeceExt enter");
    uint32_t userIdDouble = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, userIdDouble);
    int32_t ret = FBEX::LockUece(userId_, userIdDouble, isFbeSupport);
    if (ret != E_OK) {
        LOGE("LockUeceExt failed, userId");
        return ret;
    }
    return E_OK;
}

uint32_t FscryptKeyV1Ext::GetUserIdFromDir()
{
    int userId = USERID_GLOBAL_EL1; // default to global el1

    // fscrypt key dir is like `/data/foo/bar/el1/100`
    auto slashIndex = dir_.rfind('/');
    if (slashIndex != std::string::npos) {
        std::string last = dir_.substr(slashIndex + 1);
        (void)OHOS::StrToInt(last, userId);
    }

    LOGI("dir_: %{public}s, get userId is %{public}d", dir_.c_str(), userId);
    return static_cast<uint32_t>(userId);
}

uint32_t FscryptKeyV1Ext::GetTypeFromDir()
{
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
        LOGE("bad dir %{public}s", dir_.c_str());
        return type;
    }

    if (slashIndex == 0) {
        LOGE("bad dir %{public}s", dir_.c_str());
        return type;
    }

    slashIndex = dir_.rfind('/', slashIndex - 1);
    if (slashIndex == std::string::npos) {
        LOGE("bad dir %{public}s", dir_.c_str());
        return type;
    }

    std::string el = dir_.substr(slashIndex + 1); // el string is like `el1/100`
    for (const auto &it : typeStrs) {
        if (el.find(it.first) != std::string::npos) {
            type = it.second;
            break;
        }
    }
    LOGI("el string is %{public}s, parse type %{public}d", el.c_str(), type);
    return type;
}
} // namespace StorageDaemon
} // namespace OHOS
