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

#include <charconv>
#include <chrono>
#include <ctime>
#include <dirent.h>
#include <linux/fs.h>
#include <linux/quota.h>
#include <stack>
#include <sys/quota.h>
#include <thread>
#include <unistd.h>

#include "file_uri.h"
#include "sandbox_helper.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *QUOTA_DEVICE_DATA_PATH = "/data";
constexpr const char *PROC_MOUNTS_PATH = "/proc/mounts";
constexpr const char *DEV_BLOCK_PATH = "/dev/block/";
constexpr const char *CONFIG_FILE_PATH = "/etc/passwd";
constexpr const char *DATA_DEV_PATH = "/dev/block/by-name/userdata";
constexpr uint64_t FOUR_K = 4096;
constexpr uint64_t ONE_KB = 1;
constexpr uint64_t ONE_MB = 1024 * ONE_KB;
constexpr uint64_t PATH_MAX_LEN = 4096;
constexpr double DIVISOR = 1000.0 * 1000.0;
constexpr double BASE_NUMBER = 10.0;
constexpr int32_t ONE_MS = 1000;
constexpr int32_t ACCURACY_NUM = 2;
constexpr int32_t MAX_UID_COUNT = 100000;
constexpr int32_t BLOCK_BYTE = 512;
constexpr int32_t TOP_SPACE_COUNT = 20;
constexpr int32_t LINE_MAX_LEN = 32;
static std::map<std::string, std::string> mQuotaReverseMounts;
static std::vector<int32_t> SYS_UIDS = {0, 1000, 5523};
#define Q_GETNEXTQUOTA_LOCAL 0x800009
std::recursive_mutex mMountsLock;
std::mutex cacheMutex_;

QuotaManager &QuotaManager::GetInstance()
{
    static QuotaManager instance_;
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
    struct dqblk dq;
#ifdef ENABLE_EMULATOR
    if (!InitialiseQuotaMounts()) {
        LOGE("Failed to initialise quota mounts");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }
    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGE("skip when device no quotas present");
        return E_OK;
    }
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) == 0) {
        size = static_cast<int64_t>(dq.dqb_curspace);
        LOGE("get size for emulator by quota success, size is %{public}s", std::to_string(size).c_str());
        return E_OK;
    }
    LOGE("get size for emulator by quota failed, errno is %{public}d", errno);
#else
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), DATA_DEV_PATH, uid, reinterpret_cast<char*>(&dq)) == 0) {
        size = static_cast<int64_t>(dq.dqb_curspace);
        LOGE("get size by quota success, size is %{public}s", std::to_string(size).c_str());
        return E_OK;
    }
    LOGE("get size by quota failed, errno is %{public}d", errno);
#endif
    return E_QUOTA_CTL_KERNEL_ERR;
}

void QuotaManager::GetUidStorageStats(std::string &storageStatus,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    LOGI("GetUidStorageStats begin!");
    struct AllAppVec allVec;
    auto ret = ParseConfigFile(CONFIG_FILE_PATH, allVec.sysSaVec);
    if (ret != E_OK) {
        LOGE("parsePasswd File failed.");
        return;
    }
    uint64_t iNodes;
    GetOccupiedSpaceForUidList(allVec, iNodes);

    std::ostringstream extraData;

    extraData << "{iNodes count is:" << iNodes << ",iNodes size is:" <<
    ConvertBytesToMB(iNodes * FOUR_K, ACCURACY_NUM) <<"MB}" << std::endl;

    GetSaOrOtherTotal(allVec.sysSaVec, extraData, true);

    if (!allVec.otherAppVec.empty()) {
        GetSaOrOtherTotal(allVec.otherAppVec, extraData, false);
    }
    ProcessVecList(allVec, bundleNameAndUid);
    extraData << "{Sa data is:}" << std::endl;
    WriteExtraData(allVec.sysSaVec, extraData);
    extraData << "{SysApp data is:}" << std::endl;
    WriteExtraData(allVec.sysAppVec, extraData);
    extraData << "{UserApp data is:}" << std::endl;
    WriteExtraData(allVec.userAppVec, extraData);
    if (!allVec.otherAppVec.empty()) {
        extraData << "{otherAppVec data is:}" << std::endl;
        WriteExtraData(allVec.otherAppVec, extraData);
    } else {
        extraData << "{otherAppVec data is null}" << std::endl;
    }
    storageStatus = extraData.str();
    LOGI("extraData is %{public}s", extraData.str().c_str());
    LOGI("GetUidStorageStats end!");
}

