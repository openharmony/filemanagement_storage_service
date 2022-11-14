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

#include "fscrypt_key_v2.h"

#include "libfscrypt/key_control.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
#ifdef SUPPORT_FSCRYPT_V2
bool FscryptKeyV2::ActiveKey(uint32_t flag, const std::string &mnt)
{
    LOGD("enter");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("rawkey is null");
        return false;
    }

    auto buf = std::make_unique<char[]>(sizeof(fscrypt_add_key_arg) + FSCRYPT_MAX_KEY_SIZE);
    auto arg = reinterpret_cast<fscrypt_add_key_arg *>(buf.get());
    (void)memset_s(arg, sizeof(fscrypt_add_key_arg) + FSCRYPT_MAX_KEY_SIZE, 0, sizeof(fscrypt_add_key_arg) +
        FSCRYPT_MAX_KEY_SIZE);
    arg->key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
    arg->raw_size = keyInfo_.key.size;
    auto err = memcpy_s(arg->raw, FSCRYPT_MAX_KEY_SIZE, keyInfo_.key.data.get(), keyInfo_.key.size);
    if (err != EOK) {
        LOGE("memcpy failed ret %{public}d", err);
        return false;
    }

    if (!KeyCtrlInstallKey(mnt.c_str(), arg)) {
        LOGE("InstallKey failed");
        return false;
    }
    keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE);
    auto ret = memcpy_s(keyInfo_.keyId.data.get(), keyInfo_.keyId.size, arg->key_spec.u.identifier,
        FSCRYPT_KEY_IDENTIFIER_SIZE);
    if (ret != EOK) {
        LOGE("memcpy_s failed ret %{public}d", ret);
        return false;
    }

    LOGD("success. key_id len:%{public}d, data(hex):%{private}s", keyInfo_.keyId.size,
        keyInfo_.keyId.ToString().c_str());
    if (!SaveKeyBlob(keyInfo_.keyId, dir_ + PATH_KEYID)) {
        return false;
    }
    keyInfo_.key.Clear();
    LOGD("success");
    return true;
}

bool FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    LOGD("enter");
    if (keyInfo_.keyId.size != FSCRYPT_KEY_IDENTIFIER_SIZE) {
        LOGE("keyId is invalid, %{public}u", keyInfo_.keyId.size);
        return false;
    }

    fscrypt_remove_key_arg arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
    auto ret = memcpy_s(arg.key_spec.u.identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, keyInfo_.keyId.data.get(),
        keyInfo_.keyId.size);
    if (ret != EOK) {
        LOGE("memcpy_s failed ret %{public}d", ret);
        return false;
    }

    if (!KeyCtrlRemoveKey(mnt.c_str(), &arg)) {
        return false;
    }
    if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS) {
        LOGE("Other users still have this key added");
    } else if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY) {
        LOGE("Some files using this key are still in-use");
    }

    LOGD("success");
    keyInfo_.keyId.Clear();
    return true;
}

#else
bool FscryptKeyV2::ActiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return false;
}

bool FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return false;
}
#endif
} // namespace StorageDaemon
} // namespace OHOS
