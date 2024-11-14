/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
#include <cstdio>
#include <string>
#include <unistd.h>
#include <dirent.h>

#include "directory_ex.h"
#include "fbex.h"
#include "file_ex.h"
#include "huks_master.h"
#include "iam_client.h"
#include "key_backup.h"
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
const std::string PATH_NEED_RESTORE_SUFFIX = "/latest/need_restore";
const std::string PATH_USER_EL1_DIR = "/data/service/el1/public/storage_daemon/sd/el1/";

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
BaseKey::BaseKey(const std::string &dir, uint8_t keyLen) : dir_(dir), keyLen_(keyLen),
    keyEncryptType_(KeyEncryptType::KEY_CRYPT_HUKS)
{
}

static void DoTempStore(const KeyContext &sourceCtx, KeyContext &targetCtx)
{
    LOGI("Store huks result temporary.");
    KeyBlob tempAad(sourceCtx.aad);
    KeyBlob tempNonce(sourceCtx.nonce);
    KeyBlob tempRndEnc(sourceCtx.rndEnc);
    KeyBlob tempShield(sourceCtx.shield);
    targetCtx.aad = std::move(tempAad);
    targetCtx.nonce = std::move(tempNonce);
    targetCtx.rndEnc = std::move(tempRndEnc);
    targetCtx.shield = std::move(tempShield);
}

bool BaseKey::InitKey(bool needGenerateKey)
{
    LOGI("enter");
    if (keyInfo_.version == FSCRYPT_INVALID || keyInfo_.version > KeyCtrlGetFscryptVersion(MNT_DATA.c_str())) {
        LOGE("invalid version %{public}u", keyInfo_.version);
        return false;
    }
    if (!keyInfo_.key.IsEmpty()) {
        LOGE("key is not empty");
        return false;
    }
    if (needGenerateKey && !GenerateKeyBlob(keyInfo_.key, keyLen_)) {
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
        LOGE("blob is empty");
        return false;
    }
    LOGI("enter %{public}s, size=%{public}d", path.c_str(), blob.size);
    return WriteFileSync(path.c_str(), blob.data.get(), blob.size);
}

bool BaseKey::GenerateAndSaveKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size)
{
    if (!GenerateKeyBlob(blob, size)) {
        return false;
    }
    return SaveKeyBlob(blob, path);
}