void QuotaManager::GetSaOrOtherTotal(const std::vector<UidSaInfo> &vec, std::ostringstream &extraData, bool isSaVec)
{
    int64_t totalSize = 0;
    for (const auto &info : vec) {
        totalSize += info.size;
    }
    if (isSaVec) {
        extraData << "{sa totalSize is:" << ConvertBytesToMB(totalSize, ACCURACY_NUM) << "MB}" << std::endl;
        return;
    }
    extraData << "{other totalSize is:" << ConvertBytesToMB(totalSize, ACCURACY_NUM) << "MB}" << std::endl;
}

int32_t QuotaManager::GetFileData(const std::string &path, int64_t &size)
{
    if (path.empty() || path.size() >= PATH_MAX) {
        return E_FILE_PATH_INVALID;
    }

    char realPath[PATH_MAX] = {0x00};
    if (!realpath(path.c_str(), realPath)) {
        return E_FILE_PATH_INVALID;
    }

    // 确保规范化后的路径在预期范围内
    std::string normalizedPath(realPath);
    if (normalizedPath != path) {
        return E_FILE_PATH_INVALID;
    }

    std::ifstream infile(normalizedPath, std::ios::in);
    if (!infile.is_open()) {
        return E_OPEN_JSON_FILE_ERROR;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) {
            continue;
        }
        // 添加长度限制
        if (line.size() > LINE_MAX_LEN) {
            return E_NON_ACCESS;
        }

        int64_t listNum = 0;
        if (StringToInt64(line, listNum)) {
            // 检查加法溢出
            if (size > INT64_MAX - listNum) {
                return E_NON_ACCESS;
            }
            size += listNum;
        }
    }
    return E_OK;
}

bool QuotaManager::StringToInt64(const std::string& str, int64_t& out_value)
{
    if (str.empty() || str.size() > 20) { // 20是INT64_MAX的字符串长度
        LOGE("Invalid input length");
        return false;
    }
    auto result = std::from_chars(str.data(), str.data() + str.size(), out_value);
    if (result.ec == std::errc::invalid_argument) {
        LOGE("Invalid argument");
        return false;
    }

    if (result.ec == std::errc::result_out_of_range) {
        LOGE("Integer overflow");
        return false;
    }

    if (result.ptr != str.data() + str.size()) {
        LOGE("The string contains invalid characters");
        return false;
    }

    return true;
}

void QuotaManager::GetCurrentTime(std::ostringstream &extraData)
{
    auto now = std::chrono::system_clock::now();
    auto timeStamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    struct tm timeInfo;
    localtime_r(&timeT, &timeInfo);
    std::ostringstream timeStr;
    timeStr << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S");
    extraData << "{timeStamp is:" << timeStamp
              << "MS,BeiJingTime is:" << timeStr.str() << "}" << std::endl;
}

