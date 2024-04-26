/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include <string>
#include <unistd.h>

#include "directory_ex.h"
#include "file_ex.h"
#include "huks_master.h"
#include "libfscrypt/key_control.h"
#include "openssl_crypto.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"

namespace {
const std::string PATH_LATEST_BACKUP = "/latest_bak";
const std::string PATH_KEY_VERSION = "/version_";
const std::string PATH_KEY_TEMP = "/temp";

#ifndef F2FS_IOCTL_MAGIC
#define F2FS_IOCTL_MAGIC 0xf5
#endif

#ifndef F2FS_IOC_SEC_TRIM_FILE
    struct F2fsSectrimRange {
        uint64_t start;
        uint64_t len;
        uint64_t flags;
    };
    using F2fsSectrim = F2fsSectrimRange;
#define F2FS_IOC_SEC_TRIM_FILE _IOW(F2FS_IOCTL_MAGIC, 20, F2fsSectrim)
#define F2FS_TRIM_FILE_DISCARD 0x1
#define F2FS_TRIM_FILE_ZEROOUT 0x2
#endif
#ifndef F2FS_IOC_SET_PIN_FILE
#define F2FS_IOC_SET_PIN_FILE _IOW(F2FS_IOCTL_MAGIC, 13, set)
#define F2FS_IOC_GET_PIN_FILE _IOR(F2FS_IOCTL_MAGIC, 14, set)
#endif
}