bool BaseKey::LoadKeyBlob(KeyBlob &blob, const std::string &path, const uint32_t size)
{
    LOGI("enter %{public}s, size=%{public}d", path.c_str(), size);
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
    LOGI("candidate key version is %{public}d", candidate);
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
    LOGI("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (DoStoreKey(auth, needGenerateShield)) {
#else
    if (DoStoreKey(auth)) {
#endif
        // rename keypath/temp/ to keypath/version_xx/
        auto candidate = GetNextCandidateDir();
        LOGI("rename %{public}s to %{public}s", pathTemp.c_str(), candidate.c_str());
        if (rename(pathTemp.c_str(), candidate.c_str()) == 0) {
            LOGI("start sync");
            SyncKeyDir();
            LOGI("sync end");
            return true;
        }
        LOGE("rename fail return %{public}d, cleanup the temp dir", errno);
    } else {
        LOGE("DoStoreKey fail, cleanup the temp dir");
    }
    OHOS::ForceRemoveDirectory(pathTemp);
    LOGI("start sync");
    SyncKeyDir();
    LOGI("sync end");
    return false;
}

// All key files are saved under keypath/temp/ in this function.
#ifdef USER_CRYPTO_MIGRATE_KEY
bool BaseKey::DoStoreKey(const UserAuth &auth, bool needGenerateShield)
#else
bool BaseKey::DoStoreKey(const UserAuth &auth)
#endif
{
    LOGI("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    if (!MkDirRecurse(pathTemp, S_IRWXU)) {
        LOGE("MkDirRecurse failed!");
    }
    if (!CheckAndUpdateVersion()) {
        return false;
    }
    uint32_t keyType = GetTypeFromDir();
    if (keyType == TYPE_EL1 || keyType == TYPE_GLOBAL_EL1) {
        return EncryptDe(auth, pathTemp);
    }
    if ((auth.token.IsEmpty() && auth.secret.IsEmpty()) || // OOBE首次开机  删除密码(ABC)
        !auth.token.IsEmpty()) {  // 新增密码  修改密码(ABC)
        LOGI("Encrypt huks openssl.");
        KeyContext keyCtx = {};
        if (!InitKeyContext(auth, pathTemp, keyCtx)) {
            LOGE("init key context failed !");
            return false;
        }

        if (!EncryptEceSece(auth, keyType, keyCtx)) {
            LOGE("Encrypt key failed !");
            ClearKeyContext(keyCtx);
            return false;
        }
        // save key buff nonce+rndEnc+aad
        if (!SaveAndCleanKeyBuff(pathTemp, keyCtx)) {
            LOGE("save key buff failed !");
            return false;
        }
    }
    LOGI("finish");
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

bool BaseKey::InitKeyContext(const UserAuth &auth, const std::string &keyPath, KeyContext &keyCtx)
{
    LOGI("enter");
    if (!LoadAndSaveShield(auth, keyPath + PATH_SHIELD, true, keyCtx)) {
        LOGE("Load or save shield failed !");
        return false;
    }

    if (!GenerateAndSaveKeyBlob(keyCtx.secDiscard, keyPath + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("Generate sec_discard failed");
        return false;
    }

    if (!GenerateKeyBlob(keyCtx.nonce, GCM_NONCE_BYTES) ||
        !GenerateKeyBlob(keyCtx.aad, GCM_MAC_BYTES)) {
        LOGE("Generate nonce and aad failed !");
        return false;
    }
    return true;
}

bool BaseKey::SaveAndCleanKeyBuff(const std::string &keyPath, KeyContext &keyCtx)
{
    KeyBlob storeKey(keyCtx.nonce.size + keyCtx.rndEnc.size + keyCtx.aad.size);
    if (!CombKeyCtx(keyCtx.nonce, keyCtx.rndEnc, keyCtx.aad, storeKey)) {
        LOGE("CombKeyCtx failed");
        return false;
    }

    if (!SaveKeyBlob(storeKey, keyPath + PATH_ENCRYPTED)) {
        return false;
    }

    const std::string NEED_UPDATE_PATH = keyPath + SUFFIX_NEED_UPDATE;
    if (!SaveStringToFile(NEED_UPDATE_PATH, KeyEncryptTypeToString(keyEncryptType_))) {
        LOGE("Save key type file failed");
        return false;
    }

    storeKey.Clear();
    ClearKeyContext(keyCtx);
    return true;
}

bool BaseKey::LoadAndSaveShield(const UserAuth &auth, const std::string &pathShield,
                                bool needGenerateShield, KeyContext &keyCtx)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (needGenerateShield) {
        if (!HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield)) {
            LOGE("GenerateKey of shield failed");
            return false;
        }
    } else {
        if (!LoadKeyBlob(keyCtx.shield, dir_ + PATH_LATEST + PATH_SHIELD)) {  // needcheck is update
            keyCtx.rndEnc.Clear();
            return false;
        }
    }
#else
    if (!HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield)) {
        LOGE("GenerateKey of shield failed");
        return false;
    }
#endif
    if (!SaveKeyBlob(keyCtx.shield,  pathShield)) {
        return false;
    }
    return true;
}