void QuotaManager::WriteExtraData(const std::vector<UidSaInfo> &vec, std::ostringstream &extraData)
{
    for (const auto& info : vec) {
        extraData << "{uid:" << info.uid
            << ",saName:" << info.saName
            << ",size:" << ConvertBytesToMB(info.size, ACCURACY_NUM)
            << "MB, iNodes:" << info.iNodes << "}" << std::endl;
    }
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

    uint64_t uid;
    auto res = std::from_chars(strUid.data(), strUid.data() + strUid.size(), uid);
    if (res.ec != std::errc()) {
        return false;
    }
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

void QuotaManager::ProcessVecList(struct AllAppVec &allVec, const std::map<int32_t, std::string> &bundleNameAndUid)
{
    SortAndCutSaInfoVec(allVec.sysAppVec);
    SortAndCutSaInfoVec(allVec.userAppVec);
    SortAndCutSaInfoVec(allVec.otherAppVec);
    SortAndCutSaInfoVec(allVec.sysSaVec);
    AssembleSaInfoVec(allVec.otherAppVec, bundleNameAndUid);
    AssembleSaInfoVec(allVec.sysAppVec, bundleNameAndUid);
    AssembleSaInfoVec(allVec.userAppVec, bundleNameAndUid);
}

void QuotaManager::AssembleSaInfoVec(std::vector<UidSaInfo> &vec,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    if (bundleNameAndUid.empty()) {
        return;
    }
    for (UidSaInfo &saInfo : vec) {
        auto it = bundleNameAndUid.find(saInfo.uid);
        if (it != bundleNameAndUid.end()) {
            saInfo.saName = it->second;
        }
    }
}


void QuotaManager::SortAndCutSaInfoVec(std::vector<struct UidSaInfo> &vec)
{
    std::sort(vec.begin(), vec.end(), [](const UidSaInfo& a, const UidSaInfo& b) {
        return a.size > b.size;
    });
    vec.erase(vec.begin() + std::min(static_cast<size_t>(TOP_SPACE_COUNT), vec.size()), vec.end());
}

void QuotaManager::GetOccupiedSpaceForUidList(struct AllAppVec &allVec, uint64_t &iNodes)
{
    LOGI("GetOccupiedSpaceForUidList begin!");
    int32_t curUid = 0;
    int32_t count = 0;
    std::map<int32_t, int64_t> userAppSizeMap;
    while (count < MAX_UID_COUNT) {
        NextDqBlk dq;
        if (quotactl(QCMD(Q_GETNEXTQUOTA_LOCAL, USRQUOTA), DATA_DEV_PATH, curUid, reinterpret_cast<char*>(&dq)) != 0) {
            LOGE("failed to get next quota, uid is %{public}d, errno is %{public}d,", curUid, errno);
            break;
        }
        int32_t dqUid = static_cast<int32_t>(dq.dqbId);
        bool isSaUid = false;
        iNodes += dq.dqbCurInodes;
        for (struct UidSaInfo &info : allVec.sysSaVec) {
            if (info.uid == dqUid) {
                isSaUid = true;
                info.size = static_cast<int64_t>(dq.dqbCurSpace);
                info.iNodes = dq.dqbCurInodes;
                break;
            }
        }
        if (dqUid >= StorageService::APP_UID) {
            int32_t userId = dqUid / StorageService::USER_ID_BASE;
            if (userAppSizeMap.find(userId) != userAppSizeMap.end()) {
                userAppSizeMap[userId] += static_cast<int64_t>(dq.dqbCurSpace);
            } else {
                userAppSizeMap[userId] = static_cast<int64_t>(dq.dqbCurSpace);
            }
            allVec.userAppVec.push_back(UidSaInfo(dqUid, "", static_cast<int64_t>(dq.dqbCurSpace), dq.dqbCurInodes));
        } else if (dqUid >= StorageService::ZERO_USER_MIN_UID && dqUid <= StorageService::ZERO_USER_MAX_UID) {
            AssembleSysAppVec(dqUid, dq, userAppSizeMap, allVec.sysAppVec);
        } else if (!isSaUid) {
            allVec.otherAppVec.push_back(UidSaInfo(dqUid, "", static_cast<int64_t>(dq.dqbCurSpace), dq.dqbCurInodes));
        }
        count++;
        curUid = dqUid + 1;
        if (curUid == 0) {
            break;
        }
        usleep(ONE_MS);
    }
    for (const auto &pair : userAppSizeMap) {
        UidSaInfo info = {pair.first, "userId", pair.second};
        allVec.sysSaVec.push_back(info);
    }
    LOGI("GetOccupiedSpaceForUidList end!");
}

void QuotaManager::AssembleSysAppVec(int32_t dqUid, const NextDqBlk &dq,
    std::map<int32_t, int64_t> &userAppSizeMap, std::vector<struct UidSaInfo> &sysAppVec)
{
    int32_t userId = StorageService::ZERO_USER;
    if (userAppSizeMap.find(userId) != userAppSizeMap.end()) {
        userAppSizeMap[userId] += static_cast<int64_t>(dq.dqbCurSpace);
    } else {
        userAppSizeMap[userId] = static_cast<int64_t>(dq.dqbCurSpace);
    }
    sysAppVec.push_back(UidSaInfo(dqUid, "", static_cast<int64_t>(dq.dqbCurSpace), dq.dqbCurInodes));
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

int32_t QuotaManager::SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    if (bundleDataDirPath.empty() || uid < 0 || limitSizeMb < 0) {
        LOGE("Calling the function SetBundleQuota with invalid param");
        return E_PARAMS_INVALID;
    }

    LOGE("SetBundleQuota Start, uid is %{public}d, bundleDataDirPath is %{public}s, "
         "limit is %{public}d.", uid, bundleDataDirPath.c_str(), limitSizeMb);
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

int32_t QuotaManager::AddBlksRecurse(const std::string &path, int64_t &blks, uid_t uid)
{
    AddBlks(path, blks, uid);
    if (!IsDir(path)) {
        return E_OK;
    }
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("open dir %{public}s failed, errno %{public}d", path.c_str(), errno);
        return E_STATISTIC_OPEN_DIR_FAILED;
    }
    int ret = E_OK;
    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }
        std::string subPath = path + "/" + ent->d_name;
        int32_t retTmp = AddBlksRecurse(subPath, blks, uid);
        if (retTmp != E_OK) {
            ret = retTmp;
        }
    }
    (void)closedir(dir);
    return ret;
}

