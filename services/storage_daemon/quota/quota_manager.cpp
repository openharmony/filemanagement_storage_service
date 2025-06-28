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

#include <dirent.h>
#include <fstream>
#include <linux/fs.h>
#include <linux/quota.h>
#include <stack>
#include <sys/quota.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <thread>
#include <unistd.h>

#include "file_uri.h"
#include "sandbox_helper.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *QUOTA_DEVICE_DATA_PATH = "/data";
constexpr const char *PROC_MOUNTS_PATH = "/proc/mounts";
constexpr const char *DEV_BLOCK_PATH = "/dev/block/";
constexpr const char *CONFIG_FILE_PATH = "/etc/passwd";
constexpr uint64_t ONE_KB = 1;
constexpr uint64_t ONE_MB = 1024 * ONE_KB;
constexpr int32_t FIVE_HANDRED_M_BIT = 1024 * 1024 * 500;
constexpr uint64_t PATH_MAX_LEN = 4096;
constexpr double DIVISOR = 1024.0 * 1024.0;
constexpr double BASE_NUMBER = 10.0;
constexpr int32_t ONE_MS = 1000;
constexpr int32_t ACCURACY_NUM = 2;
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

    while (in.peek() != EOF) {
        std::getline(in, source, ' ');
        std::getline(in, target, ' ');
        std::getline(in, ignored);
        if (source.compare(0, strlen(DEV_BLOCK_PATH), DEV_BLOCK_PATH) == 0) {
            struct dqblk dq;
            if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), source.c_str(), 0, reinterpret_cast<char*>(&dq)) == 0) {
                mQuotaReverseMounts[target] = source;
            }
        }
    }

    return true;
}

static std::string GetQuotaSrcMountPath(const std::string &target)
{
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);
    if (mQuotaReverseMounts.find(target) != mQuotaReverseMounts.end()) {
        return mQuotaReverseMounts[target];
    } else {
        return "";
    }
}

static int64_t GetOccupiedSpaceForUid(int32_t uid, int64_t &size)
{
    LOGE("GetOccupiedSpaceForUid uid:%{public}d", uid);
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    LOGE("GetOccupiedSpaceForUid size:%{public}s", std::to_string(size).c_str());
    return E_OK;
}

void QuotaManager::GetUidStorageStats()
{
    LOGI("GetUidStorageStats begin!");
    std::vector<UidSaInfo> vec;
    auto ret = ParseConfigFile(CONFIG_FILE_PATH, vec);
    if (ret != E_OK) {
        LOGE("parsePasswd File failed.");
        return;
    }
 
    ret = GetOccupiedSpaceForUidList(vec);
    if (ret != E_OK) {
        return;
    }
 
    std::sort(vec.begin(), vec.end(), [](const UidSaInfo& a, const UidSaInfo& b) {
    return a.size > b.size;
    });
    std::ostringstream extraData;
    for (const auto& info : vec) {
        if (info.size < FIVE_HANDRED_M_BIT) {
            continue;
        }
        extraData << "{uid:" << info.uid
            << ",saName:" << info.saName
            << ",size:" << ConvertBytesToMB(info.size, ACCURACY_NUM)
            << "MB}"<<std::endl;
    }
    LOGI("extraData is %{public}s", extraData.str().c_str());
    StorageService::StorageRadar::ReportSaSizeResult("QuotaManager::GetUidStorageStats", E_STORAGE_STATUS,
        extraData.str());
    LOGI("GetUidStorageStats end!");
}

double QuotaManager::ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces)
{
    if (bytes < 0) {
        return 0.0;
    }
    double mb = static_cast<double>(bytes) / DIVISOR;
 
    if (decimalPlaces < 0) {
        decimalPlaces = 0;
    }
    double factor = std::pow(BASE_NUMBER, decimalPlaces);
    if (factor == 0) {
        return 0.0;
    }
    return std::round(mb * factor) / factor;
}
 
bool QuotaManager::StringToInt32(const std::string &strUid, int32_t &outUid32)
{
    if (strUid.empty()) {
        return false;
    }
    for (char ch : strUid) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
 
    uint64_t uid = std::stoull(strUid);
    if (uid > static_cast<uint64_t>(INT32_MAX)) {
        return false;
    }
    outUid32 = static_cast<int32_t>(uid);
    return true;
}
 
bool QuotaManager::GetUid32FromEntry(const std::string &entry, int32_t &outUid32, std::string &saName)
{
    size_t firstColon = entry.find(':');
    if (firstColon == std::string::npos) {
        return false;
    }
    saName = entry.substr(0, firstColon);
    size_t secondColon = entry.find(':', firstColon + 1);
    if (secondColon == std::string::npos) {
        return false;
    }
    size_t thirdColon = entry.find(':', secondColon + 1);
    if (thirdColon == std::string::npos) {
        return false;
    }
    std::string uidStr = entry.substr(secondColon + 1, thirdColon - (secondColon + 1));
    return StringToInt32(uidStr, outUid32);
}
 
int32_t QuotaManager::ParseConfigFile(const std::string &path, std::vector<struct UidSaInfo> &vec)
{
    LOGI("pasePasswdFile begin!");
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("path not valid, path = %{private}s", path.c_str());
        return E_JSON_PARSE_ERROR;
    }
 
    std::ifstream infile(std::string(realPath), std::ios::in);
    if (!infile.is_open()) {
        LOGE("Open file failed, errno = %{public}d", errno);
        return E_OPEN_JSON_FILE_ERROR;
    }
 
    std::string line;
    while (getline(infile, line)) {
        if (line == "") {
            continue;
        }
        struct UidSaInfo info;
        if (GetUid32FromEntry(line, info.uid, info.saName)) {
            vec.push_back(info);
        }
    }
    infile.close();
    LOGI("pasePasswdFile end!");
    return E_OK;
}
 