// update the latest and do cleanups.
bool BaseKey::UpdateKey(const std::string &keypath)
{
    LOGI("enter");
    auto candidate = keypath.empty() ? GetCandidateDir() : keypath;
    if (candidate.empty() && GetTypeFromDir() == TYPE_EL5) {
        LOGI("no uece candidate dir, do not need updateKey.");
        return true;
    }
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
        LOGI("backup the latest success");
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
    LOGI("rename candidate %{public}s to latest success", candidate.c_str());

    // cleanup backup and other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    for (const auto &it: files) {
        if (it != PATH_LATEST.substr(1)) {
            OHOS::ForceRemoveDirectory(dir_ + "/" + it);
        }
    }

    std::string backupDir;
    KeyBackup::GetInstance().GetBackupDir(dir_, backupDir);
    KeyBackup::GetInstance().CreateBackup(dir_, backupDir, true);

    SyncKeyDir();
    return true;
}

// 针对De只通过Huks加密
bool BaseKey::EncryptDe(const UserAuth &auth, const std::string &path)
{
    LOGI("enter");
    KeyContext ctxDe;
    if (!InitKeyContext(auth, path, ctxDe)) {
        LOGE("init key context failed !");
        return false;
    }
    keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS;
    if (!HuksMaster::GetInstance().EncryptKey(ctxDe, auth, keyInfo_, true)) {
        LOGE("Encrypt by hks failed.");
        ClearKeyContext(ctxDe);
        return false;
    }
    if (!SaveKeyBlob(ctxDe.rndEnc, path + PATH_ENCRYPTED)) {
        LOGE("SaveKeyBlob rndEnc failed.");
        ClearKeyContext(ctxDe);
        return false;
    }
    const std::string NEED_UPDATE_PATH = path + SUFFIX_NEED_UPDATE;
    if (!SaveStringToFile(NEED_UPDATE_PATH, KeyEncryptTypeToString(keyEncryptType_))) {
        LOGE("Save key type file failed");
        return false;
    }
    LOGI("finish");
    return true;
}

// 不针对RND_ENC 会将传入的keyCtx 加密  HKS->OpenSSL
bool BaseKey::EncryptEceSece(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx)
{
    LOGI("enter");
    // rnd 64 -> rndEnc 80
    if (!HuksMaster::GetInstance().EncryptKeyEx(auth, keyInfo_.key, keyCtx)) {
        LOGE("Encrypt by hks failed.");
        return false;
    }
    LOGI("Huks encrypt end.");

    UserAuth mUserAuth = auth;
    if (auth.secret.IsEmpty()) {
        mUserAuth.secret = KeyBlob(NULL_SECRET);
    }

    if (keyType == TYPE_EL3 || keyType == TYPE_EL4) {
        DoTempStore(keyCtx, keyContext_);
    }

    KeyBlob rndEnc(keyCtx.rndEnc);
    LOGI("Encrypt by openssl start"); // rndEnc 80 -> rndEncEnc 108
    if (!OpensslCrypto::AESEncrypt(mUserAuth.secret, rndEnc, keyCtx)) {
        LOGE("Encrypt by openssl failed.");
        return false;
    }
    LOGI("Encrypt by openssl end");
    rndEnc.Clear();
    keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL;
    LOGI("finish");
    return true;
}

