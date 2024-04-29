/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "user/user_manager.h"
#include <cstdlib>
#include "bundle_mgr_client.h"
#ifdef USER_CRYPTO_MANAGER
#include "crypto/key_manager.h"
#endif
#include "ipc/istorage_daemon.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
static constexpr int MODE_0711 = 0711;
static constexpr int MODE_02771 = 02771;
std::shared_ptr<UserManager> UserManager::instance_ = nullptr;
UserManager::UserManager()
    : rootDirVec_{{"/data/app/%s/%d", MODE_0711, OID_ROOT, OID_ROOT},
                  {"/data/service/%s/%d", MODE_0711, OID_ROOT, OID_ROOT},
                  {"/data/chipset/%s/%d", MODE_0711, OID_ROOT, OID_ROOT}},
      el3DirEl4DirEl5DirVec_{{"/data/app/%s/%d", MODE_0711, OID_ROOT, OID_ROOT},
                     {"/data/service/%s/%d", MODE_0711, OID_ROOT, OID_ROOT}},
      subDirVec_{{"/data/app/%s/%d/base", MODE_0711, OID_ROOT, OID_ROOT},
                 {"/data/app/%s/%d/database", MODE_0711, OID_ROOT, OID_ROOT}},
      el2DirVec_{{"/data/service/el2/%d/backup", MODE_02771, OID_BACKUP, OID_BACKUP},
                 {"/data/service/el2/%d/backup/backup_sa", MODE_0711, OID_BACKUP, OID_BACKUP},
                 {"/data/service/el2/%d/backup/bundles", MODE_0711, OID_BACKUP, OID_BACKUP},
                 {"/data/app/el2/%d/log", MODE_0711, OID_ROOT, OID_ROOT}},
      el1DirVec_{{"/data/service/el1/%d/distributeddata", MODE_0711, OID_DDMS, OID_DDMS},
                 {"/data/service/el1/%d/backup", MODE_02771, OID_BACKUP, OID_BACKUP},
                 {"/data/service/el1/%d/backup/bundles", MODE_0711, OID_BACKUP, OID_BACKUP}}
{
}

std::shared_ptr<UserManager> UserManager::GetInstance()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() { instance_ = std::make_shared<UserManager>(); });

    return instance_;
}

int32_t UserManager::StartUser(int32_t userId)
{
    LOGI("start user %{public}d", userId);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("UserManager::StartUser userId %{public}d out of range", userId);
        return err;
    }
    return MountManager::GetInstance()->MountByUser(userId);
}

int32_t UserManager::StopUser(int32_t userId)
{
    LOGI("stop user %{public}d", userId);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("UserManager::StopUser userId %{public}d out of range", userId);
        return err;
    }
    return MountManager::GetInstance()->UmountByUser(userId);
}

int32_t UserManager::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("prepare user dirs for %{public}d, flags %{public}u", userId, flags);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = CheckCrypto(userId, flags);
    if (err != E_OK) {
        return err;
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL2) {
        err = MountManager::GetInstance()->PrepareHmdfsDirs(userId);
        if (err != E_OK) {
            LOGE("Prepare hmdfs dir error");
            return err;
        }
        err = MountManager::GetInstance()->PrepareFileManagerDirs(userId);
        if (err != E_OK) {
            LOGE("Prepare fileManager dir error");
            return err;
        }
    }

    return E_OK;
}

