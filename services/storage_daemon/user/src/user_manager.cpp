/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <sys/stat.h>

#ifdef USER_CRYPTO_MANAGER
#include "crypto/key_manager.h"
#endif
#include "istorage_daemon.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "user/mount_constant.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "ipc/storage_manager_client.h"

#include "user/user_path_resolver.h"
#include "quota/quota_manager.h"
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif

using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {

UserManager &UserManager::GetInstance()
{
    static UserManager instance_;
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
    uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2 | IStorageDaemonEnum::CRYPTO_FLAG_EL3 |
        IStorageDaemonEnum::CRYPTO_FLAG_EL4 | IStorageDaemonEnum::CRYPTO_FLAG_EL5;
    CreateServiceDirs(userId, flags);
    return MountManager::GetInstance().MountByUser(userId);
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
    return MountManager::GetInstance().UmountByUser(userId);
}

int32_t UserManager::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("prepare user dirs for %{public}d, flags %{public}u", userId, flags);
    std::lock_guard<std::mutex> lock(mutex_);

    int32_t err = CheckUserIdRange(userId);
    if (err != E_OK) {
        LOGE("UserManager::PrepareUserDirs userId %{public}d out of range", userId);
        return err;
    }

    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetUserBasePath(userId, flags, dirInfoList.data);
    if (ret != E_OK) {
        return ret;
    }
    for (const auto &dirInfo : dirInfoList.data) {
        ret = dirInfo.MakeDir();
        if (ret != E_OK && dirInfo.path.find(EL1) == std::string::npos) {
            std::string extraData = "dirPath=" + dirInfo.path + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("PrepareUserDirs", userId, E_PREPARE_DIR, extraData);
            return ret;
        }
        if (SetElDirFscryptPolicy(userId, dirInfo.path)) {
            return E_SET_POLICY;
        }
    }
    return CreateServiceDirs(userId, flags);
}

int32_t UserManager::PrepareAllUserEl1Dirs()
{
    LOGI("start");
    std::vector<int32_t> userIds {GLOBAL_USER_ID};
    MountManager::GetInstance().GetAllUserId(userIds);
    int32_t ret = E_OK;
    for (const int32_t &item: userIds) {
        auto err = PrepareUserDirs(item, IStorageDaemonEnum::CRYPTO_FLAG_EL1);
        if (err != E_OK) {
            LOGE("PrepareAllUserEl1Dirs fail, userId=%{public}d", item);
            ret = err;
        }
    }
    return ret;
}

int32_t UserManager::CreateServiceDirs(int32_t userId, uint32_t flags)
{
    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetUserServicePath(userId, flags, dirInfoList.data);
    if (ret != E_OK) {
        return ret;
    }
    for (auto &dirInfo : dirInfoList.data) {
        dirInfo.UpdateDirUid(userId);
        auto err = dirInfo.MakeDir();
        if (err != E_OK) {
            std::string extraData = "dirPath=" + dirInfo.path + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("CreateServiceDirs", userId, E_PREPARE_DIR, extraData);
            ret = err;
        }
        
        auto it = dirInfo.options.find("set_prjId");
        if (it != dirInfo.options.end()) {
            int64_t prjId = 0;
            ConvertStringToInt(it->second, prjId);
            QuotaManager::GetInstance().SetQuotaPrjId(dirInfo.path, static_cast<int32_t>(prjId), true);
        }
    }
    return ret;
}

int32_t UserManager::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("destroy user dirs for %{public}d, flags %{public}u", userId, flags);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = CheckUserIdRange(userId);
    if (ret != E_OK) {
        LOGE("UserManager::DestroyUserDirs userId %{public}d out of range", userId);
        return ret;
    }

    InfoList<DirInfo> dirInfoList;
    ret = UserPathResolver::GetUserServicePath(userId, flags, dirInfoList.data);
    if (ret != E_OK) {
        return ret;
    }
    for (auto dirInfo = dirInfoList.data.rbegin(); dirInfo != dirInfoList.data.rend(); ++dirInfo) {
        auto err = dirInfo->RemoveDir();
        ret = (err != E_OK) ? err : ret;
    }
  
    std::vector<DirInfo>().swap(dirInfoList.data);
    auto ret2 = UserPathResolver::GetUserBasePath(userId, flags, dirInfoList.data);
    if (ret2 != E_OK) {
        return ret2;
    }
    for (auto dirInfo = dirInfoList.data.rbegin(); dirInfo != dirInfoList.data.rend(); ++dirInfo) {
        auto err = dirInfo->RemoveDir();
        ret = (err != E_OK) ? err : ret;
    }
    return ret;
}