namespace OHOS {
namespace StorageDaemon {
BaseKey::BaseKey(const std::string &dir, uint8_t keyLen) : dir_(dir), keyLen_(keyLen)
{
}

bool BaseKey::InitKey()
{
    LOGD("enter");
    if (keyInfo_.version == FSCRYPT_INVALID || keyInfo_.version > KeyCtrlGetFscryptVersion(MNT_DATA.c_str())) {
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
    blob = HuksMaster::GenerateRandomKey(size);
    return !blob.IsEmpty();
}

bool BaseKey::SaveKeyBlob(const KeyBlob &blob, const std::string &path)
{
    if (blob.IsEmpty()) {
        return false;
    }
    LOGD("enter %{public}s, size=%{public}d", path.c_str(), blob.size);
    return WriteFileSync(path.c_str(), blob.data.get(), blob.size);
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
        LOGE("open %{public}s failed, errno %{public}d", path.c_str(), errno);
        return false;
    }

    file.seekg(0, std::ios::end);
    uint32_t length = static_cast<uint32_t>(file.tellg());
    // zero size means use the file length.
    if ((size != 0) && (length != size)) {
        LOGE("file:%{public}s size error, real len %{public}d not expected %{public}d", path.c_str(), length, size);
        return false;
    }
    if (!blob.Alloc(length)) {
        return false;
    }

    file.seekg(0, std::ios::beg);
    if (file.read(reinterpret_cast<char *>(blob.data.get()), length).fail()) {
        LOGE("read %{public}s failed, errno %{public}d", path.c_str(), errno);
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

#ifdef USER_CRYPTO_MIGRATE_KEY
bool BaseKey::StoreKey(const UserAuth &auth, bool needGenerateShield)
#else
bool BaseKey::StoreKey(const UserAuth &auth)
#endif
{
    LOGD("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (DoStoreKey(auth, needGenerateShield)) {
#else
    if (DoStoreKey(auth)) {
#endif
        // rename keypath/temp/ to keypath/version_xx/
        auto candidate = GetNextCandidateDir();
        LOGD("rename %{public}s to %{public}s", pathTemp.c_str(), candidate.c_str());
        if (rename(pathTemp.c_str(), candidate.c_str()) == 0) {
            SyncKeyDir();
            return true;
        }
        LOGE("rename fail return %{public}d, cleanup the temp dir", errno);
    } else {
        LOGE("DoStoreKey fail, cleanup the temp dir");
    }
    OHOS::ForceRemoveDirectory(pathTemp);
    SyncKeyDir();
    return false;
}

// All key files are saved under keypath/temp/ in this function.
#ifdef USER_CRYPTO_MIGRATE_KEY
bool BaseKey::DoStoreKey(const UserAuth &auth, bool needGenerateShield)
#else
bool BaseKey::DoStoreKey(const UserAuth &auth)
#endif
{
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    MkDirRecurse(pathTemp, S_IRWXU);
    const std::string NEED_UPDATE_PATH = pathTemp + SUFFIX_NEED_UPDATE;
    if (!CheckAndUpdateVersion()) {
        return false;
    }
    if ((auth.secret.IsEmpty()) && (!LoadAndSaveShield(auth, pathTemp, needGenerateShield))) {
        return false;
    }
    if (!GenerateAndSaveKeyBlob(keyContext_.secDiscard, pathTemp + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("GenerateAndSaveKeyBlob sec_discard failed");
        return false;
    }
    if (!Encrypt(auth)) {
        return false;
    }
    if (!SaveKeyBlob(keyContext_.encrypted, pathTemp + PATH_ENCRYPTED)) {
        return false;
    }
    if (!SaveStringToFile(NEED_UPDATE_PATH, KeyEncryptTypeToString(keyEncryptType_))) {
        LOGE("make file fali");
        return false;
    }
    keyContext_.encrypted.Clear();
    LOGD("finish");
    return true;
}

bool BaseKey::CheckAndUpdateVersion()
{
    auto pathVersion = dir_ + PATH_FSCRYPT_VER;
    std::string version;
    if (OHOS::LoadStringFromFile(pathVersion, version)) {
        if (version != std::to_string(keyInfo_.version)) {
            LOGE("version already exist %{public}s, not expected %{public}d", version.c_str(), keyInfo_.version);
            return false;
        }
    } else if (SaveStringToFileSync(pathVersion, std::to_string(keyInfo_.version)) == false) {
        LOGE("save version failed, errno:%{public}d", errno);
        return false;
    }
    ChMod(pathVersion, S_IREAD | S_IWRITE);
    return true;
}

bool BaseKey::LoadAndSaveShield(const UserAuth &auth, const std::string &pathTemp, bool needGenerateShield)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
        if (needGenerateShield) {
            if (!HuksMaster::GetInstance().GenerateKey(auth, keyContext_.shield)) {
                LOGE("GenerateKey of shield failed");
                return false;
            }
        } else {
            if (!LoadKeyBlob(keyContext_.shield, dir_ + PATH_LATEST + PATH_SHIELD)) {
                keyContext_.encrypted.Clear();
                return false;
            }
        }
#else
        if (!HuksMaster::GetInstance().GenerateKey(auth, keyContext_.shield)) {
            LOGE("GenerateKey of shield failed");
            return false;
        }
#endif
        if (!SaveKeyBlob(keyContext_.shield, pathTemp + PATH_SHIELD)) {
            return false;
        }
    return true;
}

// update the latest and do cleanups.
bool BaseKey::UpdateKey(const std::string &keypath)
{
    LOGD("enter");
    auto candidate = keypath.empty() ? GetCandidateDir() : keypath;
    if (candidate.empty()) {
        LOGE("no candidate dir");
        return false;
    }

    // backup the latest
    std::string pathLatest = dir_ + PATH_LATEST;
    std::string pathLatestBak = dir_ + PATH_LATEST_BACKUP;
    bool hasLatest = IsDir(dir_ + PATH_LATEST);
    if (hasLatest) {
        OHOS::ForceRemoveDirectory(pathLatestBak);
        if (rename(pathLatest.c_str(),
                   pathLatestBak.c_str()) != 0) {
            LOGE("backup the latest fail errno:%{public}d", errno);
        }
        LOGD("backup the latest success");
    }

    // rename {candidate} to latest
    OHOS::ForceRemoveDirectory(dir_ + PATH_LATEST);
    if (rename(candidate.c_str(), pathLatest.c_str()) != 0) {
        LOGE("rename candidate to latest fail return %{public}d", errno);
        if (hasLatest) {
            // revert from the backup
            if (rename(pathLatestBak.c_str(),
                       pathLatest.c_str()) != 0) {
                LOGE("restore the latest_backup fail errno:%{public}d", errno);
            } else {
                LOGI("restore the latest_backup success");
            }
        }
        SyncKeyDir();
        return false;
    }
    LOGD("rename candidate %{public}s to latest success", candidate.c_str());

    // cleanup backup and other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    for (const auto &it: files) {
        if (it != PATH_LATEST.substr(1)) {
            OHOS::ForceRemoveDirectory(dir_ + "/" + it);
        }
    }

    SyncKeyDir();
    return true;
}

bool BaseKey::Encrypt(const UserAuth &auth)
{
    LOGD("enter");
    bool ret;
    if (!auth.secret.IsEmpty()) {
        LOGI("Enhanced encrypt start");
        ret = OpensslCrypto::AESEncrypt(auth.secret, keyInfo_.key, keyContext_);
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_OPENSSL;
    } else {
        LOGI("Huks encrypt start");
        ret = HuksMaster::GetInstance().EncryptKey(keyContext_, auth, keyInfo_);
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS;
    }
    keyContext_.shield.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    LOGD("finish");
    return ret;
}

bool BaseKey::RestoreKey(const UserAuth &auth)
{
    LOGD("enter");
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        return DoRestoreKeyEx(auth, dir_ + PATH_LATEST);
    }

    if (DoRestoreKeyEx(auth, candidate)) {
        // update the latest with the candidate
        UpdateKey();
        return true;
    }

    LOGE("DoRestoreKey with %{public}s failed", candidate.c_str());
    // try to restore from other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    std::sort(files.begin(), files.end(), [&](const std::string &a, const std::string &b) {
        if (a.length() != b.length() ||
            a.length() < PATH_KEY_VERSION.length() ||
            b.length() < PATH_KEY_VERSION.length()) {
            return a.length() > b.length();
        }
        // make sure a.length() >= PATH_KEY_VERSION.length() && b.length() >= PATH_KEY_VERSION.length()
        return std::stoi(a.substr(PATH_KEY_VERSION.size() - 1)) > std::stoi(b.substr(PATH_KEY_VERSION.size() - 1));
    });
    for (const auto &it: files) {
        if (it != candidate) {
            if (DoRestoreKeyEx(auth, dir_ + "/" + it)) {
                UpdateKey(it);
                return true;
            }
        }
    }
    return false;
}

bool BaseKey::DoRestoreKeyEx(const UserAuth &auth, const std::string &keyPath)
{
    LOGD("enter restore key ex");
    if (!DoRestoreKey(auth, keyPath)) {
        LOGE("First restore failed !");
        return false;
    }
    if (keyEncryptType_ == KeyEncryptType::KEY_CRYPT_HUKS) {
        LOGE("Key encrypted by huks, skip !");
        return true;
    }
    if (!StoreKey(auth)) {
        LOGE("Store key failed !");
        return false;
    }
    if (!UpdateKey()) {
        LOGE("Update key context failed !");
        return false;
    }
    if (!DoRestoreKey(auth, keyPath)) {
        LOGE("Second restore failed !");
        return false;
    }
    return true;
}

bool BaseKey::DoRestoreKey(const UserAuth &auth, const std::string &path)
{
    LOGD("enter, path = %{public}s", path.c_str());
    const std::string NEED_UPDATE_PATH = dir_ + PATH_LATEST + SUFFIX_NEED_UPDATE;
    if (!auth.secret.IsEmpty() && FileExists(NEED_UPDATE_PATH)) {
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_OPENSSL;
        LOGI("set keyEncryptType_ as KEY_CRYPT_OPENSSL success");
    } else if (auth.secret.IsEmpty() || (!auth.secret.IsEmpty() && !FileExists(NEED_UPDATE_PATH))) {
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS;
        LOGI("set keyEncryptType_ as KEY_CRYPT_HUKS success");
    }
    auto ver = KeyCtrlLoadVersion(dir_.c_str());
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return false;
    }
    if (!LoadKeyBlob(keyContext_.encrypted, path + PATH_ENCRYPTED)) {
        return false;
    }
    if (keyEncryptType_ == KeyEncryptType::KEY_CRYPT_HUKS) {
        if (!LoadKeyBlob(keyContext_.shield, path + PATH_SHIELD)) {
            keyContext_.encrypted.Clear();
            return false;
        }
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.encrypted.Clear();
        keyContext_.shield.Clear();
        return false;
    }
    return Decrypt(auth);
}

