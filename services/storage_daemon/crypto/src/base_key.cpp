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

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "directory_ex.h"
#include "file_ex.h"
#include "huks_master.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/file_utils.h"

namespace {
const std::string PATH_LATEST_BACKUP = "/latest_bak";
const std::string PATH_KEY_VERSION = "/version_";
const std::string PATH_KEY_TEMP = "/temp";
}

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
    if (keyInfo_.version == FSCRYPT_INVALID || keyInfo_.version > KeyCtrl::GetFscryptVersion(MNT_DATA)) {
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

bool BaseKey::SaveKeyBlob(const KeyBlob &blob, const std::string &path)
{
    if (blob.IsEmpty()) {
        return false;
    }
    LOGD("enter %{public}s, size=%{public}d", path.c_str(), blob.size);
    std::ofstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }
    file.write(reinterpret_cast<char *>(blob.data.get()), blob.size);
    file.flush();
    return true;
}

bool BaseKey::GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size)
{
    if (!GenerateKeyBlob(blob, size)) {
        return false;
    }
    return SaveKeyBlob(blob, path);
}

bool BaseKey::LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size = 0)
{
    LOGD("enter %{public}s, size=%{public}d", path.c_str(), size);
    std::ifstream file(path, std::ios::binary);
    if (file.fail()) {
        LOGE("path:%{public}s fail", path.c_str());
        return false;
    }

    file.seekg(0, std::ios::end);
    uint32_t length = file.tellg();
    // zero size means use the file length.
    if ((size != 0) && (length != size)) {
        LOGE("file:%{public}s size error, real len %{public}d not expected %{public}d", path.c_str(), length, size);
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

int BaseKey::GetCandidateVersion() const
{
    auto prefix = PATH_KEY_VERSION.substr(1); // skip the first slash
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    int candidate = -1;
    for (const auto &it: files) {
        if (it.rfind(prefix) == 0) {
            std::string str = it.substr(prefix.length());
            int ver;
            if (IsNumericStr(str) && StrToInt(str, ver) && ver >= candidate) {
                candidate = ver;
            }
        }
    }
    LOGD("candidate key version is %{public}d", candidate);
    return candidate;
}

// Get last version_xx dir to load key files.
std::string BaseKey::GetCandidateDir() const
{
    auto candidate = GetCandidateVersion();
    // candidate is -1 means no version_xx dir.
    if (candidate == -1) {
        return "";
    }

    return dir_ + PATH_KEY_VERSION + std::to_string(candidate);
}

// Get next available version_xx dir to save key files.
std::string BaseKey::GetNextCandidateDir() const
{
    auto candidate = GetCandidateVersion();
    return dir_ + PATH_KEY_VERSION + std::to_string(candidate + 1);
}

bool BaseKey::StoreKey(const UserAuth &auth)
{
    LOGD("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    if (DoStoreKey(auth)) {
        // rename keypath/temp/ to keypath/version_xx/
        auto candidate = GetNextCandidateDir();
        LOGD("rename %{public}s to %{public}s", pathTemp.c_str(), candidate.c_str());
        if (rename(pathTemp.c_str(), candidate.c_str()) == 0) {
            return true;
        }
        LOGE("rename fail return %{public}d, cleanup the temp dir", errno);
    } else {
        LOGE("DoStoreKey fail, cleanup the temp dir");
    }
    RemoveAlias(pathTemp);
    OHOS::ForceRemoveDirectory(pathTemp);
    return false;
}

// All key files are saved under keypath/temp/ in this function.
bool BaseKey::DoStoreKey(const UserAuth &auth)
{
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    OHOS::ForceCreateDirectory(pathTemp);

    auto pathVersion = dir_ + PATH_FSCRYPT_VER;
    std::string version;
    if (OHOS::LoadStringFromFile(pathVersion, version) && version != std::to_string(keyInfo_.version)) {
        LOGE("version already exist %{public}s, not expected %{public}d", version.c_str(), keyInfo_.version);
        return false;
    }
    if (OHOS::SaveStringToFile(pathVersion, std::to_string(keyInfo_.version)) == false) {
        LOGE("save version fail");
        return false;
    }

    if (!GenerateAndSaveKeyBlob(keyContext_.alias, pathTemp + PATH_ALIAS, CRYPTO_KEY_ALIAS_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob alias failed");
        return false;
    }
    if (!HuksMaster::GenerateKey(keyContext_.alias)) {
        LOGE("HuksMaster::GenerateKey failed");
        return false;
    }
    if (!GenerateAndSaveKeyBlob(keyContext_.secDiscard, pathTemp + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob sec_discard failed");
        return false;
    }
    if (!EncryptKey(auth)) {
        return false;
    }
    if (!SaveKeyBlob(keyContext_.encrypted, pathTemp + PATH_ENCRYPTED)) {
        return false;
    }
    keyContext_.encrypted.Clear();
    return true;
}

// update the latest and cleanup the version_xx.
bool BaseKey::UpdateKey(const std::string &keypath)
{
    LOGD("enter");
    auto candidate = keypath.empty() ? GetCandidateDir() : keypath;
    if (candidate.empty()) {
        LOGE("no candidate dir");
        return false;
    }

    // rename latest -> latest.bak
    bool hasLatest = IsDir(dir_ + PATH_LATEST);
    if (hasLatest) {
        OHOS::ForceRemoveDirectory(dir_ + PATH_LATEST_BACKUP);
        if (rename(std::string(dir_ + PATH_LATEST).c_str(),
                   std::string(dir_ + PATH_LATEST_BACKUP).c_str()) != 0) {
            LOGE("backup the latest fail errno:%{public}d", errno);
        }
        LOGD("backup the latest success");
    }

    // rename {candidate} -> latest
    OHOS::ForceRemoveDirectory(dir_ + PATH_LATEST);
    if (rename(candidate.c_str(), std::string(dir_ + PATH_LATEST).c_str()) != 0) {
        LOGE("rename candidate to latest fail return %{public}d", errno);
        if (hasLatest) {
            // rename latest.bak -> latest
            if (rename(std::string(dir_ + PATH_LATEST_BACKUP).c_str(),
                       std::string(dir_ + PATH_LATEST).c_str()) != 0) {
                LOGE("restore the latest_backup fail errno:%{public}d", errno);
            } else {
                LOGI("restore the latest_backup success");
            }
        }
        return false;
    }
    LOGD("rename candidate %{public}s to latest success", candidate.c_str());

    // cleanup latest.bak and version_*
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    for (const auto &it: files) {
        if (it != PATH_LATEST.substr(1)) {
            RemoveAlias(dir_ + "/" + it);
            OHOS::ForceRemoveDirectory(dir_ + "/" + it);
        }
    }

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
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        return DoRestoreKey(auth, dir_ + PATH_LATEST);
    }

    if (DoRestoreKey(auth, candidate)) {
        // update the latest with the candidate
        UpdateKey();
        return true;
    } else {
        LOGE("DoRestoreKey with %{public}s failed", candidate.c_str());
        // try to restore from other versions
        std::vector<std::string> files;
        GetSubDirs(dir_, files);
        // sort descreasely
        std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b) {
            return a > b;
        });
        for (const auto &it: files) {
            if (it != candidate) {
                if (DoRestoreKey(auth, dir_ + "/" + it)) {
                    UpdateKey(it);
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

bool BaseKey::DoRestoreKey(const UserAuth &auth, const std::string &path)
{
    LOGD("enter");
    auto ver = KeyCtrl::LoadVersion(dir_);
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return false;
    }

    if (!LoadKeyBlob(keyContext_.encrypted, path + PATH_ENCRYPTED)) {
        return false;
    }
    if (!LoadKeyBlob(keyContext_.alias, path + PATH_ALIAS, CRYPTO_KEY_ALIAS_SIZE)) {
        keyContext_.alias.Clear();
        return false;
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
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

bool BaseKey::RemoveAlias(const std::string &keypath)
{
    KeyBlob alias {};
    return LoadKeyBlob(alias, keypath + PATH_ALIAS, CRYPTO_KEY_ALIAS_SIZE) &&
           HuksMaster::DeleteKey(alias);
}

bool BaseKey::ClearKey(const std::string &mnt)
{
    InactiveKey(mnt);

    // remove all key alias of all versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    for (const auto &it : files) {
        RemoveAlias(dir_ + "/" + it);
    }

    return OHOS::ForceRemoveDirectory(dir_);
    // use F2FS_IOC_SEC_TRIM_FILE
}

} // namespace StorageDaemon
} // namespace OHOS