int32_t QuotaManager::AddBlks(const std::string &path, int64_t &blks, uid_t uid)
{
    struct stat st;
    if (lstat(path.c_str(), &st) != E_OK) {
        int32_t errnoTmp = errno;
        std::string extraData = "path=" + path + ",kernelCode=" + std::to_string(errnoTmp);
        StorageService::StorageRadar::ReportSpaceRadar("AddBlks", E_STATISTIC_STAT_FAILED, extraData);
        LOGE("lstat failed, path is %{public}s, errno is %{public}d", path.c_str(), errno);
        return E_STATISTIC_STAT_FAILED;
    }
    if (uid == st.st_uid) {
        blks += static_cast<int64_t>(st.st_blocks);
    }
    return E_OK;
}

int32_t QuotaManager::GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks)
{
    LOGI("GetDqBlkSpacesByUids start, uids size: %{public}zu", uids.size());
    dqBlks.clear();
    for (auto &uid : uids) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            LOGI("GetDqBlkSpacesByUids stopped by stopScanFlg");
            std::vector<NextDqBlk>().swap(dqBlks);
            return E_ERR;
        }
        struct dqblk dq;
#ifdef ENABLE_EMULATOR
        return E_NOT_SUPPORT;
#else
        if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), DATA_DEV_PATH, uid, reinterpret_cast<char *>(&dq)) != 0) {
            LOGE("get size by quota failed, size is %{public}s", std::to_string(dq.dqb_curspace).c_str());
            std::vector<NextDqBlk>().swap(dqBlks);
            return E_ERR;
        }
#endif
        // 将 dqblk 转换为 NextDqBlk 对象
        NextDqBlk nextDq(dq.dqb_bhardlimit, dq.dqb_bsoftlimit, dq.dqb_curspace, dq.dqb_ihardlimit, dq.dqb_isoftlimit,
                         dq.dqb_curinodes, dq.dqb_btime, dq.dqb_itime, dq.dqb_valid,
                         uid);
        dqBlks.push_back(nextDq);
    }
    LOGI("GetDqBlkSpacesByUids end, dqBlks size: %{public}zu", dqBlks.size());
    return E_OK;
}