bool BaseKey::RestoreKey(const UserAuth &auth)
{
    LOGI("enter");
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        return KeyBackup::GetInstance().TryRestoreKey(shared_from_this(), auth) == 0;
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
    LOGI("enter restore key ex");
    if (!DoRestoreKey(auth, keyPath)) {
        LOGE("First restore failed !");
        return false;
    }
    if (keyEncryptType_ == KeyEncryptType::KEY_CRYPT_HUKS) {
        LOGE("Key encrypted by huks, skip !");
        return true;
    }

    KeyBlob tempEnc(keyContext_.rndEnc.size);
    if (!LoadKeyBlob(tempEnc, keyPath + PATH_ENCRYPTED)) {
        LOGE("key encrypted by huks, skip !");
        return true;
    }

    uint32_t ivSum = 0;
    for (size_t i = 0; i < GCM_NONCE_BYTES; ++i) {
        ivSum += tempEnc.data[i];
    }
    if (ivSum != 0) {
        LOGE("key already update, skip !");
        tempEnc.Clear();
        return true;
    }
    tempEnc.Clear();
    
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

bool BaseKey::DoRestoreKeyOld(const UserAuth &auth, const std::string &path)
{
    LOGI("enter, path = %{public}s", path.c_str());
    const std::string NEED_UPDATE_PATH = dir_ + PATH_LATEST + SUFFIX_NEED_UPDATE;
    if (!auth.secret.IsEmpty() && FileExists(NEED_UPDATE_PATH)) {
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_OPENSSL;
        LOGI("set keyEncryptType_ as KEY_CRYPT_OPENSSL success");
    } else {
        keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS;
        LOGI("set keyEncryptType_ as KEY_CRYPT_HUKS success");
    }
    auto ver = KeyCtrlLoadVersion(dir_.c_str());
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return false;
    }
    if (!LoadKeyBlob(keyContext_.rndEnc, path + PATH_ENCRYPTED)) {
        return false;
    }
    if (keyEncryptType_ == KeyEncryptType::KEY_CRYPT_HUKS) {
        if (!LoadKeyBlob(keyContext_.shield, path + PATH_SHIELD)) {
            keyContext_.rndEnc.Clear();
            return false;
        }
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.rndEnc.Clear();
        keyContext_.shield.Clear();
        return false;
    }
    return Decrypt(auth);
}

bool BaseKey::DoRestoreKeyDe(const UserAuth &auth, const std::string &path)
{
    LOGI("enter");
    KeyContext ctxNone;  // 1.设备级,用户el1  无token  无secret    d
    if (!LoadKeyBlob(ctxNone.rndEnc, path + PATH_ENCRYPTED)) {
        LOGE("Load rndEnc failed !");
        return false;
    }

    if (!LoadKeyBlob(ctxNone.secDiscard, path + PATH_SECDISC) ||
        !LoadKeyBlob(ctxNone.shield, path + PATH_SHIELD)) {
        ctxNone.rndEnc.Clear();
        LOGE("Load shield failed !");
        return false;
    }

    LOGI("Decrypt by hks start.");    // keyCtx.rndEnc 80 -> 64
    if (!HuksMaster::GetInstance().DecryptKey(ctxNone, auth, keyInfo_, true)) {
        LOGE("Decrypt by hks failed.");
        ClearKeyContext(ctxNone);
        return false;
    }
    ClearKeyContext(ctxNone);
    LOGI("finish");
    return true;
}

bool BaseKey::DoRestoreKeyCeEceSece(const UserAuth &auth, const std::string &path, const uint32_t keyType)
{
    LOGI("enter");
    if ((auth.secret.IsEmpty() && auth.token.IsEmpty()) ||  // 无密码Avtive  新增密码,用空密码解密(ABC)
        (!auth.secret.IsEmpty() && !auth.token.IsEmpty())) { // 有密码Avtive 修改密码,老密码解密(ABC)  Pin码解锁(AB)
        KeyContext ctxNone;
        if (!LoadKeyBlob(ctxNone.rndEnc, path + PATH_ENCRYPTED)) {
            LOGE("Load rndEnc failed !");
            return false;
        }

        ctxNone.aad.Alloc(GCM_MAC_BYTES);
        ctxNone.nonce.Alloc(GCM_NONCE_BYTES);
        if (!SplitKeyCtx(ctxNone.rndEnc, ctxNone.nonce, ctxNone.rndEnc, ctxNone.aad)) {
            ctxNone.rndEnc.Clear();
            LOGE("Split key context failed !");
            return false;
        }
        if (!LoadKeyBlob(ctxNone.secDiscard, path + PATH_SECDISC) ||
            !LoadKeyBlob(ctxNone.shield, path + PATH_SHIELD)) {
            ctxNone.rndEnc.Clear();
            LOGE("Load shield failed !");
            return false;
        }
        return DecryptReal(auth, keyType, ctxNone);
    }

    // 人脸指纹场景  有token  无secret(AB)
    if (auth.secret.IsEmpty() && !auth.token.IsEmpty()) {
        if (!keyContext_.shield.IsEmpty() || !keyContext_.rndEnc.IsEmpty()) {
            LOGI("Restore key by face/finger");
            UserAuth mUserAuth = auth;
            mUserAuth.secret = KeyBlob(NULL_SECRET);
            KeyContext tempCtx = {};
            DoTempStore(keyContext_, tempCtx);
            if (!HuksMaster::GetInstance().DecryptKeyEx(tempCtx, mUserAuth, keyInfo_.key)) {
                LOGE("Decrypt by hks failed.");
                ClearKeyContext(tempCtx);
                return false;
            }
            ClearKeyContext(tempCtx);
        }
        return true;
    }
    LOGE("Decrypt failed, invalid param !");
    return false;
}

