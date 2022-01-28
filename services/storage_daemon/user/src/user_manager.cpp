/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "crypto/key_manager.h"
#include "ipc/istorage_daemon.h"
#include "parameter.h"
#include "user/user_manager.h"
#include "utils/errno.h"
#include "utils/file_utils.h"
#include "utils/log.h"
#include "utils/mount_argument_utils.h"
#include "utils/string_utils.h"
#include <stdlib.h>
#include <sys/mount.h>

using namespace std;

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t UMOUNT_RETRY_TIMES = 3;
std::shared_ptr<UserManager> UserManager::instance_ = nullptr;

const std::string HMDFS_SYS_CAP = "const.distributed_file_property.enabled";
const int32_t HMDFS_VAL_LEN = 6;
UserManager::UserManager()
    : rootDirVec_{{"/data/app/%s/%d", 0711, OID_ROOT, OID_ROOT},
                  {"/data/service/%s/%d", 0711, OID_ROOT, OID_ROOT},
                  {"/data/chipset/%s/%d", 0711, OID_ROOT, OID_ROOT}},
      subDirVec_{{"/data/app/%s/%d/base", 0711, OID_ROOT, OID_ROOT},
                 {"/data/app/%s/%d/database", 0711, OID_ROOT, OID_ROOT}},
      hmdfsDirVec_{{"/data/service/el2/%d/hmdfs", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account/files", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account/data", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account/cache", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account/files", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account/data", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account/cache", 0711, OID_SYSTEM, OID_SYSTEM}},
      virtualDir_{{"/storage/media/%d", 0711, OID_ROOT, OID_ROOT},
                  {"/storage/media/%d/local", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/account", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/non_account", 0711, OID_ROOT, OID_ROOT}}
{
}

std::shared_ptr<UserManager> UserManager::GetInstance()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() mutable { instance_ = std::make_shared<UserManager>(); });

    return instance_;
}

int32_t UserManager::HmdfsMount(int32_t userId)
{
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, true));
    int ret = Mount(StringPrintf(hmdfsSrc_.c_str(), userId), StringPrintf(hmdfsDest_.c_str(), userId), "hmdfs",
                    hmdfsMntArgs.GetFlags(), hmdfsMntArgs.OptionsToString().c_str());
    if (ret == -1 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to mount hmdfs, err %{public}d", errno);
        return E_MOUNT;
    }

    Utils::MountArgument hmdfsAuthMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, false));
    ret = Mount(StringPrintf(hmdfAuthSrc_.c_str(), userId), StringPrintf(hmdfsAuthDest_.c_str(), userId), "hmdfs",
                hmdfsAuthMntArgs.GetFlags(), hmdfsAuthMntArgs.OptionsToString().c_str());
    if (ret == -1 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to mount auth group hmdfs, err %{public}d", errno);
        return E_MOUNT;
    }

    // bind mount
    ret = Mount(StringPrintf((hmdfsDest_ + "device_view/").c_str(), userId), StringPrintf(ComDataDir_.c_str(), userId),
                nullptr, MS_BIND, nullptr);
    if (ret == -1 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to bind mount, err %{public}d", errno);
        return E_MOUNT;
    }

    return E_OK;
}

int32_t UserManager::HmdfsUnMount(int32_t userId)
{
    int32_t err = E_OK;
    // un bind mount
    err = UMount(StringPrintf(ComDataDir_.c_str(), userId));
    if (err != E_OK) {
        LOGE("failed to un bind mount, errno %{public}d, ComDataDir_ dst %{public}s", err,
             StringPrintf(ComDataDir_.c_str(), userId).c_str());
    }

    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, true));
    err = UMount2(hmdfsMntArgs.GetFullDst().c_str(), MNT_DETACH);
    if (err != E_OK) {
        LOGE("identical account hmdfs umount failed, errno %{public}d, hmdfs dst %{public}s", err,
             hmdfsMntArgs.GetFullDst().c_str());
    }

    Utils::MountArgument hmdfsAuthMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, false));
    err += UMount2(hmdfsAuthMntArgs.GetFullDst().c_str(), MNT_DETACH);
    if (err != E_OK) {
        LOGE("umount auth hmdfs, errno %{public}d, auth hmdfs dst %{public}s", err,
             hmdfsAuthMntArgs.GetFullDst().c_str());
    }

    return err;
}

bool UserManager::SupportHmdfs()
{
    char hmdfsEnable[HMDFS_VAL_LEN + 1] = {"false"};
    int ret = GetParameter(HMDFS_SYS_CAP.c_str(), "", hmdfsEnable, HMDFS_VAL_LEN);
    LOGI("GetParameter hmdfsEnable %{public}s, ret %{public}d", hmdfsEnable, ret);
    if (strcmp(hmdfsEnable, "true") == 0) {
        return true;
    }
    return false;
}

int32_t UserManager::LocalMount(int32_t userId)
{
    if (Mount(StringPrintf((hmdfsSrc_ + "files/").c_str(), userId),
              StringPrintf((ComDataDir_ + "local/").c_str(), userId), nullptr, MS_BIND, nullptr)) {
        LOGE("failed to bind mount, err %{public}d", errno);
        return E_MOUNT;
    }
    return E_OK;
}

