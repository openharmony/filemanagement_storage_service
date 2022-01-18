/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "base_key.h"

#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

#include "directory_ex.h"
#include "string_ex.h"
#include "file_ex.h"
#include "utils/log.h"
#include "huks_master.h"

namespace OHOS {
namespace StorageDaemon {
static bool g_isHuksMasterInit = false;
BaseKey::BaseKey(std::string dir, uint8_t keyLen) : dir_(dir), keyLen_(keyLen)
{
    if (!g_isHuksMasterInit && (HuksMaster::Init() == 0)) {
        g_isHuksMasterInit = true;
    }
}

bool BaseKey::InitKey()
{
    LOGD("enter");
    if (keyInfo_.version == FSCRYPT_INVALID || keyInfo_.version > KeyCtrl::GetFscryptVersion()) {
        LOGE("invalid version %{public}u", keyInfo_.version);
        return false;
    }
    if (!keyInfo_.key.IsEmpty()) {
        LOGE("key is not empty");
        return false;
    }
    if (!GenerateKeyBlob(keyInfo_.key, keyLen_)) {
        LOGE("GenerateKeyBlob raw key failed");
        return false;
    }
    return true;
}

bool BaseKey::GenerateKeyBlob(KeyBlob &blob, const uint32_t size)
{
    if (!blob.Alloc(size)) {
        return false;
    }
    if (!HuksMaster::GenerateRandomKey(blob)) {
        blob.Clear();
        return false;
    }
    return true;
}

bool BaseKey::SaveKeyBlob(const KeyBlob &blob, const std::string &name)
{
    LOGD("enter %{public}s, size=%{public}d", name.c_str(), blob.size);
    if (blob.IsEmpty()) {
        return false;
    }
    std::string path = dir_ + "/" + name;
    std::ofstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }
    file.write(reinterpret_cast<char *>(blob.data.get()), blob.size);
    file.flush();
    return true;
}

bool BaseKey::GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size)
{
    if (!GenerateKeyBlob(blob, size)) {
        return false;
    }
    return SaveKeyBlob(blob, name);
}

bool BaseKey::LoadKeyBlob(KeyBlob &blob, const std::string &name, const uint32_t size = 0)
{
    LOGD("enter %{public}s, size=%{public}d", name.c_str(), size);
    std::string path = dir_ + "/" + name;
    std::ifstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    uint32_t length = file.tellg();
    // zero size means use the file length.
    if ((size != 0) && (length != size)) {
        LOGE("file:%{public}s size error, real len %{public}d !=  expected %{public}d", name.c_str(), length, size);
        return false;
    }
    if (!blob.Alloc(length)) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char *>(blob.data.get()), length)) {
        LOGE("read fail"); // print what?
        return false;
    }
    return true;
}

bool BaseKey::StoreKey(const UserAuth &auth)
{
    LOGD("enter");
    std::string originalDir = dir_;
    dir_ += ".tmp";
    if (DoStoreKey(auth)) {
        /*
         * if originalDir
         * del original alias of key, and alias should be different.
         */
        OHOS::ForceRemoveDirectory(originalDir);
        if (rename(dir_.c_str(), originalDir.c_str()) != 0) {
            LOGE("rename fail return %{public}d", errno);
            OHOS::ForceRemoveDirectory(dir_);
            dir_ = originalDir;
            return false;
        }
    } else {
        OHOS::ForceRemoveDirectory(dir_);
    }
    dir_ = originalDir;
    return true;
}

bool BaseKey::DoStoreKey(const UserAuth &auth)
{
    OHOS::ForceCreateDirectory(dir_);
    OHOS::SaveStringToFile(dir_ + "/version", std::to_string(keyInfo_.version));
    if (!GenerateAndSaveKeyBlob(keyContext_.alias, "alias", CRYPTO_KEY_ALIAS_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob alias failed");
        return false;
    }
    if (!HuksMaster::GenerateKey(keyContext_.alias)) {
        LOGE("HuksMaster::GenerateKey failed");
        return false;
    }
    if (!GenerateAndSaveKeyBlob(keyContext_.secDiscard, "sec_discard", CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob sec_discard failed");
        return false;
    }
    if (!EncryptKey(auth)) {
        return false;
    }
    if (!SaveKeyBlob(keyContext_.encrypted, "encrypted")) {
        return false;
    }
    keyContext_.encrypted.Clear();
    return true;
}

bool BaseKey::EncryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::EncryptKey(keyContext_, auth, keyInfo_);
    keyContext_.alias.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    return ret;
}

bool BaseKey::RestoreKey(const UserAuth &auth)
{
    LOGD("enter");
    auto ver = KeyCtrl::LoadVersion(dir_);
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return false;
    }

    if (!LoadKeyBlob(keyContext_.encrypted, "encrypted")) {
        return false;
    }
    if (!LoadKeyBlob(keyContext_.alias, "alias", CRYPTO_KEY_ALIAS_SIZE)) {
        keyContext_.alias.Clear();
        return false;
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, "sec_discard", CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.encrypted.Clear();
        keyContext_.alias.Clear();
        return false;
    }
    return DecryptKey(auth);
}

bool BaseKey::DecryptKey(const UserAuth &auth)
{
    auto ret = HuksMaster::DecryptKey(keyContext_, auth, keyInfo_);
    keyContext_.encrypted.Clear();
    keyContext_.alias.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    return ret;
}

bool BaseKey::RemoveAlias()
{
    KeyBlob alias {};
    return LoadKeyBlob(alias, "alias", CRYPTO_KEY_ALIAS_SIZE) && HuksMaster::DeleteKey(alias);
}

bool BaseKey::ClearKey(const std::string &mnt)
{
    InactiveKey(mnt);
    return RemoveAlias() && OHOS::ForceRemoveDirectory(dir_);
    // use F2FS_IOC_SEC_TRIM_FILE
}

} // namespace StorageDaemon
} // namespace OHOS