bool BaseKey::DoRestoreKey(const UserAuth &auth, const std::string &path)
{
    auto ver = KeyCtrlLoadVersion(dir_.c_str());
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return false;
    }

    std::string encryptType;
    LoadStringFromFile(path + SUFFIX_NEED_UPDATE, encryptType);
    LOGI("encrypt type : %{public}s, keyInfo empty: %{public}u", encryptType.c_str(), keyInfo_.key.IsEmpty());

    uint32_t keyType = GetTypeFromDir();
    if (keyType == TYPE_EL1 || keyType == TYPE_GLOBAL_EL1) {
        LOGI("Restore device key.");
        return DoRestoreKeyDe(auth, path);
    }
    int ret;
    if (encryptType == KeyEncryptTypeToString(KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL)) {
        LOGI("Restore ce ece sece key.");
        ret = DoRestoreKeyCeEceSece(auth, path, keyType);
    } else {
        ret = DoUpdateRestore(auth, path);
    }
    LOGI("end ret %{public}u", ret);
    return ret != 0;
}

bool BaseKey::DoUpdateRestore(const UserAuth &auth, const std::string &keyPath)
{
    LOGI("enter");
    if (!DoRestoreKeyOld(auth, keyPath)) {
        LOGE("Restore old failed !");
        return false;
    }
    if (std::filesystem::exists(dir_ + PATH_NEED_RESTORE_SUFFIX) && !auth.token.IsEmpty()) {
        LOGE("Double 2 single, skip huks -> huks-openssl !");
        return true;
    }
    uint64_t secureUid = { 0 };
    if (!IamClient::GetInstance().GetSecureUid(GetIdFromDir(), secureUid)) {
        LOGE("Get secure uid form iam failed, use default value.");
    }

    if (!StoreKey({ auth.token, auth.secret, secureUid })) {
        LOGE("Store old failed !");
        return false;
    }
    if (!UpdateKey()) {
        LOGE("Update old failed !");
        return false;
    }
    LOGI("finish");
    return true;
}

bool BaseKey::DecryptReal(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx)
{
    LOGI("enter");
    UserAuth mUserAuth = auth;
    if (auth.secret.IsEmpty()) {
        mUserAuth.secret = KeyBlob(NULL_SECRET);
    }
    KeyBlob rndEnc(keyCtx.rndEnc);
    if (!OpensslCrypto::AESDecrypt(mUserAuth.secret, keyCtx, rndEnc)) { // rndEncEnc -> rndEnc
        LOGE("Decrypt by openssl failed.");
        return false;
    }

    keyCtx.rndEnc = std::move(rndEnc);
    if (keyType == TYPE_EL3 || keyType == TYPE_EL4) {
        DoTempStore(keyCtx, keyContext_);
    }

    if (!HuksMaster::GetInstance().DecryptKeyEx(keyCtx, auth, keyInfo_.key)) { // rndEnc -> rnd
        LOGE("Decrypt by hks failed.");
        return false;
    }

    rndEnc.Clear();
    LOGI("finish");
    return true;
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
            ret = HuksMaster::GetInstance().DecryptKey(keyContext_, auth, keyInfo_, true);
            break;
        case KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL:
            LOGI("Huks openssl decrypt key, skip");
            break;
    }
    ClearKeyContext(keyContext_);
    return ret;
}