int32_t UserManager::StartUser(int32_t userId)
{
    LOGI("start user %{public}d", userId);
    if (CreateVirtualDirs(userId) != E_OK) {
        LOGE("create hmdfs virtual dir error");
        return E_PREPARE_DIR;
    }

    if (!SupportHmdfs()) {
        return LocalMount(userId);
    } else {
        return HmdfsMount(userId);
    }

    return E_OK;
}

int32_t UserManager::LocalUnMount(int32_t userId)
{
    return UMount(StringPrintf((ComDataDir_ + "local/").c_str(), userId));
}

int32_t UserManager::StopUser(int32_t userId)
{
    LOGI("stop user %{public}d", userId);

    int32_t count = 0;
    while (count < UMOUNT_RETRY_TIMES) {
        int32_t err = E_OK;
        if (!SupportHmdfs()) {
            err = LocalUnMount(userId);
        } else {
            err = HmdfsUnMount(userId);
        }
        if (err == E_OK) {
            break;
        } else if (errno == EBUSY) {
            count++;
            continue;
        } else {
            LOGE("failed to umount, errno %{public}d", errno);
            return E_UMOUNT;
        }
    }

    return E_OK;
}

int32_t UserManager::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("prepare user dirs for %{public}d, flags %{public}u", userId, flags);

    int32_t err = E_OK;

    if (flags & IStorageDaemon::CRYPTO_FLAG_EL1) {
        err = PrepareDirsFromIdAndLevel(userId, el1_);
        if (err != E_OK) {
            return err;
        }
    }

    if (flags & IStorageDaemon::CRYPTO_FLAG_EL2) {
        err = PrepareDirsFromIdAndLevel(userId, el2_);
        if (err != E_OK) {
            return err;
        }

        err = PrepareHmdfsDirs(userId);
        if (err != E_OK) {
            LOGE("Prepare hmdfs dir error");
            return err;
        }
    }

    return E_OK;
}

int32_t UserManager::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    LOGI("destroy user dirs for %{public}d, flags %{public}u", userId, flags);

    int32_t err = E_OK;

    if (flags & IStorageDaemon::CRYPTO_FLAG_EL1) {
        err = DestroyDirsFromIdAndLevel(userId, el1_);
    }

    if (flags & IStorageDaemon::CRYPTO_FLAG_EL2) {
        err = DestroyDirsFromIdAndLevel(userId, el2_);
    }

    err = DestroyHmdfsDirs(userId);

    return err;
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
            err &= RmDirRecurse(StringPrintf(dir.path.c_str(), level.c_str(), userId));
        }
    }

    return err;
}

int32_t UserManager::PrepareDirsFromIdAndLevel(int32_t userId, const std::string &level)
{
    if (!PrepareDirsFromVec(userId, level, rootDirVec_)) {
        LOGE("failed to prepare %{public}s root dirs for userid %{public}d", level.c_str(), userId);
        return E_PREPARE_DIR;
    }

    // set policy here
    std::vector<FileList> list;
    for (auto item : rootDirVec_) {
        FileList temp;
        temp.userId = userId;
        temp.path = StringPrintf(item.path.c_str(), level.c_str(), userId);
        list.push_back(temp);
    }
    if (level == el1_) {
        if (KeyManager::GetInstance()->SetDirectoryElPolicy(userId, EL1_KEY, list)) {
            LOGE("Set user dir el1 policy error");
            return E_SET_POLICY;
        }
    } else if (level == el2_) {
        if (KeyManager::GetInstance()->SetDirectoryElPolicy(userId, EL2_KEY, list)) {
            LOGE("Set user dir el1 policy error");
            return E_SET_POLICY;
        }
    }

    if (!PrepareDirsFromVec(userId, level, subDirVec_)) {
        LOGE("failed to prepare %{public}s sub dirs for userid %{public}d", level.c_str(), userId);
        return E_PREPARE_DIR;
    }

    return E_OK;
}

int32_t UserManager::DestroyDirsFromIdAndLevel(int32_t userId, const std::string &level)
{
    if (!DestroyDirsFromVec(userId, level, rootDirVec_)) {
        LOGE("failed to destroy %{public}s dirs for userid %{public}d", level.c_str(), userId);
        return E_DESTROY_DIR;
    }

    return E_OK;
}

int32_t UserManager::PrepareHmdfsDirs(int32_t userId)
{
    for (const DirInfo &dir : hmdfsDirVec_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::CreateVirtualDirs(int32_t userId)
{
    for (const DirInfo &dir : virtualDir_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t UserManager::DestroyHmdfsDirs(int32_t userId)
{
    bool err = true;

    for (const DirInfo &dir : hmdfsDirVec_) {
        if (IsEndWith(dir.path.c_str(), "%d")) {
            err &= RmDirRecurse(StringPrintf(dir.path.c_str(), userId));
        }
    }

    return err ? E_OK : E_DESTROY_DIR;
}
} // namespace StorageDaemon
} // namespace OHOS