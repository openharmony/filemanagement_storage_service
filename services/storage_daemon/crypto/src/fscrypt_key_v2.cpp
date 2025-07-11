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

#include "fscrypt_key_v2.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
#ifdef SUPPORT_FSCRYPT_V2
int32_t FscryptKeyV2::ActiveKey(const KeyBlob &authToken, uint32_t flag, const std::string &mnt)
{
    LOGI("enter");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("rawkey is null");
        return E_KEY_EMPTY_ERROR;
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
        return err;
    }

    if (!KeyCtrlInstallKey(mnt.c_str(), arg)) {
        LOGE("InstallKey failed");
        return E_KEY_CTRL_INSTALL_ERROR;
    }
    keyInfo_.keyId.Alloc(FSCRYPT_KEY_IDENTIFIER_SIZE);
    auto ret = memcpy_s(keyInfo_.keyId.data.get(), keyInfo_.keyId.size, arg->key_spec.u.identifier,
        FSCRYPT_KEY_IDENTIFIER_SIZE);
    if (ret != EOK) {
        LOGE("memcpy_s failed ret %{public}d", ret);
        return ret;
    }
    (void)memset_s(arg, sizeof(fscrypt_add_key_arg) + FSCRYPT_MAX_KEY_SIZE, 0, sizeof(fscrypt_add_key_arg) +
        FSCRYPT_MAX_KEY_SIZE);

    LOGI("success. key_id len:%{public}d, data(hex):%{private}s", keyInfo_.keyId.size,
        keyInfo_.keyId.ToString().c_str());
    if (!SaveKeyBlob(keyInfo_.keyId, dir_ + PATH_KEYID)) {
        return E_SAVE_KEY_BLOB_ERROR;
    }
    keyInfo_.key.Clear();
    LOGI("success");
    return E_OK;
}

int32_t FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    LOGI("enter");
    if (keyInfo_.keyId.size == 0) {
        LOGE("keyId size is 0");
        return E_OK;
    }
    if (keyInfo_.keyId.size != FSCRYPT_KEY_IDENTIFIER_SIZE) {
        LOGE("keyId is invalid, %{public}u", keyInfo_.keyId.size);
        return E_INVAILd_KEY_ID_ERROR;
    }

    fscrypt_remove_key_arg arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.key_spec.type = FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER;
    auto ret = memcpy_s(arg.key_spec.u.identifier, FSCRYPT_KEY_IDENTIFIER_SIZE, keyInfo_.keyId.data.get(),
        keyInfo_.keyId.size);
    if (ret != EOK) {
        LOGE("memcpy_s failed ret %{public}d", ret);
        return ret;
    }

    if (!KeyCtrlRemoveKey(mnt.c_str(), &arg)) {
        return E_REMOVE_KEY_ERROR;
    }
    if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS) {
        LOGE("Other users still have this key added");
    } else if (arg.removal_status_flags & FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY) {
        LOGE("Some files using this key are still in-use");
    }

    LOGI("success");
    keyInfo_.keyId.Clear();
    return E_OK;
}

#else
int32_t FscryptKeyV2::ActiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return E_PARAMS_INVALID;
}

int32_t FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return E_PARAMS_INVALID;
}
#endif

int32_t FscryptKeyV2::LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::LockUece(bool &isFbeSupport)
{
    isFbeSupport = false;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::UnlockUserScreen(const KeyBlob &authToken, uint32_t flag,
    uint32_t sdpClass, const std::string &mnt)
{
    (void)mnt;
    (void)flag;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId)
{
    LOGI("Unsupported fscrypt v2");
    return E_NOT_SUPPORT;
}

int32_t FscryptKeyV2::DeleteAppkey(const std::string keyId)
{
    LOGI("Unsupported fscrypt v2");
    return E_NOT_SUPPORT;
}

int32_t FscryptKeyV2::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    (void)isNeedEncryptClassE;
    (void)status;
    (void)isSupport;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::DeleteClassEPinCode(uint32_t user)
{
    (void)user;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId)
{
    (void)userId;
    isFbeSupport = false;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::UpdateClassEBackUp(uint32_t userId)
{
    (void)userId;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::DecryptClassE(const UserAuth &auth, bool &isSupport,
                                    bool &eBufferStatue, uint32_t user, bool needSyncCandidate)
{
    (void)auth;
    (void)user;
    (void)needSyncCandidate;
    isSupport = false;
    eBufferStatue = false;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}

int32_t FscryptKeyV2::EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status)
{
    (void)auth;
    (void)user;
    (void)status;
    isSupport = false;
    LOGI("Unsupported fscrypt v2");
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
