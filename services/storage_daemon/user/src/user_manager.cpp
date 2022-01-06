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

#include "user/user_manager.h"
#include <stdlib.h>
#include "utils/errno.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "utils/log.h"
#include "ipc/istorage_daemon.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
constexpr int32_t UMOUNT_RETRY_TIMES = 3;
UserManager* UserManager::instance_ = nullptr;

UserManager::UserManager()
    : rootDirVec_{
        {"/data/app/%s/%d", 0711, OID_ROOT, OID_ROOT},
        {"/data/service/%s/%d", 0711, OID_ROOT, OID_ROOT},
        {"/data/chipset/%s/%d", 0711, OID_ROOT, OID_ROOT}
    },
    subDirVec_{
        {"/data/app/%s/%d/base", 0711, OID_ROOT, OID_ROOT},
        {"/data/app/%s/%d/database", 0711, OID_ROOT, OID_ROOT}
    },
    hmdfsDirVec_{
        {"/data/service/el2/%d/hmdfs", 0711, OID_SYSTEM, OID_SYSTEM},
        {"/data/service/el2/%d/hmdfs/files", 0711, OID_SYSTEM, OID_SYSTEM},
        {"/data/service/el2/%d/hmdfs/data", 0711, OID_SYSTEM, OID_SYSTEM},
        {"/storage/media/%d", 0711, OID_ROOT, OID_ROOT},
        {"/storage/media/%d/local", 0711, OID_ROOT, OID_ROOT}
    }
{}

UserManager* UserManager::Instance()
{
    if (instance_ == nullptr) {
        instance_ = new UserManager();
    }

    return instance_;
}

int32_t UserManager::StartUser(int32_t userId)
{
    LOGI("start user %{public}d", userId);

    // get syspara: hmdfs

    if ((Mount(StringPrintf(hmdfsSource_.c_str(), userId),
               StringPrintf(hmdfsTarget_.c_str(), userId),
               nullptr, MS_BIND, nullptr))) {
        LOGE("failed to mount, err %{public}d", errno);
        return E_MOUNT;
    }

    return E_OK;
}

int32_t UserManager::StopUser(int32_t userId)
{
    LOGI("stop user %{public}d", userId);

    // get syspara: hmdfs

    int32_t count = 0;
    while (count < UMOUNT_RETRY_TIMES) {
        int32_t err = UMount(StringPrintf(hmdfsTarget_.c_str(), userId));
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
    }

    // get syspara: hmdfs
    err = PrepareHmdfsDirs(userId);
    if (err != E_OK) {
        return err;
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

    // get syspara: hmdfs
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
} // StorageDaemon
} // OHOS