bool BaseKey::Decrypt(const UserAuth &auth)
{
    bool ret = false;
    switch (keyEncryptType_) {
        case KeyEncryptType::KEY_CRYPT_OPENSSL:
            LOGI("Enhanced decrypt key start");
            ret = OpensslCrypto::AESDecrypt(auth.secret, keyContext_, keyInfo_.key);
            break;
        case KeyEncryptType::KEY_CRYPT_HUKS:
            LOGI("Huks decrypt key start");
            ret = HuksMaster::GetInstance().DecryptKey(keyContext_, auth, keyInfo_);
            break;
    }
    keyContext_.encrypted.Clear();
    keyContext_.shield.Clear();
    keyContext_.secDiscard.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
    return ret;
}

bool BaseKey::ClearKey(const std::string &mnt)
{
    LOGD("enter, dir_ = %{public}s", dir_.c_str());
    InactiveKey(USER_DESTROY, mnt);
    keyInfo_.key.Clear();
    WipingActionDir(dir_);
    return OHOS::ForceRemoveDirectory(dir_);
    // use F2FS_IOC_SEC_TRIM_FILE
}

void BaseKey::WipingActionDir(std::string &path)
{
    std::vector<std::string> fileList;
    LOGI("WipingActionDir path.c_str() is %{public}s", path.c_str());
    OpenSubFile(path.c_str(), fileList);
    for (const auto &it: fileList) {
        int fd = open(it.c_str(), O_WRONLY | O_CLOEXEC);
        if (fd < 0) {
            LOGE("open %{public}s failed, errno %{public}u", it.c_str(), errno);
            return;
        }
        uint32_t  set = 1;
        int ret = ioctl(fd, F2FS_IOC_SET_PIN_FILE, &set);
        if (ret != 0) {
            LOGE("F2FS_IOC_SET_PIN_FILE ioctl is %{public}u, errno = %{public}u", ret, errno);
        }
        struct F2fsSectrimRange trimRange;
        trimRange.start = 0;
        trimRange.len = -1;
        trimRange.flags = F2FS_TRIM_FILE_DISCARD | F2FS_TRIM_FILE_ZEROOUT;
        ret = ioctl(fd, F2FS_IOC_SEC_TRIM_FILE, &trimRange);
        if (ret != 0 && errno == EOPNOTSUPP) {
            trimRange.flags = F2FS_TRIM_FILE_ZEROOUT;
            ret = ioctl(fd, F2FS_IOC_SEC_TRIM_FILE, &trimRange);
            if (ret != 0) {
                LOGE("F2FS_IOC_SEC_TRIM_FILE ioctl is %{public}u, errno = %{public}u", ret, errno);
            }
        }
        set = 0;
        ret = ioctl(fd, F2FS_IOC_SET_PIN_FILE, &set);
        if (ret != 0) {
            LOGE("F2FS_IOC_SET_PIN_FILE ioctl is %{public}u", ret);
        }
        LOGI("WipingActionDir success");
        close(fd);
    }
}