void QuotaManager::ProcessSingleDir(const DirSpaceInfo &dirInfo, std::vector<DirSpaceInfo> &resultDirs)
{
    std::string path = dirInfo.path;
    uid_t uid = dirInfo.uid;
    int64_t blks = 0;
    AddBlksRecurse(path, blks, uid);
    int64_t dirSize = blks * BLOCK_BYTE;
    resultDirs.push_back({path, uid, dirSize});
}

void QuotaManager::ProcessDirWithUserId(const DirSpaceInfo &dirInfo, const std::vector<int32_t> &userIds,
    std::vector<DirSpaceInfo> &resultDirs)
{
    std::string path = dirInfo.path;
    uid_t uid = dirInfo.uid;
    for (const int32_t userId : userIds) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            LOGI("GetDirListSpace stopped by stopScanFlag, userIds");
            std::vector<DirSpaceInfo>().swap(resultDirs);
            return;
        }
        std::string userPath = StringPrintf(path.c_str(), userId);
        int64_t blks = 0;
        AddBlksRecurse(userPath, blks, uid);
        int64_t dirSize = blks * BLOCK_BYTE;
        resultDirs.push_back({userPath, uid, dirSize});
    }
}

int32_t QuotaManager::GetDirListSpace(std::vector<DirSpaceInfo> &dirs)
{
    LOGI("GetDirListSpace start, input dirs size: %{public}zu", dirs.size());
    std::vector<int32_t> userIds;
    GetAllUserIds(userIds);
    if (userIds.empty()) {
        userIds.push_back(StorageService::DEFAULT_USER_ID);
    }
    std::vector<DirSpaceInfo> resultDirs;
    for (const auto &dirInfo : dirs) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            LOGI("GetDirListSpace stopped by stopScanFlag, dirs");
            std::vector<DirSpaceInfo>().swap(resultDirs);
            return E_ERR;
        }
        if (dirInfo.path.find("%d") == std::string::npos) {
            ProcessSingleDir(dirInfo, resultDirs);
        } else {
            ProcessDirWithUserId(dirInfo, userIds, resultDirs);
        }
    }
    std::sort(resultDirs.begin(), resultDirs.end(), [](const DirSpaceInfo& a, const DirSpaceInfo& b) {
        return a.size > b.size;
    });
    dirs = resultDirs;
    LOGI("GetDirListSpace end, result dirs size: %{public}zu", dirs.size());
    return E_OK;
}

void QuotaManager::SetStopScanFlag(bool stop)
{
    stopScanFlag_.store(stop);
    LOGI("QuotaManager::SetStopScanFlag called with stop=%{public}d", stop);
}

void QuotaManager::GetAncoSizeData(std::string &extraData)
{
    if (stopScanFlag_.load(std::memory_order_relaxed)) {
        LOGI("GetAncoSize stopped by stopScanFlg");
        return;
    }
    LOGI("begin get Anco info.");
    std::ostringstream oss;
    uint64_t imageSize = 0;
    GetRmgResourceSize("rgm_hmos", imageSize);
    oss << "{anco image size:" << ConvertBytesToMB(imageSize, ACCURACY_NUM) << "MB}" << std::endl;
    std::vector<std::string> ignorePaths;
    //data/virt_service/rgm_hmos/anco_hmos_data/media/0
    ignorePaths.push_back("anco_hmos_data/media/0/Pictures/oh_pictures");
    //未找到
    ignorePaths.push_back("anco_hmos_data/media/0/oh_Docs");
    //data/virt_service/rgm_hmos/anco_hmos_data/meda/0
    ignorePaths.push_back("anco_hmos_data/media/0/我的手机(鸿蒙)");
    //被quota 7758 统计目录
    ignorePaths.push_back("anco_hmos_data/cota/anco");
    uint64_t dirSize = 0;
    GetRmgDataSize("rgm_hmos", "anco_hmos_data", ignorePaths, dirSize);
    oss << "{anco dir size:" << ConvertBytesToMB(dirSize, ACCURACY_NUM) << "MB" << std::endl;
    oss << "{anco total size:" << ConvertBytesToMB((imageSize + dirSize), ACCURACY_NUM) << "MB}" << std::endl;
    extraData = oss.str();
    LOGI("end get Anco info.");
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