bool BaseKey::ClearKey(const std::string &mnt)
{
    LOGI("enter, dir_ = %{public}s", dir_.c_str());
    bool ret = InactiveKey(USER_DESTROY, mnt);
    if (!ret) {
        LOGE("InactiveKey failed.");
    }
    keyInfo_.key.Clear();
    bool needClearFlag = true;
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::string elNeedRestorePath = PATH_USER_EL1_DIR + std::to_string(GetIdFromDir()) + PATH_NEED_RESTORE_SUFFIX;
    if (std::filesystem::exists(elNeedRestorePath)) {
        needClearFlag = false;
        LOGI("needRestore flag exist, do not remove secret.");
    }
#endif
    if (needClearFlag) {
        LOGI("do clear key.");
        if (!IsDir(dir_)) {
            LOGE("dir not exist, do not need to remove dir");
            return ret;
        }
        WipingActionDir(dir_);
        std::string backupDir;
        KeyBackup::GetInstance().GetBackupDir(dir_, backupDir);
        WipingActionDir(backupDir);
        KeyBackup::GetInstance().RemoveNode(backupDir);
        LOGI("force remove backupDir, %{public}s.", backupDir.c_str());
        OHOS::ForceRemoveDirectory(backupDir);
        LOGI("force remove dir_, %{public}s.", dir_.c_str());
        bool removeRet = OHOS::ForceRemoveDirectory(dir_);
        if (!removeRet) {
            LOGE("ForceRemoveDirectory failed.");
            return removeRet;
        }
        // use F2FS_IOC_SEC_TRIM_FILE
    }
    return ret;
}

void BaseKey::WipingActionDir(std::string &path)
{
    std::vector<std::string> fileList;
    LOGI("WipingActionDir path.c_str() is %{public}s", path.c_str());
    OpenSubFile(path.c_str(), fileList);
    for (const auto &it: fileList) {
        FILE *f = fopen(it.c_str(), "w");
        if (f == nullptr) {
            LOGE("open %{public}s failed, errno %{public}u", it.c_str(), errno);
            return;
        }
        int fd = fileno(f);
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
        (void)fclose(f);
    }
}

void BaseKey::SyncKeyDir() const
{
    DIR *dir = opendir(dir_.c_str());
    if (dir == nullptr) {
        LOGE("open %{public}s failed, errno %{public}u", dir_.c_str(), errno);
        return;
    }
    int fd = dirfd(dir);
    if (fd < 0) {
        LOGE("open %{public}s failed, errno %{public}d", dir_.c_str(), errno);
        sync();
        return;
    }
    LOGI("start fsync, dir_ is %{public}s", dir_.c_str());
    if (fsync(fd) != 0) {
        LOGE("fsync %{public}s failed, errno %{public}d", dir_.c_str(), errno);
        syncfs(fd);
    }
    LOGI("fsync end");
    (void)closedir(dir);
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

bool BaseKey::GetOriginKey(KeyBlob &originKey)
{
    LOGI("enter");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("origin key is empty, need restore !");
        return false;
    }
    KeyBlob key(keyInfo_.key);
    originKey = std::move(key);
    return true;
}

void BaseKey::SetOriginKey(KeyBlob &originKey)
{
    LOGI("enter");
    keyInfo_.key = std::move(originKey);
    return;
}

