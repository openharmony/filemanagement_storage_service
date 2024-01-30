/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "quota/quota_manager.h"

#include <cstdint>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <linux/dqblk_xfs.h>
#include <linux/fs.h>
#include <linux/quota.h>
#include <map>
#include <sstream>
#include <stack>
#include <sys/ioctl.h>
#include <sys/quota.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <tuple>
#include <unique_fd.h>
#include <unistd.h>

#include "file_uri.h"
#include "sandbox_helper.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
const std::string QUOTA_DEVICE_DATA_PATH = "/data";
const std::string PROC_MOUNTS_PATH = "/proc/mounts";
const std::string DEV_BLOCK_PATH = "/dev/block/";
const int32_t DEV_BLOCK_PATH_LEN = DEV_BLOCK_PATH.length();
const uint64_t ONE_KB = int64_t(1);
const uint64_t ONE_MB = int64_t(1024 * ONE_KB);
static std::map<std::string, std::string> mQuotaReverseMounts;
std::recursive_mutex mMountsLock;

QuotaManager* QuotaManager::instance_ = nullptr;
QuotaManager* QuotaManager::GetInstance()
{
    if (instance_ == nullptr) {
        instance_ = new QuotaManager();
    }

    return instance_;
}

static bool InitialiseQuotaMounts()
{
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);
    mQuotaReverseMounts.clear();
    std::ifstream in(PROC_MOUNTS_PATH);

    if (!in.is_open()) {
        LOGE("Failed to open mounts file");
        return false;
    }
    std::string source;
    std::string target;
    std::string ignored;

    while (!in.eof()) {
        std::getline(in, source, ' ');
        std::getline(in, target, ' ');
        std::getline(in, ignored);
        if (source.compare(0, DEV_BLOCK_PATH_LEN, DEV_BLOCK_PATH) == 0) {
            struct dqblk dq;
            if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), source.c_str(), 0, reinterpret_cast<char*>(&dq)) == 0) {
                mQuotaReverseMounts[target] = source;
            }
        }
    }

    return true;
}

static int64_t GetOccupiedSpaceForUid(int32_t uid, int64_t &size)
{
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_SYS_ERR;
    }

    std::string device = "";
    device = mQuotaReverseMounts[QUOTA_DEVICE_DATA_PATH];
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_SYS_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    return E_OK;
}

static int64_t GetOccupiedSpaceForGid(int32_t gid, int64_t &size)
{
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_SYS_ERR;
    }

    std::string device = "";
    device = mQuotaReverseMounts[QUOTA_DEVICE_DATA_PATH];
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, GRPQUOTA), device.c_str(), gid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_SYS_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    return E_OK;
}


static int64_t GetOccupiedSpaceForPrjId(int32_t prjId, int64_t &size)
{
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_SYS_ERR;
    }

    std::string device = "";
    device = mQuotaReverseMounts[QUOTA_DEVICE_DATA_PATH];
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, PRJQUOTA), device.c_str(), prjId, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_SYS_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    return E_OK;
}

int32_t QuotaManager::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    switch (idType) {
        case USRID:
            return GetOccupiedSpaceForUid(id, size);
            break;
        case GRPID:
            return GetOccupiedSpaceForGid(id, size);
            break;
        case PRJID:
            return GetOccupiedSpaceForPrjId(id, size);
            break;
        default:
            return E_NON_EXIST;
    }
    return E_OK;
}

int32_t QuotaManager::SetBundleQuota(const std::string &bundleName, int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    if (bundleName.empty() || bundleDataDirPath.empty() || uid < 0 || limitSizeMb <= 0) {
        LOGE("Calling the function PrepareBundleDirQuotaWithSize with invalid param");
        return E_NON_EXIST;
    }

    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_NON_EXIST;
    }

    std::string device = "";
    if (bundleDataDirPath.find(QUOTA_DEVICE_DATA_PATH) == 0) {
        device = mQuotaReverseMounts[QUOTA_DEVICE_DATA_PATH];
    }
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get hard quota, errno : %{public}d", errno);
        return E_SYS_CALL;
    }

    // dqb_bhardlimit is count of 1kB blocks, dqb_curspace is bytes
    struct statvfs stat;
    if (statvfs(bundleDataDirPath.c_str(), &stat) != 0) {
        LOGE("Failed to statvfs, errno : %{public}d", errno);
        return E_SYS_CALL;
    }

    dq.dqb_valid = QIF_LIMITS;
    dq.dqb_bhardlimit = (uint32_t)limitSizeMb * ONE_MB;
    if (quotactl(QCMD(Q_SETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to set hard quota, errno : %{public}d", errno);
        return E_SYS_CALL;
    } else {
        LOGE("Applied hard quotas ok");
        return E_OK;
    }
}

int32_t QuotaManager::SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit)
{
    struct fsxattr fsx;
    char *realPath = realpath(path.c_str(), NULL);
    if (realPath == NULL) {
        LOGE("realpath failed");
        return E_SYS_CALL;
    }

    int fd = open(realPath, O_RDONLY | O_CLOEXEC);
    free(realPath);
    if (fd < 0) {
        LOGE("Failed to open %{public}s, errno: %{public}d", path.c_str(), errno);
        return E_SYS_CALL;
    }
    if (ioctl(fd, FS_IOC_FSGETXATTR, &fsx) == -1) {
        LOGE("Failed to get extended attributes of %{public}s, errno: %{public}d", path.c_str(), errno);
        (void)close(fd);
        return E_SYS_CALL;
    }
    uint32_t uintprjId = static_cast<uint32_t>(prjId);
    if (fsx.fsx_projid == uintprjId) {
        return E_OK;
    }
    fsx.fsx_projid = static_cast<uint32_t>(prjId);
    if (ioctl(fd, FS_IOC_FSSETXATTR, &fsx) == -1) {
        LOGE("Failed to set project id for %{public}s, errno: %{public}d", path.c_str(), errno);
        (void)close(fd);
        return E_SYS_CALL;
    }

    if (inherit) {
        uint32_t flags;
        if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1) {
            LOGE("Failed to get flags for %{public}s, errno:%{public}d", path.c_str(), errno);
            (void)close(fd);
            return E_SYS_CALL;
        }
        flags |= FS_PROJINHERIT_FL;
        if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1) {
            LOGE("Failed to set flags for %{public}s, errno:%{public}d", path.c_str(), errno);
            (void)close(fd);
            return E_SYS_CALL;
        }
    }
    return E_OK;
}

int32_t QuotaManager::GetBundleStatsForIncrease(uint32_t userId, const std::vector<std::string> &bundleNames,
    const std::vector<int64_t> &incrementalBackTimes, std::vector<int64_t> &pkgFileSizes)
{
    LOGI("GetBundleStatsForIncrease start");

    return E_OK;
}
} // StorageDaemon
} // OHOS
