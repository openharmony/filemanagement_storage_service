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

#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <regex>

#include "directory_ex.h"
#include "fbex.h"
#include "file_ex.h"
#include "huks_master.h"
#include "iam_client.h"
#include "key_backup.h"
#include "libfscrypt/key_control.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "utils/storage_radar.h"

namespace {
constexpr const char *PATH_LATEST_BACKUP = "/latest_bak";
constexpr const char *PATH_KEY_TEMP = "/temp";
constexpr const char *PATH_NEED_RESTORE_SUFFIX = "/latest/need_restore";
constexpr const char *PATH_USER_EL1_DIR = "/data/service/el1/public/storage_daemon/sd/el1/";
constexpr uint8_t USER_DESTROY = 0x1;
constexpr int32_t RESTORE_VERSION = 3;
const std::vector<uint8_t> NULL_SECRET = { '!' };
const std::vector<uint8_t> DEFAULT_KEY = { 'D', 'o', 'c', 's' };

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
    LOGI("Store huks result temporary");
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
    if (keyInfo_.version == FSCRYPT_INVALID || keyInfo_.version > KeyCtrlGetFscryptVersion(MNT_DATA)) {
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
    std::string errMsg = "";
    return WriteFileSync(path.c_str(), blob.data.get(), blob.size, errMsg);
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
    LOGW("enter %{public}s, size=%{public}d", path.c_str(), size);
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

void BaseKey::ClearKeyInfo()
{
    LOGI("begin clear key info.");
    keyInfo_.key.Clear();
}

int BaseKey::GetCandidateVersion() const
{
    std::string prefix(PATH_KEY_VERSION + 1); // skip the first slash
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
    if (candidate != -1) {
        LOGW("candidate key version is %{public}d", candidate);
    }
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
int32_t BaseKey::StoreKey(const UserAuth &auth, bool needGenerateShield)
#else
int32_t BaseKey::StoreKey(const UserAuth &auth)
#endif
{
    LOGI("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    int32_t ret = E_PERMISSION_DENIED;
#ifdef USER_CRYPTO_MIGRATE_KEY
    ret = DoStoreKey(auth, needGenerateShield);
#else
    ret = DoStoreKey(auth);
#endif
    if (ret == E_OK) {
        // rename keypath/temp/ to keypath/version_xx/
        auto candidate = GetNextCandidateDir();
        LOGI("rename %{public}s to %{public}s", pathTemp.c_str(), candidate.c_str());
        if (rename(pathTemp.c_str(), candidate.c_str()) == 0) {
            LOGI("start sync");
            SyncKeyDir();
            LOGI("sync end");
            return E_OK;
        }
        LOGE("rename fail return %{public}d, cleanup the temp dir", errno);
        ret = errno;
    } else {
        StorageService::StorageRadar::ReportUserKeyResult("StoreKey", 0, ret, "", "dir_=" + dir_);
        LOGE("DoStoreKey fail, cleanup the temp dir");
    }
    OHOS::ForceRemoveDirectory(pathTemp);
    LOGI("start sync");
    SyncKeyDir();
    LOGI("sync end");
    return ret;
}

// All key files are saved under keypath/temp/ in this function.
#ifdef USER_CRYPTO_MIGRATE_KEY
int32_t  BaseKey::DoStoreKey(const UserAuth &auth, bool needGenerateShield)
#else
int32_t  BaseKey::DoStoreKey(const UserAuth &auth)
#endif
{
    LOGI("enter");
    auto pathTemp = dir_ + PATH_KEY_TEMP;
    if (!MkDirRecurse(pathTemp, S_IRWXU)) {
        LOGE("MkDirRecurse failed!");
    }
    if (!CheckAndUpdateVersion()) {
        return E_VERSION_ERROR;
    }
    uint32_t keyType = GetTypeFromDir();
    if (keyType == TYPE_EL1 || keyType == TYPE_GLOBAL_EL1) {
        return EncryptDe(auth, pathTemp);
    }
    if ((auth.token.IsEmpty() && auth.secret.IsEmpty()) || // Create user/Delete pincode(EL2-4)
        !auth.token.IsEmpty()) {  // add/change pin code (EL2-4)
        LOGE("Encrypt huks openssl.");
        KeyContext keyCtx = {};
        auto ret = InitKeyContext(auth, pathTemp, keyCtx);
        if (ret != E_OK) {
            LOGE("init key context failed !");
            return ret;
        }

        ret = EncryptEceSece(auth, keyType, keyCtx);
        if (ret != E_OK) {
            LOGE("Encrypt key failed !");
            ClearKeyContext(keyCtx);
            return ret;
        }
        // save key buff nonce+rndEnc+aad
        if (!SaveAndCleanKeyBuff(pathTemp, keyCtx)) {
            LOGE("save key buff failed !");
            return E_SAVE_KEY_BUFFER_ERROR;
        }
    }
    LOGI("finish");
    return E_OK;
}

bool BaseKey::CheckAndUpdateVersion()
{
    auto pathVersion = dir_ + PATH_FSCRYPT_VER;
    std::string version;
    std::string errMsg = "";
    if (OHOS::LoadStringFromFile(pathVersion, version)) {
        if (version != std::to_string(keyInfo_.version)) {
            LOGE("version already exist %{public}s, not expected %{public}d", version.c_str(), keyInfo_.version);
            return false;
        }
    } else if (SaveStringToFileSync(pathVersion, std::to_string(keyInfo_.version), errMsg) == false) {
        StorageService::StorageRadar::ReportUserKeyResult("CheckAndUpdateVersion", 0, 0, "", "dir_=" + dir_);
        LOGE("save version failed, errno:%{public}d", errno);
        return false;
    }
    ChMod(pathVersion, S_IREAD | S_IWRITE);
    return true;
}

int32_t BaseKey::InitKeyContext(const UserAuth &auth, const std::string &keyPath, KeyContext &keyCtx)
{
    LOGI("enter");
    auto ret = LoadAndSaveShield(auth, keyPath + PATH_SHIELD, true, keyCtx);
    if (ret != E_OK) {
        LOGE("Load or save shield failed !");
        return ret;
    }

    if (!GenerateAndSaveKeyBlob(keyCtx.secDiscard, keyPath + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("Generate sec_discard failed");
        return E_GENERATE_DISCARD_ERROR;
    }

    if (!GenerateKeyBlob(keyCtx.nonce, GCM_NONCE_BYTES) ||
        !GenerateKeyBlob(keyCtx.aad, GCM_MAC_BYTES)) {
        LOGE("Generate nonce and aad failed !");
        return E_KEY_BLOB_ERROR;
    }
    return E_OK;
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

    storeKey.Clear();
    ClearKeyContext(keyCtx);
    const std::string NEED_UPDATE_PATH = keyPath + SUFFIX_NEED_UPDATE;
    if (!SaveStringToFile(NEED_UPDATE_PATH, KeyEncryptTypeToString(keyEncryptType_))) {
        LOGE("Save key type file failed");
        return false;
    }

    return true;
}

int32_t BaseKey::LoadAndSaveShield(const UserAuth &auth, const std::string &pathShield,
                                   bool needGenerateShield, KeyContext &keyCtx)
{
#ifdef USER_CRYPTO_MIGRATE_KEY
    if (needGenerateShield) {
        auto ret = HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield);
        if (ret != HKS_SUCCESS) {
            LOGE("GenerateKey of shield failed");
            return ret;
        }
    } else {
        if (!LoadKeyBlob(keyCtx.shield, dir_ + PATH_LATEST + PATH_SHIELD)) {  // needcheck is update
            keyCtx.rndEnc.Clear();
            return E_LOAD_KEY_BLOB_ERROR;
        }
    }
#else
    auto ret = HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield);
    if (ret != HKS_SUCCESS) {
        LOGE("GenerateKey of shield failed");
        return ret;
    }
#endif
    if (!SaveKeyBlob(keyCtx.shield,  pathShield)) {
        return E_SAVE_KEY_BLOB_ERROR;
    }
    return E_OK;
}

// update the latest and do cleanups.
int32_t BaseKey::UpdateKey(const std::string &keypath, bool needSyncCandidate)
{
    LOGI("enter");
    if (!needSyncCandidate) {
        LOGE("Do not update candidate file !");
        return E_OK;
    }
    auto candidate = keypath.empty() ? GetCandidateDir() : keypath;
    LOGI("ready to update, candiate = %{public}s", candidate.c_str());
    if (candidate.empty() && GetTypeFromDir() == TYPE_EL5) {
        LOGI("no uece candidate dir, do not need updateKey.");
        return E_OK;
    }
    if (candidate.empty()) {
        LOGE("no candidate dir");
        return E_EMPTY_CANDIDATE_ERROR;
    }

    if (strcmp(candidate.c_str(), (dir_ + PATH_LATEST).c_str()) != 0) {
        int32_t ret = UpdateOrRollbackKey(candidate);
        if (ret != E_OK) {
            LOGE("backup or rename failed, errno=%{public}d", errno);
            return ret;
        }
    }

    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    for (const auto &it: files) { // cleanup backup and other versions
        if (it != PATH_LATEST + 1) {
            OHOS::ForceRemoveDirectory(dir_ + "/" + it);
        }
    }

    std::string backupDir;
    KeyBackup::GetInstance().GetBackupDir(dir_, backupDir);
    KeyBackup::GetInstance().CreateBackup(dir_, backupDir, true);
    SyncKeyDir();
    return E_OK;
}

int32_t BaseKey::UpdateOrRollbackKey(const std::string &candidate)
{
    std::string latestPath = dir_ + PATH_LATEST;
    std::string latestPathBak = dir_ + PATH_LATEST_BACKUP;
    if (IsDir(latestPath)) { // rename latest to latest_backup
        OHOS::ForceRemoveDirectory(latestPathBak);
        if (rename(latestPath.c_str(), latestPathBak.c_str()) != 0) {
            LOGE("backup the latest fail errno:%{public}d", errno);
            return E_RENAME_FILE_ERROR;
        }
        LOGI("backup the latest success");
    }

    OHOS::ForceRemoveDirectory(latestPath);
    if (rename(candidate.c_str(), latestPath.c_str()) == 0) { // rename {candidate} to latest
        LOGI("rename candidate %{public}s to latest success", candidate.c_str());
        return E_OK;
    }
    LOGE("rename candidate to latest fail return %{public}d", errno);
    if (rename(latestPathBak.c_str(), latestPath.c_str()) != 0) {
        LOGE("restore the latest_backup fail errno:%{public}d", errno);
        return E_RENAME_FILE_ERROR;
    }
    LOGI("restore the latest_backup success");
    SyncKeyDir();
    return E_OK;
}

int32_t BaseKey::EncryptDe(const UserAuth &auth, const std::string &path)
{
    LOGI("enter");
    KeyContext ctxDe;
    auto ret = InitKeyContext(auth, path, ctxDe);
    if (ret != E_OK) {
        LOGE("init key context failed !");
        return ret;
    }
    keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS;
    ret = HuksMaster::GetInstance().EncryptKey(ctxDe, auth, keyInfo_, true);
    if (ret != E_OK) {
        LOGE("Encrypt by hks failed.");
        ClearKeyContext(ctxDe);
        return ret;
    }
    if (!SaveKeyBlob(ctxDe.rndEnc, path + PATH_ENCRYPTED)) {
        LOGE("SaveKeyBlob rndEnc failed.");
        ClearKeyContext(ctxDe);
        return E_SAVE_KEY_BLOB_ERROR;
    }
    const std::string NEED_UPDATE_PATH = path + SUFFIX_NEED_UPDATE;
    if (!SaveStringToFile(NEED_UPDATE_PATH, KeyEncryptTypeToString(keyEncryptType_))) {
        LOGE("Save key type file failed");
        return E_SAVE_KEY_TYPE_ERROR;
    }
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::EncryptEceSece(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx)
{
    LOGI("enter");
    // rnd 64 -> rndEnc 80
    auto ret = HuksMaster::GetInstance().EncryptKeyEx(auth, keyInfo_.key, keyCtx);
    if (ret != E_OK) {
        LOGE("Encrypt by hks failed.");
        return ret;
    }
    LOGE("Huks encrypt end.");

    UserAuth mUserAuth = auth;
    if (auth.secret.IsEmpty()) {
        mUserAuth.secret = KeyBlob(NULL_SECRET);
    }

    if (keyType == TYPE_EL3 || keyType == TYPE_EL4) {
        DoTempStore(keyCtx, keyContext_);
    }

    KeyBlob rndEnc(keyCtx.rndEnc);
    LOGI("Encrypt by openssl start"); // rndEnc 80 -> rndEncEnc 108
    ret = OpensslCrypto::AESEncrypt(mUserAuth.secret, rndEnc, keyCtx);
    if (ret != E_OK) {
        LOGE("Encrypt by openssl failed.");
        return ret;
    }
    LOGE("Encrypt by openssl end");
    rndEnc.Clear();
    keyEncryptType_ = KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL;
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::RestoreKey(const UserAuth &auth, bool needSyncCandidate)
{
    LOGD("BaseKey::RestoreKey enter, auth token %{public}d, auth secret %{public}d",
        auth.token.IsEmpty(), auth.secret.IsEmpty());
    auto candidate = GetCandidateDir();
    if (candidate.empty()) {
        // no candidate dir, just restore from the latest
        auto ret = KeyBackup::GetInstance().TryRestoreKey(shared_from_this(), auth);
        if (ret == 0) {
            return E_OK;
        }
        return ret;
    }

    auto ret = DoRestoreKey(auth, candidate);
    if (ret == E_OK) {
        // update the latest with the candidate
        UpdateKey("", needSyncCandidate);
        return E_OK;
    }

    LOGE("DoRestoreKey with %{public}s failed", candidate.c_str());
    // try to restore from other versions
    std::vector<std::string> files;
    GetSubDirs(dir_, files);
    std::sort(files.begin(), files.end(), [&](const std::string &a, const std::string &b) {
        if (a.length() != b.length() ||
            a.length() < strlen(PATH_KEY_VERSION) ||
            b.length() < strlen(PATH_KEY_VERSION)) {
            return a.length() > b.length();
        }
        std::regex pattern("^version_\\d+$");
        if (!std::regex_search(a, pattern)) return false;
        if (!std::regex_search(b, pattern)) return true;

        // make sure a and b is version_\d+
        auto a_len = std::atoi(a.substr(strlen(PATH_KEY_VERSION) - 1).c_str());
        auto b_len = std::atoi(b.substr(strlen(PATH_KEY_VERSION) - 1).c_str());
        return a_len > b_len;
    });
    for (const auto &it: files) {
        if (it != candidate) {
            ret = DoRestoreKey(auth, dir_ + "/" + it);
            if (ret == E_OK) {
                UpdateKey((dir_ + "/" + it), needSyncCandidate);
                return E_OK;
            }
        }
    }
    return ret;
}

int32_t BaseKey::RestoreKey4Nato(const std::string &keyDir, KeyType type)
{
    LOGI("Restore key for nato for type %{public}u keyDir=%{public}s enter.", type, keyDir.c_str());
    static const std::map<KeyType, uint32_t> keyTypeMap = {
        {KeyType::EL2_KEY, TYPE_EL2},
        {KeyType::EL3_KEY, TYPE_EL3},
        {KeyType::EL4_KEY, TYPE_EL4},
    };
 
    auto iter = keyTypeMap.find(type);
    if (iter == keyTypeMap.end()) {
        return E_PARAMS_INVALID;
    }
    UserAuth auth = { {}, {} };
    auto ret = DoRestoreKeyCeEceSece(auth, keyDir + PATH_LATEST, iter->second);
    if (ret != E_OK) {
        LOGE("Restore ce ece sece for nato secen failed !");
        return ret;
    }
    ret = StoreKey({ {}, {}, 0 });
    if (ret != E_OK) {
        LOGE("Store key for nato secen failed !");
        return ret;
    }
    ret = UpdateKey();
    if (ret != E_OK) {
        LOGE("Update key for nato secen failed !");
        return ret;
    }
    LOGI("Restore key for nato for type %{public}u success.", type);
    return E_OK;
}

int32_t BaseKey::DoRestoreKeyOld(const UserAuth &auth, const std::string &path)
{
    LOGW("enter, path = %{public}s", path.c_str());
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
        return E_PARAMS_INVALID;
    }
    if (!LoadKeyBlob(keyContext_.rndEnc, path + PATH_ENCRYPTED)) {
        return E_LOAD_KEY_BLOB_ERROR;
    }
    if (keyEncryptType_ == KeyEncryptType::KEY_CRYPT_HUKS) {
        if (!LoadKeyBlob(keyContext_.shield, path + PATH_SHIELD)) {
            keyContext_.rndEnc.Clear();
            return E_LOAD_KEY_BLOB_ERROR;
        }
    }
    if (!LoadKeyBlob(keyContext_.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        keyContext_.rndEnc.Clear();
        keyContext_.shield.Clear();
        return E_LOAD_KEY_BLOB_ERROR;
    }
    return Decrypt(auth);
}

int32_t BaseKey::DoRestoreKeyDe(const UserAuth &auth, const std::string &path)
{
    LOGI("enter");
    KeyContext ctxNone;
    if (!LoadKeyBlob(ctxNone.rndEnc, path + PATH_ENCRYPTED)) {
        LOGE("Load rndEnc failed !");
        return E_LOAD_KEY_BLOB_ERROR;
    }

    if (!LoadKeyBlob(ctxNone.secDiscard, path + PATH_SECDISC) ||
        !LoadKeyBlob(ctxNone.shield, path + PATH_SHIELD)) {
        ctxNone.rndEnc.Clear();
        LOGE("Load shield failed !");
        return E_LOAD_KEY_BLOB_ERROR;
    }

    LOGW("Decrypt by hks start.");  // keyCtx.rndEnc 80 -> 64
    auto ret = HuksMaster::GetInstance().DecryptKey(ctxNone, auth, keyInfo_, true);
    if (ret != E_OK) {
        LOGE("Decrypt by hks failed.");
        ClearKeyContext(ctxNone);
        return ret;
    }
    ClearKeyContext(ctxNone);
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::DoRestoreKeyCeEceSece(const UserAuth &auth, const std::string &path, const uint32_t keyType)
{
    LOGI("enter");
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    if ((auth.secret.IsEmpty() && auth.token.IsEmpty()) ||
        (!auth.secret.IsEmpty() && !auth.token.IsEmpty())) {
        KeyContext ctxNone;
        if (!LoadKeyBlob(ctxNone.rndEnc, path + PATH_ENCRYPTED)) {
            LOGE("Load rndEnc failed !");
            return E_LOAD_KEY_BLOB_ERROR;
        }

        ctxNone.aad.Alloc(GCM_MAC_BYTES);
        ctxNone.nonce.Alloc(GCM_NONCE_BYTES);
        if (!SplitKeyCtx(ctxNone.rndEnc, ctxNone.nonce, ctxNone.rndEnc, ctxNone.aad)) {
            ctxNone.rndEnc.Clear();
            LOGE("Split key context failed !");
            return E_SPILT_KEY_CTX_ERROR;
        }
        if (!LoadKeyBlob(ctxNone.secDiscard, path + PATH_SECDISC) ||
            !LoadKeyBlob(ctxNone.shield, path + PATH_SHIELD)) {
            ctxNone.rndEnc.Clear();
            LOGE("Load shield failed !");
            return E_SHIELD_OPERATION_ERROR;
        }
        auto delay = StorageService::StorageRadar::ReportDuration("READ KEY FILE: FILE OPS", startTime);
        LOGI("SD_DURATION: READ KEY FILE: delay time = %{public}s", delay.c_str());
        return DecryptReal(auth, keyType, ctxNone);
    }

    // face/finger (EL3 EL4)
    if (auth.secret.IsEmpty() && !auth.token.IsEmpty()) {
        if (!keyContext_.shield.IsEmpty() || !keyContext_.rndEnc.IsEmpty()) {
            LOGI("Restore key by face/finger");
            UserAuth mUserAuth = auth;
            mUserAuth.secret = KeyBlob(NULL_SECRET);
            KeyContext tempCtx = {};
            DoTempStore(keyContext_, tempCtx);
            auto ret = HuksMaster::GetInstance().DecryptKeyEx(tempCtx, mUserAuth, keyInfo_.key);
            if (ret != E_OK) {
                LOGE("Decrypt by hks failed.");
                ClearKeyContext(tempCtx);
                return ret;
            }
            ClearKeyContext(tempCtx);
        }
        return E_OK;
    }
    LOGE("Decrypt failed, invalid param !");
    return E_PARAMS_INVALID;
}

int32_t BaseKey::DoRestoreKey(const UserAuth &auth, const std::string &path)
{
    auto ver = KeyCtrlLoadVersion(dir_.c_str());
    if (ver == FSCRYPT_INVALID || ver != keyInfo_.version) {
        LOGE("RestoreKey fail. bad version loaded %{public}u not expected %{public}u", ver, keyInfo_.version);
        return E_VERSION_ERROR;
    }

    std::string encryptType;
    LoadStringFromFile(path + SUFFIX_NEED_UPDATE, encryptType);
    LOGI("encrypt type : %{public}s, keyInfo empty: %{public}u", encryptType.c_str(), keyInfo_.key.IsEmpty());

    uint32_t keyType = GetTypeFromDir();
    if (keyType == TYPE_EL1 || keyType == TYPE_GLOBAL_EL1) {
        LOGI("Restore device key.");
        return DoRestoreKeyDe(auth, path);
    }
    int ret = -1;
    if (encryptType == KeyEncryptTypeToString(KeyEncryptType::KEY_CRYPT_HUKS_OPENSSL)) {
        LOGD("Restore ce ece sece key.");
        ret = DoRestoreKeyCeEceSece(auth, path, keyType);
    }
    std::error_code errCode;
    std::string need_restore;
    LoadStringFromFile(path + SUFFIX_NEED_RESTORE, need_restore);
    uint32_t restore_version = static_cast<uint32_t>(std::atoi(need_restore.c_str()));
    UpdateVersion update_version = static_cast<UpdateVersion>(std::atoi(need_restore.c_str()));
    LOGI("NeedRestore Path is: %{public}s, restore_version: %{public}u", path.c_str(), restore_version);
    if (std::filesystem::exists(path + SUFFIX_NEED_RESTORE, errCode)) {
        if (restore_version < RESTORE_VERSION) {
            LOGW("Old DOUBLE_2_SINGLE.");
            ret = DoUpdateRestore(auth, path);
        } else {
            LOGW("New DOUBLE_2_SINGLE.");
            ret = DoUpdateRestoreVx(auth, path, update_version);
        }
    }
    LOGI("end ret %{public}u, filepath isExist: %{public}u", ret, errCode.value());
    return ret;
}

int32_t BaseKey::DoUpdateRestore(const UserAuth &auth, const std::string &keyPath)
{
    LOGI("enter");
    auto ret = DoRestoreKeyOld(auth, keyPath);
    if (ret != E_OK) {
        LOGE("Restore old failed !");
        return ret;
    }
    std::error_code errCode;
    if (std::filesystem::exists(dir_ + PATH_NEED_RESTORE_SUFFIX, errCode) && !auth.token.IsEmpty()) {
        LOGE("Double 2 single, skip huks -> huks-openssl !");
        return E_OK;
    }
    uint64_t secureUid = 0;
    uint32_t userId = GetIdFromDir();
    if ((userId < StorageService::START_APP_CLONE_USER_ID || userId >= StorageService::MAX_APP_CLONE_USER_ID) &&
        !IamClient::GetInstance().GetSecureUid(userId, secureUid)) {
        LOGE("Get secure uid form iam failed, use default value.");
    }
    ret = StoreKey({ auth.token, auth.secret, secureUid });
    if (ret != E_OK) {
        LOGE("Store old failed !");
        return ret;
    }
    ret = UpdateKey();
    if (ret != E_OK) {
        LOGE("Update old failed !");
        return ret;
    }
    LOGI("finish");
    return E_OK;
}
int32_t BaseKey::DoUpdateRestoreVx(const UserAuth &auth, const std::string &keyPath, UpdateVersion update_version)
{
    LOGI("enter");
    LOGI("Restore version %{public}u", update_version);
    auto ret = DoRestoreKeyCeEceSece(auth, keyPath, GetTypeFromDir());
    if (ret != E_OK) {
        LOGE("Restore ce ece sece failed !");
        return ret;
    }
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::DecryptReal(const UserAuth &auth, const uint32_t keyType, KeyContext &keyCtx)
{
    LOGI("enter");
    UserAuth mUserAuth = auth;
    if (auth.secret.IsEmpty()) {
        mUserAuth.secret = KeyBlob(NULL_SECRET);
    }
    KeyBlob rndEnc(keyCtx.rndEnc);
    auto ret = OpensslCrypto::AESDecrypt(mUserAuth.secret, keyCtx, rndEnc);
    if (ret != E_OK) { // rndEncEnc -> rndEnc
        LOGE("Decrypt by openssl failed.");
        return ret;
    }

    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    keyCtx.rndEnc = std::move(rndEnc);
    if (keyType == TYPE_EL3 || keyType == TYPE_EL4) {
        DoTempStore(keyCtx, keyContext_);
    }
    ret = HuksMaster::GetInstance().DecryptKeyEx(keyCtx, auth, keyInfo_.key);
    if (ret != E_OK) { // rndEnc -> rnd
        LOGE("Decrypt by hks failed.");
        return ret;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("HUKS: DECRYPT KEY EX", startTime);
    LOGI("SD_DURATION: HUKS: DECRYPT KEY EX: delay time = %{public}s", delay.c_str());

    rndEnc.Clear();
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::Decrypt(const UserAuth &auth)
{
    int32_t ret = E_PARAMS_INVALID;
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
    auto ret = InactiveKey(USER_DESTROY, mnt);
    if (ret != E_OK) {
        LOGE("InactiveKey failed.");
    }
    keyInfo_.key.Clear();
    keyInfo_.keyHash.Clear();
    bool needClearFlag = true;
#ifdef USER_CRYPTO_MIGRATE_KEY
    std::error_code errCode;
    std::string elNeedRestorePath(PATH_USER_EL1_DIR);
    elNeedRestorePath += std::to_string(GetIdFromDir());
    elNeedRestorePath += PATH_NEED_RESTORE_SUFFIX;
    if (std::filesystem::exists(elNeedRestorePath, errCode)) {
        needClearFlag = false;
        LOGI("needRestore flag exist, do not remove secret.");
    }
#endif
    if (needClearFlag) {
        LOGI("do clear key.");
        if (!IsDir(dir_)) {
            LOGE("dir not exist, do not need to remove dir");
            return (ret == E_OK);
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
    return (ret == E_OK);
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
            (void)fclose(f);
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
        (void)closedir(dir);
        return;
    }
    LOGW("start fsync, dir_ is %{public}s", dir_.c_str());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    if (fsync(fd) != 0) {
        LOGE("fsync %{public}s failed, errno %{public}d", dir_.c_str(), errno);
        syncfs(fd);
    }
    auto delay = StorageService::StorageRadar::ReportDuration("FSYNC",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, StorageService::DEFAULT_USERID);
    LOGW("fsync end. SD_DURATION: FSYNC: delay time = %{public}s", delay.c_str());
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

bool BaseKey::GetHashKey(KeyBlob &hashKey)
{
    LOGI("enter");
    if (keyInfo_.keyHash.IsEmpty()) {
        LOGE("hash key is empty.");
        return false;
    }
    KeyBlob key(keyInfo_.keyHash);
    hashKey = std::move(key);
    return true;
}

bool BaseKey::GenerateHashKey()
{
    LOGI("enter");
    if (keyInfo_.key.IsEmpty()) {
        LOGE("origin key is empty, Generate error");
        return false;
    }
    
    if (!keyInfo_.keyHash.IsEmpty()) {
        LOGW("clear hash key when is not empty");
        keyInfo_.keyHash.Clear();
    }

    KeyBlob preKey(DEFAULT_KEY);
    KeyBlob hashKey = OpensslCrypto::HashWithPrefix(preKey, keyInfo_.key, AES_256_HASH_RANDOM_SIZE);
    keyInfo_.keyHash = std::move(hashKey);
    return true;
}

int32_t BaseKey::EncryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey,
                                KeyBlob &encryptedKey)
{
    LOGI("enter");
    KeyContext keyCtx;
    if (!MkDirRecurse(keyPath, S_IRWXU)) {
        LOGE("MkDirRecurse failed!");
    }

    LOGW("key path is exist : %{public}d", FileExists(keyPath));
    if (HuksMaster::GetInstance().GenerateKey(auth, keyCtx.shield) != HKS_SUCCESS ||
        !SaveKeyBlob(keyCtx.shield, keyPath + PATH_SHIELD)) {
        LOGE("GenerateKey and save shield failed!");
        return E_SHIELD_OPERATION_ERROR;
    }
    if (!GenerateAndSaveKeyBlob(keyCtx.secDiscard, keyPath + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        keyCtx.shield.Clear();
        LOGE("GenerateAndSaveKeyBlob sec_discard failed!");
        return E_SAVE_KEY_BLOB_ERROR;
    }
    auto ret = HuksMaster::GetInstance().EncryptKey(keyCtx, auth, {.key = planKey}, false);
    if (ret != E_OK) {
        keyCtx.shield.Clear();
        keyCtx.secDiscard.Clear();
        LOGE("HUKS encrypt key failed!");
        return ret;
    }
    CombKeyBlob(keyCtx.rndEnc, keyCtx.nonce, encryptedKey);

    ClearKeyContext(keyCtx);
    LOGI("finish");
    return E_OK;
}

int32_t BaseKey::DecryptKeyBlob(const UserAuth &auth, const std::string &keyPath, KeyBlob &planKey,
                                KeyBlob &decryptedKey)
{
    LOGI("enter");
    KeyContext keyCtx;
    auto candidate = GetCandidateDir();
    std::string path = candidate.empty() ? keyPath : candidate;
    LOGI("Key path is exist : %{public}d", FileExists(path));

    if (!LoadKeyBlob(keyCtx.shield, path + PATH_SHIELD)) {
        LOGE("Load KeyBlob shield failed!");
        return E_LOAD_KEY_BLOB_ERROR;
    }
    if (!LoadKeyBlob(keyCtx.secDiscard, path + PATH_SECDISC, CRYPTO_KEY_SECDISC_SIZE)) {
        LOGE("Load key secDiscard failed!");
        keyCtx.shield.Clear();
        return E_LOAD_KEY_BLOB_ERROR;
    }

    KeyInfo planKeyInfo = {.key = planKey};
    SplitKeyBlob(planKey, keyCtx.rndEnc, keyCtx.nonce, AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES);
    LOGE("decrypted size : %{public}d, nonce size : %{public}d", keyCtx.rndEnc.size, keyCtx.nonce.size);

    auto ret = HuksMaster::GetInstance().DecryptKey(keyCtx, auth, planKeyInfo, false);
    if (ret != E_OK) {
        keyCtx.shield.Clear();
        keyCtx.rndEnc.Clear();
        LOGE("HUKS decrypt key failed!");
        return ret;
    }

    decryptedKey = std::move(planKeyInfo.key);
    planKeyInfo.key.Clear();
    ClearKeyContext(keyCtx);
    LOGI("finish");
    return E_OK;
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

void BaseKey::ClearKeyContext(KeyContext &keyCtx)
{
    LOGD("BaseKey::ClearKeyContext enter");
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
            StorageService::StorageRadar::ReportUserKeyResult("GetKeyDir", 0, 0, "", "dir_=" + dir_);
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
    LOGI("The keyBlob is null? %{public}d", keyInfo_.keyDesc.IsEmpty());
    return keyInfo_.keyDesc.IsEmpty();
}
} // namespace StorageDaemon
} // namespace OHOS
