/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <vector>

#include "fbex.h"
#include "storage_service_log.h"
#include "string_ex.h"

namespace OHOS {
namespace StorageDaemon {
static const std::string NEED_RESTORE_PATH = "/data/service/el0/storage_daemon/sd/latest/need_restore";
static const uint32_t DEFAULT_SINGLE_FIRST_USER_ID = 100;
static const uint32_t USER_ID_DIFF = 91;

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
    if (std::filesystem::exists(NEED_RESTORE_PATH) &&
        (type == TYPE_EL2 || type == TYPE_EL3 || type == TYPE_EL4)) {
        if (userId == DEFAULT_SINGLE_FIRST_USER_ID) {
            return 0;
        }

        if (userId > DEFAULT_SINGLE_FIRST_USER_ID) {
            return userId - USER_ID_DIFF;
        }
    }

    return userId;
}

bool FscryptKeyV1Ext::ActiveKeyExt(uint32_t flag, uint8_t *iv, uint32_t size, uint32_t &elType)
{
    if (!FBEX::IsFBEXSupported()) {
        return true;
    }

    LOGD("enter");
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    // iv buffer returns derived keys
    if (FBEX::InstallKeyToKernel(user, type_, iv, size, static_cast<uint8_t>(flag)) != 0) {
        LOGE("InstallKeyToKernel failed, user %{public}d, type %{public}d, flag %{public}u", user, type_, flag);
        return false;
    }

    //Used to associate el3 and el4 kernels.
    elType = type_;
    return true;
}

bool FscryptKeyV1Ext::UnlockUserScreenExt(uint32_t flag, uint8_t *iv, uint32_t size)
{
    if (!FBEX::IsFBEXSupported()) {
        return true;
    }
    LOGD("enter");
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    if (FBEX::UnlockScreenToKernel(user, type_, iv, size)) {
        LOGE("UnlockScreenToKernel failed, userId %{public}d, %{public}d", userId_, flag);
        return false;
    }
    return true;
}

bool FscryptKeyV1Ext::InactiveKeyExt(uint32_t flag)
{
    if (!FBEX::IsFBEXSupported()) {
        return true;
    }

    LOGD("enter");
    bool destroy = !!flag;
    if ((type_ != TYPE_EL2) && !destroy) {
        LOGD("not el2, no need to inactive");
        return true;
    }
    uint8_t buf[FBEX_IV_SIZE] = {0};
    buf[0] = 0xfb; // fitst byte const to kernel
    buf[1] = 0x30; // second byte const to kernel

    uint32_t user = GetMappedUserId(userId_, type_);
    LOGI("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    if (FBEX::UninstallOrLockUserKeyToKernel(user, type_, buf, FBEX_IV_SIZE, destroy) != 0) {
        LOGE("UninstallOrLockUserKeyToKernel failed, userId %{public}d, type %{public}d, destroy %{public}u", userId_,
             type_, destroy);
        return false;
    }
    return true;
}

bool FscryptKeyV1Ext::LockUserScreenExt(uint32_t flag, uint32_t &elType)
{
    if (!FBEX::IsFBEXSupported()) {
        return true;
    }
    LOGD("enter");
    uint32_t user = GetMappedUserId(userId_, type_);
    LOGD("type_ is %{public}u, map userId %{public}u to %{public}u", type_, userId_, user);
    if (FBEX::LockScreenToKernel(user)) {
        LOGE("LockScreenToKernel failed, userId %{public}d", flag);
        return false;
    }
    //Used to associate el3 and el4 kernels.
    elType = type_;
    return true;
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
    };
    uint32_t type = TYPE_GLOBAL_EL1; // default to global el1

    // fscrypt key dir is like `/data/foo/bar/el1/100`
    auto slashIndex = dir_.rfind('/');
    if (slashIndex == std::string::npos) {
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