int32_t UserManager::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("destroy user dirs for %{public}d, flags %{public}u", userId, flags);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("UserManager::DestroyUserDirs userId %{public}d out of range", userId);
        return err;
    }
    int32_t ret = E_OK;
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL1) {
        err = DestroyDirsFromIdAndLevel(userId, EL1);
        ret = (err != E_OK) ? err : ret;

        err = DestroyEl1BundleDir(userId);
        ret = (err != E_OK) ? err : ret;
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL2) {
        err = DestroyDirsFromIdAndLevel(userId, EL2);
        ret = (err != E_OK) ? err : ret;

        err = MountManager::GetInstance()->DestroyFileManagerDirs(userId);
        ret = (err != E_OK) ? err : ret;

        err = MountManager::GetInstance()->DestroyHmdfsDirs(userId);
        ret = (err != E_OK) ? err : ret;

        err = MountManager::GetInstance()->DestroySystemServiceDirs(userId);
        ret = (err != E_OK) ? err : ret;

        err = DestroyEl2BackupDir(userId);
        ret = (err != E_OK) ? err : ret;

        err = DestroyEl1Dir(userId);
        ret = (err != E_OK) ? err : ret;
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL3) {
        err = DestroyDirsFromIdAndLevel(userId, EL3);
        ret = (err != E_OK) ? err : ret;
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL4) {
        err = DestroyDirsFromIdAndLevel(userId, EL4);
        ret = (err != E_OK) ? err : ret;
    }

    return ret;
}

inline bool PrepareDirsFromVec(int32_t userId, const std::string &level, const std::vector<DirInfo> &vec)
{
    for (const DirInfo &dir : vec) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), level.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return false;
        }
    }

    return true;
}

inline bool DestroyDirsFromVec(int32_t userId, const std::string &level, const std::vector<DirInfo> &vec)
{
    bool err = true;

    for (const DirInfo &dir : vec) {
        if (IsEndWith(dir.path.c_str(), "%d")) {
            err = RmDirRecurse(StringPrintf(dir.path.c_str(), level.c_str(), userId));
        }
    }

    return err;
}

int32_t UserManager::PrepareDirsFromIdAndLevel(int32_t userId, const std::string &level)
{
    std::vector<FileList> list;
    if (level != EL3 && level != EL4) {
        if (!PrepareDirsFromVec(userId, level, rootDirVec_)) {
            LOGE("failed to prepare %{public}s root dirs for userid %{public}d", level.c_str(), userId);
            return E_PREPARE_DIR;
        }
        // set policy here
        for (auto item : rootDirVec_) {
            FileList temp;
            temp.userId = static_cast<uint32_t>(userId);
            temp.path = StringPrintf(item.path.c_str(), level.c_str(), userId);
            list.push_back(temp);
        }
    } else {
        if (!PrepareDirsFromVec(userId, level, el3DirEl4DirEl5DirVec_)) {
            LOGE("failed to prepare %{public}s root dirs for userid %{public}d", level.c_str(), userId);
            return E_PREPARE_DIR;
        }
        // set policy here
        for (auto item : el3DirEl4DirEl5DirVec_) {
            FileList temp;
            temp.userId = static_cast<uint32_t>(userId);
            temp.path = StringPrintf(item.path.c_str(), level.c_str(), userId);
            list.push_back(temp);
        }
    }
    int ret = SetElDirFscryptPolicy(userId, level, list);
    if (ret != E_OK) {
        LOGE("Set el poilcy failed");
        return ret;
    }

    if (!PrepareDirsFromVec(userId, level, subDirVec_)) {
        LOGE("failed to prepare %{public}s sub dirs for userid %{public}d", level.c_str(), userId);
        return E_PREPARE_DIR;
    }

    return E_OK;
}

int32_t UserManager::DestroyDirsFromIdAndLevel(int32_t userId, const std::string &level)
{
    if (level != EL3 && level != EL4) {
        if (!DestroyDirsFromVec(userId, level, rootDirVec_)) {
            LOGE("failed to destroy %{public}s dirs for userid %{public}d", level.c_str(), userId);
            return E_DESTROY_DIR;
        }
    } else {
        if (!DestroyDirsFromVec(userId, level, el3DirEl4DirEl5DirVec_)) {
            LOGE("failed to destroy %{public}s dirs for userid %{public}d", level.c_str(), userId);
            return E_DESTROY_DIR;
        }
    }
    return E_OK;
}