int64_t QuotaManager::GetOccupiedSpaceForUidList(std::vector<struct UidSaInfo> &vec)
{
    LOGE("GetOccupiedSpaceForUidList begin!");
 
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_SYS_KERNEL_ERR;
    }
 
    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }
 
    for (struct UidSaInfo &info : vec) {
        struct dqblk dq;
        usleep(ONE_MS);
        if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), info.uid, reinterpret_cast<char*>(&dq)) != 0) {
            LOGE("Failed to get quotactl, errno : %{public}d", errno);
            continue;
        }
        info.size = static_cast<int64_t>(dq.dqb_curspace);
    }
    return E_OK;
}

static int64_t GetOccupiedSpaceForGid(int32_t gid, int64_t &size)
{
    LOGE("GetOccupiedSpaceForGid gid:%{public}d", gid);
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, GRPQUOTA), device.c_str(), gid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    LOGE("GetOccupiedSpaceForGid size:%{public}s", std::to_string(size).c_str());
    return E_OK;
}


static int64_t GetOccupiedSpaceForPrjId(int32_t prjId, int64_t &size)
{
    LOGE("GetOccupiedSpaceForPrjId prjId:%{public}d", prjId);
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, PRJQUOTA), device.c_str(), prjId, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get quotactl, errno : %{public}d", errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    LOGE("GetOccupiedSpaceForPrjId size:%{public}s", std::to_string(size).c_str());
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
    if (bundleName.empty() || bundleDataDirPath.empty() || uid < 0 || limitSizeMb < 0) {
        LOGE("Calling the function SetBundleQuota with invalid param");
        return E_PARAMS_INVALID;
    }

    LOGE("SetBundleQuota Start, bundleName is %{public}s, uid is %{public}d, bundleDataDirPath is %{public}s, "
         "limit is %{public}d.", bundleName.c_str(), uid, bundleDataDirPath.c_str(), limitSizeMb);
    if (InitialiseQuotaMounts() != true) {
        LOGE("Failed to initialise quota mounts");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    if (bundleDataDirPath.find(QUOTA_DEVICE_DATA_PATH) == 0) {
        device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    }
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to get hard quota, errno : %{public}d", errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    // dqb_bhardlimit is count of 1kB blocks, dqb_curspace is bytes
    struct statvfs stat;
    if (statvfs(bundleDataDirPath.c_str(), &stat) != 0) {
        LOGE("Failed to statvfs, errno : %{public}d", errno);
        return E_STAT_VFS_KERNEL_ERR;
    }

    dq.dqb_valid = QIF_LIMITS;
    dq.dqb_bhardlimit = (uint32_t)limitSizeMb * ONE_MB;
    if (quotactl(QCMD(Q_SETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("Failed to set hard quota, errno : %{public}d", errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    } else {
        LOGD("Applied hard quotas ok");
        return E_OK;
    }
}

int32_t QuotaManager::SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit)
{
    struct fsxattr fsx;
    char *realPath = realpath(path.c_str(), nullptr);
    if (realPath == nullptr) {
        LOGE("realpath failed");
        return E_PARAMS_NULLPTR_ERR;
    }
    FILE *f = fopen(realPath, "r");
    free(realPath);
    if (f == nullptr) {
        LOGE("Failed to open %{public}s, errno: %{public}d", path.c_str(), errno);
        return E_SYS_KERNEL_ERR;
    }
    int fd = fileno(f);
    if (fd < 0) {
        (void)fclose(f);
        return E_SYS_KERNEL_ERR;
    }
    if (ioctl(fd, FS_IOC_FSGETXATTR, &fsx) == -1) {
        LOGE("Failed to get extended attributes of %{public}s, errno: %{public}d", path.c_str(), errno);
        (void)fclose(f);
        return E_SYS_KERNEL_ERR;
    }
    if (fsx.fsx_projid == static_cast<uint32_t>(prjId)) {
        (void)fclose(f);
        return E_OK;
    }
    fsx.fsx_projid = static_cast<uint32_t>(prjId);
    if (ioctl(fd, FS_IOC_FSSETXATTR, &fsx) == -1) {
        LOGE("Failed to set project id for %{public}s, errno: %{public}d", path.c_str(), errno);
        (void)fclose(f);
        return E_SYS_KERNEL_ERR;
    }
    if (inherit) {
        uint32_t flags;
        if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1) {
            LOGE("Failed to get flags for %{public}s, errno:%{public}d", path.c_str(), errno);
            (void)fclose(f);
            return E_SYS_KERNEL_ERR;
        }
        flags |= FS_PROJINHERIT_FL;
        if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1) {
            LOGE("Failed to set flags for %{public}s, errno:%{public}d", path.c_str(), errno);
            (void)fclose(f);
            return E_SYS_KERNEL_ERR;
        }
    }
    (void)fclose(f);
    return E_OK;
}

uint32_t CheckOverLongPath(const std::string &path)
{
    uint32_t len = path.length();
    if (len >= PATH_MAX_LEN) {
        size_t found = path.find_last_of('/');
        std::string sub = path.substr(found + 1);
        LOGE("Path over long, length:%{public}d, fileName:%{public}s.", len, sub.c_str());
    }
    return len;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