bool BaseKey::EncryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey,
                             KeyBlob &encryptedKey)
{
    LOGI("enter");
    KeyContext keyCtx;
    if (!MkDirRecurse(keyPath, S_IRWXU)) {
        LOGE("MkDirRecurse failed!");
    }

    LOGI("key path is exist : %{public}d", FileExists(keyPath));
    if (!HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield) ||
        !SaveKeyBlob(keyCtx.shield, keyPath + PATH_SHIELD)) {
        LOGE("GenerateKey and save shield failed!");
        return false;
    }
    if (!GenerateAndSaveKeyBlob(keyCtx.secDiscard, keyPath + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        keyCtx.shield.Clear();
        LOGE("GenerateAndSaveKeyBlob sec_discard failed!");
        return false;
    }
    if (!HuksMaster::GetInstance().EncryptKey(keyCtx, auth, {.key = planKey}, false)) {
        keyCtx.shield.Clear();
        keyCtx.secDiscard.Clear();
        LOGE("HUKS encrypt key failed!");
        return false;
    }
    CombKeyBlob(keyCtx.rndEnc, keyCtx.nonce, encryptedKey);

    ClearKeyContext(keyCtx);
    LOGI("finish");
    return true;
}

bool BaseKey::DecryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey,
                             KeyBlob &decryptedKey)
{
    LOGI("enter");
    KeyContext keyCtx;
    auto candidate = GetCandidateDir();
    std::string path = candidate.empty() ? keyPath : candidate;
    if (!LoadKeyBlob(keyCtx.shield, path + PATH_SHIELD)) {
        LOGE("Load KeyBlob shield failed!");
        return false;
    }
    if (!LoadKeyBlob(keyCtx.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("Load KeyBlob secDiscard failed!");
        keyCtx.shield.Clear();
        return false;
    }

    KeyInfo planKeyInfo = {.key = planKey};
    SplitKeyBlob(planKey, keyCtx.rndEnc, keyCtx.nonce, AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES);
    LOGE("decrypted size : %{public}d, nonce size : %{public}d", keyCtx.rndEnc.size, keyCtx.nonce.size);

    if (!HuksMaster::GetInstance().DecryptKey(keyCtx, auth, planKeyInfo, false)) {
        keyCtx.shield.Clear();
        keyCtx.rndEnc.Clear();
        LOGE("HUKS decrypt key failed!");
        return false;
    }

    decryptedKey = std::move(planKeyInfo.key);
    planKeyInfo.key.Clear();
    ClearKeyContext(keyCtx);
    LOGI("finish");
    return true;
}

bool BaseKey::RenameKeyPath(const std::string &keyPath)
{
    // rename keypath/temp/ to keypath/version_xx/
    auto candidate = GetNextCandidateDir();
    LOGI("rename %{public}s to %{public}s", keyPath.c_str(), candidate.c_str());
    if (rename(keyPath.c_str(), candidate.c_str()) != 0) {
        LOGE("rename %{public}s to %{public}s failed!", keyPath.c_str(), candidate.c_str());
        return false;
    }
    SyncKeyDir();
    return true;
}

void BaseKey::CombKeyBlob(const KeyBlob &encAad, const KeyBlob &end, KeyBlob &keyOut)
{
    std::vector<uint8_t> startVct(encAad.data.get(), encAad.data.get() + encAad.size);
    std::vector<uint8_t> endVct(end.data.get(), end.data.get() + end.size);
    startVct.insert(startVct.end(), endVct.begin(), endVct.end());
    std::copy(startVct.begin(), startVct.end(), keyOut.data.get());
    startVct.clear();
    endVct.clear();
}

void BaseKey::SplitKeyBlob(const KeyBlob &keyIn, KeyBlob &encAad, KeyBlob &nonce, uint32_t start)
{
    std::vector<uint8_t> inVct(keyIn.data.get(), keyIn.data.get() + keyIn.size);
    encAad.Alloc(start);
    nonce.Alloc(keyIn.size - start);
    std::copy(inVct.begin(), inVct.begin() + start, encAad.data.get());
    std::copy(inVct.begin() + start, inVct.end(), nonce.data.get());
    inVct.clear();
}

void BaseKey::ClearMemoryKeyCtx()
{
    LOGI("enter, dir_ = %{public}s", dir_.c_str());
    keyContext_.rndEnc.Clear();
    keyContext_.shield.Clear();
    keyContext_.nonce.Clear();
    keyContext_.aad.Clear();
}

void BaseKey::ClearKeyContext(KeyContext &keyCtx)
{
    LOGI("enter clear");
    keyCtx.aad.Clear();
    keyCtx.nonce.Clear();
    keyCtx.shield.Clear();
    keyCtx.rndEnc.Clear();
    keyCtx.secDiscard.Clear();
}

std::string BaseKey::KeyEncryptTypeToString(KeyEncryptType keyEncryptType_) const
{
    switch (keyEncryptType_) {
        case KeyEncryptType::KEY_CRYPT_OPENSSL:
            return "KEY_CRYPT_OPENSSL";
        case KeyEncryptType::KEY_CRYPT_HUKS:
            return "KEY_CRYPT_HUKS";
        case KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL:
            return "KEY_CRYPT_HUKS_OPENSSL";
    }
}

bool BaseKey::CombKeyCtx(const KeyBlob &nonce, const KeyBlob &rndEnc, const KeyBlob &aad, KeyBlob &keyOut)
{
    LOGI("enter");
    if (nonce.IsEmpty() || aad.IsEmpty() || rndEnc.IsEmpty()) {
        LOGE("Invalid param, can not combine !");
        return false;
    }
    LOGE("rndEncEnc: %{public}u", rndEnc.size);
    std::vector<uint8_t> nonceVct(nonce.data.get(), nonce.data.get() + nonce.size);
    std::vector<uint8_t> rndVct(rndEnc.data.get(), rndEnc.data.get() + rndEnc.size);
    std::vector<uint8_t> aadVct(aad.data.get(), aad.data.get() + aad.size);

    nonceVct.insert(nonceVct.end(), rndVct.begin(), rndVct.end());
    nonceVct.insert(nonceVct.end(), aadVct.begin(), aadVct.end());
    std::copy(nonceVct.begin(), nonceVct.end(), keyOut.data.get());
    nonceVct.clear();
    rndVct.clear();
    aadVct.clear();
    return true;
}

bool BaseKey::SplitKeyCtx(const KeyBlob &keyIn, KeyBlob &nonce, KeyBlob &rndEnc, KeyBlob &aad)
{
    LOGI("enter");
    if (keyIn.size < (nonce.size + aad.size)) {
        LOGE("Invalid keyIn size is too small");
        return false;
    }
    std::vector<uint8_t> keyInVct(keyIn.data.get(), keyIn.data.get() + keyIn.size);
    rndEnc.Alloc(keyIn.size - nonce.size - aad.size);

    std::copy(keyInVct.begin(), keyInVct.begin() + nonce.size, nonce.data.get());
    std::copy(keyInVct.begin() + nonce.size, keyInVct.begin() + nonce.size + rndEnc.size, rndEnc.data.get());
    std::copy(keyInVct.begin() + nonce.size + rndEnc.size, keyInVct.end(), aad.data.get());
    LOGI("rndEncEnc: %{public}u", rndEnc.size);
    keyInVct.clear();
    return true;
}

uint32_t BaseKey::GetTypeFromDir()
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

std::string BaseKey::GetKeyDir()
{
    uint32_t type = GetTypeFromDir();
    switch (type) {
        case TYPE_EL1:
            return "el1";
        case TYPE_EL2:
            return "el2";
        case TYPE_EL3:
            return "el3";
        case TYPE_EL4:
            return "el4";
        case TYPE_EL5:
            return "el5";
        default:
            LOGE("type is error");
            return "";
    }
}

uint32_t BaseKey::GetIdFromDir()
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

bool BaseKey::KeyDescIsEmpty()
{
    return keyInfo_.keyDesc.IsEmpty();
}
} // namespace StorageDaemon
} // namespace OHOS