void BaseKey::SyncKeyDir() const
{
    int fd = open(dir_.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        LOGE("open %{private}s failed, errno %{public}d", dir_.c_str(), errno);
        sync();
        return;
    }
    if (syncfs(fd) != 0) {
        LOGE("syncfs %{private}s failed, errno %{public}d", dir_.c_str(), errno);
        sync();
    }
    (void)close(fd);
}

bool BaseKey::UpgradeKeys()
{
    std::vector<std::string> versions;
    GetSubDirs(dir_, versions);

    for (const auto &it : versions) {
        std::string shieldPath = dir_ + "/" + it + PATH_SHIELD;
        LOGI("Upgrade of %{public}s", shieldPath.c_str());
        LoadKeyBlob(keyContext_.shield, shieldPath);
        if (HuksMaster::GetInstance().UpgradeKey(keyContext_)) {
            LOGI("success upgrade of %{public}s", shieldPath.c_str());
            SaveKeyBlob(keyContext_.shield, shieldPath);
            SyncKeyDir();
        }
    }
    return true;
}

std::string BaseKey::KeyEncryptTypeToString(KeyEncryptType keyEncryptType_) const
{
    switch (keyEncryptType_) {
        case KeyEncryptType::KEY_CRYPT_OPENSSL:
            return "KEY_CRYPT_OPENSSL";
        case KeyEncryptType::KEY_CRYPT_HUKS:
            return "KEY_CRYPT_HUKS";
    }
}
} // namespace StorageDaemon
} // namespace OHOS