int32_t UserManager::PrepareEl1BundleDir(int32_t userId)
{
    if (!PrepareDir(StringPrintf(bundle_.c_str(), userId), MODE_0711, OID_ROOT, OID_ROOT)) {
        return E_PREPARE_DIR;
    }

    // set policy here
    std::vector<FileList> list;
    FileList temp;
    temp.userId = static_cast<uint32_t>(userId);
    temp.path = StringPrintf(bundle_.c_str(), userId);
    list.push_back(temp);
    int ret = SetElDirFscryptPolicy(userId, EL1, list);
    if (ret != E_OK) {
        LOGE("Set el1 poilcy failed");
        return ret;
    }

    return E_OK;
}

int32_t UserManager::DestroyEl1BundleDir(int32_t userId)
{
    if (!RmDirRecurse(StringPrintf(bundle_.c_str(), userId))) {
        return E_DESTROY_DIR;
    }

    return E_OK;
}

int32_t UserManager::SetElDirFscryptPolicy(int32_t userId, const std::string &level,
                                           const std::vector<FileList> &list)
{
#ifdef USER_CRYPTO_MANAGER
    if (EL_DIR_MAP.find(level) == EL_DIR_MAP.end()) {
        LOGE("el type error");
        return E_SET_POLICY;
    }
    if (KeyManager::GetInstance()->SetDirectoryElPolicy(userId, EL_DIR_MAP[level], list)) {
        LOGE("Set user dir el1 policy error");
        return E_SET_POLICY;
    }
#endif

    return E_OK;
}

int32_t UserManager::PrepareEl2BackupDir(int32_t userId)
{
    for (const DirInfo &dir : el2DirVec_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::DestroyEl2BackupDir(int32_t userId)
{
    for (const DirInfo &dir : el2DirVec_) {
        if (!RmDirRecurse(StringPrintf(dir.path.c_str(), userId))) {
            return E_DESTROY_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::PrepareEl1Dir(int32_t userId)
{
    for (const DirInfo &dir : el1DirVec_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::DestroyEl1Dir(int32_t userId)
{
    for (const DirInfo &dir : el1DirVec_) {
        if (!RmDirRecurse(StringPrintf(dir.path.c_str(), userId))) {
            return E_DESTROY_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::CheckUserIdRange(int32_t userId)
{
    if ((userId < StorageService::START_USER_ID && userId != StorageService::ZERO_USER_ID)||
        userId > StorageService::MAX_USER_ID) {
        LOGE("UserManager: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

void UserManager::CreateBundleDataDir(uint32_t userId)
{
    OHOS::AppExecFwk::BundleMgrClient client;
    LOGI("CreateBundleDataDir start: userId %{public}u", userId);
    auto ret = client.CreateBundleDataDir(userId);
    LOGI("CreateBundleDataDir end: userId %{public}u, ret %{public}d", userId, ret);
}

int32_t UserManager::CheckCrypto(int32_t userId, uint32_t flags)
{
    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("UserManager::PrepareUserDirs userId %{public}d out of range", userId);
        return err;
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL1) {
        err = PrepareDirsFromIdAndLevel(userId, EL1);
        if (err != E_OK) {
            return err;
        }
        err = PrepareEl1BundleDir(userId);
        if (err != E_OK) {
            return err;
        }
        int32_t errorCode = PrepareEl1Dir(userId);
        if (errorCode != E_OK) {
            LOGW("Prepare el1 dir fail, %{public}d.", errorCode);
        }
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL2) {
        err = PrepareDirsFromIdAndLevel(userId, EL2);
        if (err != E_OK) {
            return err;
        }
        err = PrepareEl2BackupDir(userId);
        if (err != E_OK) {
            return err;
        }
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL3) {
        err = PrepareDirsFromIdAndLevel(userId, EL3);
        if (err != E_OK) {
            return err;
        }
    }
    if (flags & IStorageDaemon::CRYPTO_FLAG_EL4) {
        err = PrepareDirsFromIdAndLevel(userId, EL4);
        if (err != E_OK) {
            return err;
        }
    }
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