void UserManager::CheckDirsFromVec(int32_t userId)
{
    uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1 | IStorageDaemonEnum::CRYPTO_FLAG_EL2 |
    IStorageDaemonEnum::CRYPTO_FLAG_EL3 | IStorageDaemonEnum::CRYPTO_FLAG_EL4 | IStorageDaemonEnum::CRYPTO_FLAG_EL5;

    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetUserBasePath(userId, flags, dirInfoList.data);
    if (ret != E_OK) return;
    for (const auto &dirInfo : dirInfoList.data) {
        const auto &options = dirInfo.options;
        if (options.find("check_dir") == options.end()) {
            continue;
        }
        dirInfo.MakeDir();
    }
}

int32_t UserManager::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    LOGI("CreateUserDir path: %{public}s, %{public}d, %{public}d, %{public}d", path.c_str(), mode, uid, gid);
    std::string prefix = "/data/virt_service/rgm_hmos/anco_hmos_data/";
    if (path.compare(0, prefix.size(), prefix) != 0) {
        LOGE("The path: %{public}s is invalid", path.c_str());
        return E_PARAMS_INVALID;
    }

    auto ret = PrepareDirSimple(path, mode, uid, gid);
    if (ret != E_OK) {
        LOGE("Failed to prepareDir %{public}s, ret: %{public}d", path.c_str(), ret);
    } else {
        LOGI("CreateUserDir end. ret: %{public}d", ret);
    }
    std::string extraData = "path=" + path + ", mode=" + std::to_string(mode) +
        ", uid=" + std::to_string(uid) + ", gid=" + std::to_string(gid);
    StorageRadar::ReportUserManager("CreateUserDir", 0, ret, extraData);
    return ret;
}

int32_t UserManager::SetElDirFscryptPolicy(int32_t userId, const std::string &path)
{
#ifdef USER_CRYPTO_MANAGER
    for (auto &level : EL_DIR_MAP) {
        if (path.find(level.first) == std::string::npos) {
            continue;
        }
        FileList temp{.userId = userId, .path = path};
        if (KeyManager::GetInstance().SetDirectoryElPolicy(userId, level.second, {temp}) != E_OK) {
            LOGE("Set user dir el1 policy error");
            return E_SET_POLICY;
        }
        return E_OK;
    }
    LOGE("Set user dir el1 policy error");
    return E_SET_POLICY;
#endif
    return E_OK;
}

void UserManager::CreateElxBundleDataDir(uint32_t userId, uint8_t elx)
{
    LOGI("CreateElxBundleDataDir start: userId %{public}u, elx is %{public}d", userId, elx);
    if (elx == EL1_KEY) {
        LOGW("CreateElxBundleDataDir pass: userId %{public}u, elx is %{public}d", userId, elx);
        return;
    }
    StorageManagerClient client;
    auto ret = client.NotifyCreateBundleDataDirWithEl(userId, elx);
    if (ret != E_OK) {
        StorageRadar::ReportBundleMgrResult("CreateElxBundleDataDir", ret, userId, std::to_string(elx));
    }
}

int32_t UserManager::CheckUserIdRange(int32_t userId)
{
    if ((userId < StorageService::START_USER_ID && userId != StorageService::ZERO_USER) ||
        userId > StorageService::MAX_USER_ID) {
        LOGE("UserManager: userId:%{public}d is out of range", userId);
        return E_USERID_RANGE;
    }
    return E_OK;
}

int32_t UserManager::RestoreconSystemServiceDirs(int32_t userId)
{
#ifdef USE_LIBRESTORECON
    uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL2 | IStorageDaemonEnum::CRYPTO_FLAG_EL3 |
        IStorageDaemonEnum::CRYPTO_FLAG_EL4 | IStorageDaemonEnum::CRYPTO_FLAG_EL5;
    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetUserServicePath(userId, flags, dirInfoList.data);
    if (ret != E_OK) {
        return ret;
    }
    for (const auto &dirInfo : dirInfoList.data) {
        auto it = dirInfo.options.find("restorecon");
        if (it == dirInfo.options.end()) {
            continue;
        }
        auto startTime = StorageService::StorageRadar::RecordCurrentTime();
        RestoreconRecurse(dirInfo.path.c_str());
        auto delay = StorageService::StorageRadar::ReportDuration("RestoreconRecurse", startTime,
            StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USER_ID);
        LOGI("delay = %{public}s, path = %{public}s ", delay.c_str(), dirInfo.path.c_str());
    }
#endif
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
