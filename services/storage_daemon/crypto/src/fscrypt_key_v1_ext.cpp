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

#include <vector>

#include "fbex.h"
#include "storage_service_log.h"
#include "string_ex.h"

namespace OHOS {
namespace StorageDaemon {
bool FscryptKeyV1Ext::ActiveKeyExt(uint32_t flag, uint8_t *iv, uint32_t size)
{
    if (!FBEX::IsFBEXSupported()) {
        return true;
    }

    LOGD("enter");
    // iv buffer returns derived keys
    if (FBEX::InstallKeyToKernel(userId_, type_, iv, size, static_cast<uint8_t>(flag)) != 0) {
        LOGE("InstallKeyToKernel failed, userId %{public}d, type %{public}d, flag %{public}u", userId_, type_, flag);
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

    if (FBEX::UninstallOrLockUserKeyToKernel(userId_, type_, buf, FBEX_IV_SIZE, destroy) != 0) {
        LOGE("UninstallOrLockUserKeyToKernel failed, userId %{public}d, type %{public}d, destroy %{public}u", userId_,
             type_, destroy);
        return false;
    }
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