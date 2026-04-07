/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <regex>

#include "cJSON.h"
#include "config_policy_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {
using OHOS::StorageManager::UserdataDirInfo;
constexpr const char *QUOTA_DEVICE_DATA_PATH = "/data";
constexpr const char *PROC_MOUNTS_PATH = "/proc/mounts";
constexpr const char *DEV_BLOCK_PATH = "/dev/block/";
constexpr const char *CONFIG_FILE_PATH = "/etc/passwd";
constexpr const char *DATA_DEV_PATH = "/dev/block/by-name/userdata";
constexpr const char* SYSTEM_DATA_CONFIG_PATH = "/etc/storage_statistic_systemdata.json";
constexpr const char* SYSTEM_DATA_KEY = "storage.statistic.systemdata";
constexpr const char* HMFS_PATH = "/sys/fs/hmfs/userdata";
constexpr const char* MAIN_BLKADDR = "/main_blkaddr";
constexpr const char* OVP_CHUNKS = "/ovp_chunks";
constexpr const char* SCAN_EXCLUDE_PATH = "/data/local/tmp";
constexpr uint64_t ONE_KB = 1;
constexpr uint64_t ONE_MB = 1024 * ONE_KB;
constexpr double DIVISOR = 1000.0 * 1000.0;
constexpr double BASE_NUMBER = 10.0;
constexpr int32_t ONE_MS = 1000;
constexpr int32_t ACCURACY_NUM = 2;
constexpr int32_t MAX_UID_COUNT = 100000;
constexpr int32_t BLOCK_BYTE = 512;
constexpr int32_t TOP_SPACE_COUNT = 20;
constexpr int32_t BYTES_PRE_KB = 1024;
constexpr int32_t LINE_MAX_LEN = 32;
constexpr int32_t MAX_WHITE_PATH_COUNT = 10;
constexpr int32_t MAX_WHITE_UID_COUNT = 3;
constexpr int32_t FOUR_K = 4096;
constexpr double SA_SIZE_THRESHOLD_MB = 100.0;
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
    LOGD("[L2:QuotaManager] InitialiseQuotaMounts: >>> ENTER <<<");
    std::lock_guard<std::recursive_mutex> lock(mMountsLock);
    mQuotaReverseMounts.clear();
    std::ifstream in(PROC_MOUNTS_PATH);

    if (!in.is_open()) {
        LOGE("[L2:QuotaManager] InitialiseQuotaMounts: <<< EXIT FAILED <<< open mounts file failed");
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

    LOGD("[L2:QuotaManager] InitialiseQuotaMounts: <<< EXIT SUCCESS <<<");
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
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForUid: >>> ENTER <<< uid=%{public}d", uid);
    struct dqblk dq;
#ifdef ENABLE_EMULATOR
    if (!InitialiseQuotaMounts()) {
        LOGE("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT FAILED <<< initialise quota mounts failed");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }
    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGI("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT SUCCESS <<< no quotas present, skipped");
        return E_OK;
    }
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) == 0) {
        size = static_cast<int64_t>(dq.dqb_curspace);
        LOGI("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT SUCCESS <<< uid=%{public}d, size=%{public}lld",
            uid, static_cast<long long>(size));
        return E_OK;
    }
    LOGE("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT FAILED <<< uid=%{public}d, errno=%{public}d",
        uid, errno);
#else
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), DATA_DEV_PATH, uid, reinterpret_cast<char*>(&dq)) == 0) {
        size = static_cast<int64_t>(dq.dqb_curspace);
        LOGI("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT SUCCESS <<< uid=%{public}d, size=%{public}lld",
            uid, static_cast<long long>(size));
        return E_OK;
    }
    LOGE("[L2:QuotaManager] GetOccupiedSpaceForUid: <<< EXIT FAILED <<< uid=%{public}d, errno=%{public}d",
        uid, errno);
#endif
    return E_QUOTA_CTL_KERNEL_ERR;
}

void QuotaManager::GetUidStorageStats(std::vector<UidSaInfo> &vec, int64_t &totalSize,
    const std::map<int32_t, std::string> &bundleNameAndUid, int32_t type)
{
    LOGI("[L2:QuotaManager] GetUidStorageStats: >>> ENTER <<<");
    AllAppVec allVec;
    auto ret = ParseConfigFile(CONFIG_FILE_PATH, allVec.sysSaVec);
    if (ret != E_OK) {
        LOGE("[L2:QuotaManager] GetUidStorageStats: <<< EXIT FAILED <<< parse passwd file failed");
        return;
    }
    uint64_t iNodes = 0;
    GetOccupiedSpaceForUidList(allVec, iNodes);
    switch (type) {
        case SYS_SA :
            totalSize = GetSaOrOtherTotal(allVec.sysSaVec);
            ProcessVecList(allVec.sysSaVec, true, bundleNameAndUid);
            vec.swap(allVec.sysSaVec);
            break;
        case SYS_APP :
            ProcessVecList(allVec.sysAppVec, false, bundleNameAndUid);
            vec.swap(allVec.sysAppVec);
            break;
        case USER_APP :
            ProcessVecList(allVec.userAppVec, false, bundleNameAndUid);
            vec.swap(allVec.userAppVec);
            break;
        case OTHER_APP :
            totalSize = GetSaOrOtherTotal(allVec.otherAppVec);
            ProcessVecList(allVec.otherAppVec, false, bundleNameAndUid);
            vec.swap(allVec.otherAppVec);
            break;
        default:
            break;
    }
    LOGI("[L2:QuotaManager] GetUidStorageStats: <<< EXIT SUCCESS <<<");
}

int64_t QuotaManager::GetSaOrOtherTotal(const std::vector<UidSaInfo> &vec)
{
    int64_t totalSize = 0;
    for (const auto &info : vec) {
        totalSize += info.size;
    }
    return totalSize;
}

int32_t QuotaManager::GetFileData(const std::string &path, int64_t &size)
{
    LOGD("[L2:QuotaManager] GetFileData: >>> ENTER <<< path=%{public}s", path.c_str());
    if (path.empty() || path.size() >= PATH_MAX) {
        LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< path is invalid or too long");
        return E_FILE_PATH_INVALID;
    }

    char realPath[PATH_MAX] = {0x00};
    if (!realpath(path.c_str(), realPath)) {
        LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< realpath failed, errno=%{public}d", errno);
        return E_FILE_PATH_INVALID;
    }

    // 确保规范化后的路径在预期范围内
    std::string normalizedPath(realPath);
    if (normalizedPath != path) {
        LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< normalized path mismatch");
        return E_FILE_PATH_INVALID;
    }

    std::ifstream infile(normalizedPath, std::ios::in);
    if (!infile.is_open()) {
        LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< open file failed, errno=%{public}d", errno);
        return E_OPEN_JSON_FILE_ERROR;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) {
            continue;
        }
        // 添加长度限制
        if (line.size() > LINE_MAX_LEN) {
            LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< line too long, len=%{public}zu",
                line.size());
            return E_NON_ACCESS;
        }

        int64_t listNum = 0;
        if (StringToInt64(line, listNum)) {
            // 检查加法溢出
            if (size > INT64_MAX - listNum) {
                LOGE("[L2:QuotaManager] GetFileData: <<< EXIT FAILED <<< size overflow");
                return E_NON_ACCESS;
            }
            size += listNum;
        }
    }
    LOGD("[L2:QuotaManager] GetFileData: <<< EXIT SUCCESS <<< size=%{public}lld",
        static_cast<long long>(size));
    return E_OK;
}

bool QuotaManager::StringToInt64(const std::string& str, int64_t& out_value)
{
    if (str.empty() || str.size() > 20) { // 20是INT64_MAX的字符串长度
        LOGE("[L2:QuotaManager] StringToInt64: <<< EXIT FAILED <<< invalid input length, len=%{public}zu",
            str.size());
        return false;
    }
    auto result = std::from_chars(str.data(), str.data() + str.size(), out_value);
    if (result.ec == std::errc::invalid_argument) {
        LOGE("[L2:QuotaManager] StringToInt64: <<< EXIT FAILED <<< invalid argument");
        return false;
    }

    if (result.ec == std::errc::result_out_of_range) {
        LOGE("[L2:QuotaManager] StringToInt64: <<< EXIT FAILED <<< integer overflow");
        return false;
    }

    if (result.ptr != str.data() + str.size()) {
        LOGE("[L2:QuotaManager] StringToInt64: <<< EXIT FAILED <<< invalid characters in string");
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

int32_t QuotaManager::ParseConfigFile(const std::string &path, std::vector<UidSaInfo> &vec)
{
    LOGI("[L2:QuotaManager] ParseConfigFile: >>> ENTER <<< path=%{private}s", path.c_str());
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("[L2:QuotaManager] ParseConfigFile: <<< EXIT FAILED <<< path invalid, errno=%{public}d", errno);
        return E_JSON_PARSE_ERROR;
    }

    std::ifstream infile(std::string(realPath), std::ios::in);
    if (!infile.is_open()) {
        LOGE("[L2:QuotaManager] ParseConfigFile: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return E_OPEN_JSON_FILE_ERROR;
    }

    std::string line;
    while (getline(infile, line)) {
        if (line == "") {
            continue;
        }
        UidSaInfo info;
        if (GetUid32FromEntry(line, info.uid, info.saName)) {
            vec.push_back(info);
        }
    }
    infile.close();
    LOGI("[L2:QuotaManager] ParseConfigFile: <<< EXIT SUCCESS <<< entries=%{public}zu", vec.size());
    return E_OK;
}

void QuotaManager::ProcessVecList(std::vector<UidSaInfo> &vec, bool isSa,
    const std::map<int32_t, std::string> &bundleNameAndUid)
{
    if (isSa) {
        SortAndCutSaInfoVec(vec, isSa);
        return;
    }
    SortAndCutSaInfoVec(vec, isSa);
    AssembleSaInfoVec(vec, bundleNameAndUid);
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

void QuotaManager::SortAndCutSaInfoVec(std::vector<UidSaInfo> &vec, bool isSa)
{
    std::sort(vec.begin(), vec.end(), [](const UidSaInfo& a, const UidSaInfo& b) {
        return a.size > b.size;
    });
    if (!isSa) {
        vec.erase(vec.begin() + std::min(static_cast<size_t>(TOP_SPACE_COUNT), vec.size()), vec.end());
        return;
    }
    std::vector<int32_t> uidList;
    if (ParseSystemDataConfigFile(uidList) != E_OK) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: failed");
    }
    for (auto it = vec.rbegin(); it != vec.rend();) {
        if (ConvertBytesToMB(it->size, ACCURACY_NUM) >= SA_SIZE_THRESHOLD_MB) {
            break;
        }
        auto ret = std::find(uidList.begin(), uidList.end(), it->uid);
        it++;
        auto element = it.base();
        if (ret == uidList.end()) {
            vec.erase(element);
        }
    }
}

void QuotaManager::GetOccupiedSpaceForUidList(AllAppVec &allVec, uint64_t &iNodes)
{
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForUidList: >>> ENTER <<<");
    int32_t curUid = 0;
    int32_t count = 0;
    std::map<int32_t, int64_t> userAppSizeMap;
    while (count < MAX_UID_COUNT) {
        KernelNextDqBlk dq;
        if (quotactl(QCMD(Q_GETNEXTQUOTA_LOCAL, USRQUOTA), DATA_DEV_PATH, curUid, reinterpret_cast<char*>(&dq)) != 0) {
            LOGE("[L2:QuotaManager] GetOccupiedSpaceForUidList: <<< EXIT FAILED <<< uid=%{public}d, errno=%{public}d",
                curUid, errno);
            break;
        }
        int32_t dqUid = static_cast<int32_t>(dq.dqbId);
        bool isSaUid = false;
        iNodes += dq.dqbCurInodes;
        for (UidSaInfo &info : allVec.sysSaVec) {
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
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForUidList: <<< EXIT SUCCESS <<< count=%{public}d, iNodes=%{public}llu",
        count, static_cast<unsigned long long>(iNodes));
}

void QuotaManager::AssembleSysAppVec(int32_t dqUid, const KernelNextDqBlk &dq,
    std::map<int32_t, int64_t> &userAppSizeMap, std::vector<UidSaInfo> &sysAppVec)
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
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForGid: >>> ENTER <<< gid=%{public}d", gid);

    if (InitialiseQuotaMounts() != true) {
        LOGE("[L2:QuotaManager] GetOccupiedSpaceForGid: <<< EXIT FAILED <<< initialise quota mounts failed");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGI("[L2:QuotaManager] GetOccupiedSpaceForGid: <<< EXIT SUCCESS <<< no quotas present, skipped");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, GRPQUOTA), device.c_str(), gid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("[L2:QuotaManager] GetOccupiedSpaceForGid: <<< EXIT FAILED <<< gid=%{public}d, errno=%{public}d",
            gid, errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForGid: <<< EXIT SUCCESS <<< gid=%{public}d, size=%{public}lld",
        gid, static_cast<long long>(size));
    return E_OK;
}


static int64_t GetOccupiedSpaceForPrjId(int32_t prjId, int64_t &size)
{
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForPrjId: >>> ENTER <<< prjId=%{public}d", prjId);

    if (InitialiseQuotaMounts() != true) {
        LOGE("[L2:QuotaManager] GetOccupiedSpaceForPrjId: <<< EXIT FAILED <<< initialise quota mounts failed");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    if (device.empty()) {
        LOGI("[L2:QuotaManager] GetOccupiedSpaceForPrjId: <<< EXIT SUCCESS <<< no quotas present, skipped");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, PRJQUOTA), device.c_str(), prjId, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("[L2:QuotaManager] GetOccupiedSpaceForPrjId: <<< EXIT FAILED <<< prjId=%{public}d, errno=%{public}d",
            prjId, errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    size = static_cast<int64_t>(dq.dqb_curspace);
    LOGI("[L2:QuotaManager] GetOccupiedSpaceForPrjId: <<< EXIT SUCCESS <<< prjId=%{public}d, size=%{public}lld",
        prjId, static_cast<long long>(size));
    return E_OK;
}

int32_t QuotaManager::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    LOGI("[L2:QuotaManager] GetOccupiedSpace: >>> ENTER <<< idType=%{public}d, id=%{public}d", idType, id);

    int32_t ret;
    switch (idType) {
        case USRID:
            ret = GetOccupiedSpaceForUid(id, size);
            break;
        case GRPID:
            ret = GetOccupiedSpaceForGid(id, size);
            break;
        case PRJID:
            ret = GetOccupiedSpaceForPrjId(id, size);
            break;
        default:
            LOGE("[L2:QuotaManager] GetOccupiedSpace: <<< EXIT FAILED <<< invalid idType=%{public}d", idType);
            return E_NON_EXIST;
    }

    if (ret == E_OK) {
        LOGI("[L2:QuotaManager] GetOccupiedSpace: <<< EXIT SUCCESS <<< idType=%{public}d, id=%{public}d,"
            "size=%{public}lld", idType, id, static_cast<long long>(size));
    } else {
        LOGE("[L2:QuotaManager] GetOccupiedSpace: <<< EXIT FAILED <<< idType=%{public}d, id=%{public}d, ret=%{public}d",
            idType, id, ret);
    }
    return ret;
}

int32_t QuotaManager::SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    LOGI("[L2:QuotaManager] SetBundleQuota: >>> ENTER <<< uid=%{public}d, path=%{public}s, limit=%{public}dMB",
        uid, bundleDataDirPath.c_str(), limitSizeMb);

    if (bundleDataDirPath.empty() || uid < 0 || limitSizeMb < 0) {
        LOGE("[L2:QuotaManager] SetBundleQuota: <<< EXIT FAILED <<< invalid params");
        return E_PARAMS_INVALID;
    }

    if (InitialiseQuotaMounts() != true) {
        LOGE("[L2:QuotaManager] SetBundleQuota: <<< EXIT FAILED <<< initialise quota mounts failed");
        return E_INIT_QUOTA_MOUNTS_FAILED;
    }

    std::string device = "";
    if (bundleDataDirPath.find(QUOTA_DEVICE_DATA_PATH) == 0) {
        device = GetQuotaSrcMountPath(QUOTA_DEVICE_DATA_PATH);
    }
    if (device.empty()) {
        LOGI("[L2:QuotaManager] SetBundleQuota: <<< EXIT SUCCESS <<< no quotas present, skipped");
        return E_OK;
    }

    struct dqblk dq;
    if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("[L2:QuotaManager] SetBundleQuota: <<< EXIT FAILED <<< get quota failed, uid=%{public}d, errno=%{public}d",
             uid, errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    }

    // dqb_bhardlimit is count of 1kB blocks, dqb_curspace is bytes
    struct statvfs stat;
    if (statvfs(bundleDataDirPath.c_str(), &stat) != 0) {
        LOGE("[L2:QuotaManager] SetBundleQuota: <<< EXIT FAILED <<< statvfs failed, errno=%{public}d", errno);
        return E_STAT_VFS_KERNEL_ERR;
    }

    dq.dqb_valid = QIF_LIMITS;
    dq.dqb_bhardlimit = (uint32_t)limitSizeMb * ONE_MB;
    if (quotactl(QCMD(Q_SETQUOTA, USRQUOTA), device.c_str(), uid, reinterpret_cast<char*>(&dq)) != 0) {
        LOGE("[L2:QuotaManager] SetBundleQuota: <<< EXIT FAILED <<< set quota failed, uid=%{public}d, errno=%{public}d",
             uid, errno);
        return E_QUOTA_CTL_KERNEL_ERR;
    } else {
        LOGI("[L2:QuotaManager] SetBundleQuota: <<< EXIT SUCCESS <<< uid=%{public}d, limit=%{public}dMB",
             uid, limitSizeMb);
        return E_OK;
    }
}

int32_t QuotaManager::SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit)
{
    LOGI("[L2:QuotaManager] SetQuotaPrjId: >>> ENTER <<< path=%{public}s, prjId=%{public}d, inherit=%{public}d",
        path.c_str(), prjId, inherit);

    struct fsxattr fsx;
    char *realPath = realpath(path.c_str(), nullptr);
    if (realPath == nullptr) {
        LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< realpath failed, errno=%{public}d", errno);
        return E_PARAMS_NULLPTR_ERR;
    }
    FILE *f = fopen(realPath, "r");
    free(realPath);
    if (f == nullptr) {
        LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< open failed, path=%{public}s, errno=%{public}d",
            path.c_str(), errno);
        return E_SYS_KERNEL_ERR;
    }
    int fd = fileno(f);
    if (fd < 0) {
        (void)fclose(f);
        LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< fileno failed");
        return E_SYS_KERNEL_ERR;
    }
    if (ioctl(fd, FS_IOC_FSGETXATTR, &fsx) == -1) {
        LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< get xattr failed, errno=%{public}d", errno);
        (void)fclose(f);
        return E_SYS_KERNEL_ERR;
    }
    if (fsx.fsx_projid == static_cast<uint32_t>(prjId)) {
        (void)fclose(f);
        LOGI("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT SUCCESS <<< already set, prjId=%{public}d", prjId);
        return E_OK;
    }
    fsx.fsx_projid = static_cast<uint32_t>(prjId);
    if (ioctl(fd, FS_IOC_FSSETXATTR, &fsx) == -1) {
        LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< set xattr failed, errno=%{public}d", errno);
        (void)fclose(f);
        return E_SYS_KERNEL_ERR;
    }

    if (inherit) {
        uint32_t flags;
        if (ioctl(fd, FS_IOC_GETFLAGS, &flags) == -1) {
            LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< get flags failed, errno=%{public}d", errno);
            (void)fclose(f);
            return E_SYS_KERNEL_ERR;
        }
        flags |= FS_PROJINHERIT_FL;
        if (ioctl(fd, FS_IOC_SETFLAGS, &flags) == -1) {
            LOGE("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT FAILED <<< set flags failed, errno=%{public}d", errno);
            (void)fclose(f);
            return E_SYS_KERNEL_ERR;
        }
    }
    (void)fclose(f);
    LOGI("[L2:QuotaManager] SetQuotaPrjId: <<< EXIT SUCCESS <<< path=%{public}s, prjId=%{public}d",
        path.c_str(), prjId);
    return E_OK;
}

int32_t QuotaManager::AddBlksRecurse(const std::string &path, int64_t &blks, uid_t uid)
{
    AddBlks(path, blks, uid);
    if (!IsDir(path)) {
        return E_OK;
    }
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("[L2:QuotaManager] AddBlksRecurse: <<< EXIT FAILED <<< open dir failed, path=%{public}s, errno=%{public}d",
            path.c_str(), errno);
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
        LOGE("[L2:QuotaManager] AddBlks: <<< EXIT FAILED <<< lstat failed, path=%{public}s, errno=%{public}d",
            path.c_str(), errno);
        return E_STATISTIC_STAT_FAILED;
    }
    if (uid == st.st_uid) {
        blks += static_cast<int64_t>(st.st_blocks);
    }
    return E_OK;
}

int32_t QuotaManager::GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks)
{
    LOGI("[L2:QuotaManager] GetDqBlkSpacesByUids: >>> ENTER <<< uids size=%{public}zu", uids.size());

    dqBlks.clear();
    for (auto &uid : uids) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            LOGI("[L2:QuotaManager] GetDqBlkSpacesByUids: stopped by stopScanFlag");
            std::vector<NextDqBlk>().swap(dqBlks);
            return E_ERR;
        }
        struct dqblk dq;
#ifdef ENABLE_EMULATOR
        return E_NOT_SUPPORT;
#else
        if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), DATA_DEV_PATH, uid, reinterpret_cast<char *>(&dq)) != 0) {
            LOGE("[L2:QuotaManager] GetDqBlkSpacesByUids: <<< EXIT FAILED <<< uid=%{public}d, errno=%{public}d",
                uid, errno);
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
    LOGI("[L2:QuotaManager] GetDqBlkSpacesByUids: end, dqBlks size: %{public}zu", dqBlks.size());
    return E_OK;
}

int32_t QuotaManager::GetSystemDataSize(int64_t &otherUidSizeSum)
{
    LOGI("[L2:QuotaManager] GetSystemDataSize: >>> ENTER <<<");
    otherUidSizeSum = 0;
    std::vector<int32_t> uidList;
    int32_t ret = ParseSystemDataConfigFile(uidList);
    if (ret != E_OK) {
        LOGE("[L2:QuotaManager] GetSystemDataSize: failed, ret=%{public}d", ret);
        return E_GET_SYSTEM_DATA_SIZE_ERROR;
    }
    int64_t systemCacheSize = 0;
    ret = GetSystemCacheSize(uidList, systemCacheSize);
    if (ret != E_OK) {
        LOGE("[L2:QuotaManager] GetSystemDataSize: failed, ret=%{public}d", ret);
        systemCacheSize = 0;
    }
    LOGI("[L2:QuotaManager] GetSystemDataSize: system_cache=%{public}lld", static_cast<long long>(systemCacheSize));
    int64_t metaDataSize = 0;
    ret = GetMetaDataSize(metaDataSize);
    if (ret != E_OK) {
        LOGW("[L2:QuotaManager] GetSystemDataSize: failed, ret=%{public}d", ret);
        metaDataSize = 0;
    }
    LOGI("[L2:QuotaManager] GetSystemDataSize: filesystem_metadata=%{public}lld", static_cast<long long>(metaDataSize));
    otherUidSizeSum = systemCacheSize + metaDataSize;
    LOGI("[L2:QuotaManager] GetSystemDataSize: end, total=%{public}lld (cache=%{public}lld + meta=%{public}lld)",
        static_cast<long long>(otherUidSizeSum), static_cast<long long>(systemCacheSize),
        static_cast<long long>(metaDataSize));
    return E_OK;
}

int32_t QuotaManager::ParseSystemDataConfigFile(std::vector<int32_t> &uidList)
{
    std::string path = SYSTEM_DATA_CONFIG_PATH;
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(path.c_str(), buf, MAX_PATH_LEN);
    if (configPath == NULL) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: config path is NULL");
        return E_PARAMS_INVALID;
    }
    char canonicalBuf[PATH_MAX] = { 0 };
    char *canonicalPath = realpath(configPath, canonicalBuf);
    if (canonicalPath == NULL || canonicalPath[0] == '\0' || strlen(canonicalPath) >= MAX_PATH_LEN) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: get ccm config file path failed");
        canonicalPath = NULL;
        return E_PARAMS_INVALID;
    }
    canonicalBuf[PATH_MAX - 1] = '\0';
    std::ifstream configFile(canonicalBuf);
    if (!configFile.is_open()) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: ParseSystemDataConfigFile cannot open config file:"
            "%{public}s, errno: %{public}d", canonicalPath, errno);
        canonicalPath = NULL;
        return E_PARAMS_INVALID;
    }
    std::string jsonString((std::istreambuf_iterator<char>(configFile)), std::istreambuf_iterator<char>());
    configFile.close();
    canonicalPath = NULL;
    cJSON* root = cJSON_Parse(jsonString.c_str());
    if (root == NULL) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: ParseSystemDataConfigFile cJSON_Parse failed");
        return E_PARAMS_INVALID;
    }
    cJSON* uidArray = cJSON_GetObjectItem(root, SYSTEM_DATA_KEY);
    if (uidArray == NULL || !cJSON_IsArray(uidArray)) {
        LOGE("[L2:QuotaManager] ParseSystemDataConfigFile: ParseSystemDataConfigFile uidArray is null or not an array");
        cJSON_Delete(root);
        return E_PARAMS_INVALID;
    }
    int arraySize = cJSON_GetArraySize(uidArray);
    for (int i = 0; i < arraySize; i++) {
        cJSON* item = cJSON_GetArrayItem(uidArray, i);
        if (item != NULL && cJSON_IsNumber(item)) {
            int32_t uid = item->valueint;
            uidList.push_back(uid);
        }
    }
    cJSON_Delete(root);
    LOGI("[L2:QuotaManager] ParseSystemDataConfigFile: ParseSystemDataConfigFile loaded %{public}zu"
        "UIDs from config file", uidList.size());
    return E_OK;
}

int32_t QuotaManager::GetSystemCacheSize(const std::vector<int32_t> &uidList, int64_t &cacheSize)
{
    LOGI("[L2:QuotaManager] GetSystemCacheSize: start, uidList size=%{public}zu", uidList.size());
#ifdef ENABLE_EMULATOR
    LOGW("GetSystemCacheSize not support on emulator");
    return E_NOT_SUPPORT;
#else
    for (auto uid : uidList) {
        if (uid == StorageService::ROOT_UID || uid == StorageService::SYSTEM_UID ||
            uid == StorageService::MEMMGR_UID) {
            continue;
        }
        struct dqblk dq;
        if (quotactl(QCMD(Q_GETQUOTA, USRQUOTA), DATA_DEV_PATH, uid, reinterpret_cast<char *>(&dq)) != 0) {
            LOGW("[L2:QuotaManager] GetSystemCacheSize: quotactl failed for uid=%{public}d,"
                "errno=%{public}d", uid, errno);
            StorageService::StorageRadar::ReportSpaceRadar("GetSystemCacheSize", E_GET_SYSTEM_DATA_SIZE_ERROR,
                "uid:" + std::to_string(uid) + ",errno:" + std::to_string(errno));
            continue;
        }
        int64_t uidSize = static_cast<int64_t>(dq.dqb_curspace);
        cacheSize += uidSize;
        LOGD("[L2:QuotaManager] GetSystemCacheSize: uid=%{public}d, size=%{public}lld",
            uid, static_cast<long long>(uidSize));
    }

    LOGI("[L2:QuotaManager] GetSystemCacheSize: end, cacheSize=%{public}lld", static_cast<long long>(cacheSize));
    return E_OK;
#endif
}

int32_t QuotaManager::GetMetaDataSize(int64_t &metaDataSize)
{
    LOGI("GetMetaDataSize start");
    std::string blkPath = std::string(HMFS_PATH) + std::string(MAIN_BLKADDR);
    int64_t blkSize = -1;
    int32_t ret = GetFileData(blkPath, blkSize);
    if (ret != E_OK || blkSize < 0) {
        LOGE("GetMetaDataSize failed to read main_blkaddr, ret=%{public}d, blkSize=%{public}lld",
             ret, static_cast<long long>(blkSize));
        return E_GET_SYSTEM_DATA_SIZE_ERROR;
    }
    std::string chunkPath = std::string(HMFS_PATH) + std::string(OVP_CHUNKS);
    int64_t chunkSize = -1;
    ret = GetFileData(chunkPath, chunkSize);
    if (ret != E_OK || chunkSize < 0) {
        LOGE("GetMetaDataSize failed to read ovp_chunks, ret=%{public}d, chunkSize=%{public}lld",
             ret, static_cast<long long>(chunkSize));
        return E_GET_SYSTEM_DATA_SIZE_ERROR;
    }
    if (blkSize > INT64_MAX / FOUR_K || chunkSize > INT64_MAX / (FOUR_K * BLOCK_BYTE)) {
        LOGE("GetMetaDataSize overflow detected: blkSize=%{public}lld, chunkSize=%{public}lld",
             static_cast<long long>(blkSize), static_cast<long long>(chunkSize));
        return E_CALCULATE_OVERFLOW_UP;
    }
    metaDataSize = static_cast<int64_t>(blkSize * FOUR_K + chunkSize * FOUR_K * BLOCK_BYTE);
    LOGI("GetMetaDataSize end: blkSize=%{public}lld, chunkSize=%{public}lld, total=%{public}lld",
        static_cast<long long>(blkSize), static_cast<long long>(chunkSize),
        static_cast<long long>(metaDataSize));
    return E_OK;
}

int32_t QuotaManager::AddBlksRecurseMultiUids(const std::string &path, std::vector<int64_t> &blks,
    const std::vector<int32_t> &uids)
{
    if (stopScanFlag_.load(std::memory_order_relaxed)) {
        std::string extraData = "path=" + path;
        StorageService::StorageRadar::ReportSpaceRadar("AddBlksRecurseMultiUids", E_ERR, extraData);
        LOGE("AddBlksRecurseMultiUids stopped by stopScanFlag, current path=%{public}s", path.c_str());
        return E_ERR;
    }
    AddBlksMultiUids(path, blks, uids);
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
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            std::string extraData = "path=" + path;
            StorageService::StorageRadar::ReportSpaceRadar("AddBlksRecurseMultiUids", E_ERR, extraData);
            LOGE("AddBlksRecurseMultiUids stopped by stopScanFlag in loop, "
                 "current dir=%{public}s, next entry=%{public}s", path.c_str(), ent->d_name);
            closedir(dir);
            return E_ERR;
        }
        if ((strcmp(ent->d_name, ".") == 0) || (strcmp(ent->d_name, "..") == 0)) {
            continue;
        }
        std::string subPath = path + "/" + ent->d_name;

        if (subPath == SCAN_EXCLUDE_PATH) {
            LOGI("AddBlksRecurseMultiUids skip excluded path: %{public}s", subPath.c_str());
            continue;
        }

        int32_t retTmp = AddBlksRecurseMultiUids(subPath, blks, uids);
        if (retTmp != E_OK) {
            ret = retTmp;
        }
    }
    (void)closedir(dir);
    return ret;
}

int32_t QuotaManager::AddBlksMultiUids(const std::string &path, std::vector<int64_t> &blks,
    const std::vector<int32_t> &uids)
{
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        int32_t errnoTmp = errno;
        std::string extraData = "path=" + path + ",kernelCode=" + std::to_string(errnoTmp);
        StorageService::StorageRadar::ReportSpaceRadar("AddBlksMultiUids", E_STATISTIC_STAT_FAILED, extraData);
        LOGE("lstat failed, path is %{public}s, errno is %{public}d", path.c_str(), errno);
        return E_STATISTIC_STAT_FAILED;
    }
    for (size_t i = 0; i < uids.size(); ++i) {
        if (static_cast<uid_t>(uids[i]) == st.st_uid) {
            blks[i] += static_cast<int64_t>(st.st_blocks);
            break; // Each file belongs to only one UID
        }
    }
    return E_OK;
}

int32_t QuotaManager::GetDirListSpaceByPaths(const std::vector<std::string> &paths,
    const std::vector<int32_t> &uids, std::vector<DirSpaceInfo> &resultDirs)
{
    LOGI("GetDirListSpaceByPaths start, paths size=%{public}zu", paths.size());
    if (paths.empty() || uids.empty() || paths.size() > MAX_WHITE_PATH_COUNT || uids.size() > MAX_WHITE_UID_COUNT) {
        LOGE("GetDirListSpaceByPaths params is invalid, paths size %{public}zu, uids size %{public}zu",
             paths.size(), uids.size());
        return E_PARAMS_INVALID;
    }

    resultDirs.clear();

    for (size_t pathIdx = 0; pathIdx < paths.size(); ++pathIdx) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            std::string extraData = "path=" + paths[pathIdx];
            StorageService::StorageRadar::ReportSpaceRadar("GetDirListSpaceByPaths", E_ERR, extraData);
            LOGE("GetDirListSpaceByPaths stopped by stopScanFlag");
            std::vector<DirSpaceInfo>().swap(resultDirs);
            return E_ERR;
        }

        const std::string &path = paths[pathIdx];
        auto pathStartTime = std::chrono::steady_clock::now();

        std::vector<int64_t> blks(uids.size(), 0);
        int32_t ret = AddBlksRecurseMultiUids(path, blks, uids);
        auto pathEndTime = std::chrono::steady_clock::now();
        auto pathDurationMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            pathEndTime - pathStartTime).count();
        LOGE("GetDirListSpaceByPaths scan path[%{public}zu/%{public}zu] '%{public}s' "
             "completed in %{public}lldms, ret=%{public}d",
             pathIdx + 1, paths.size(), path.c_str(),
             static_cast<long long>(pathDurationMs), ret);

        if (ret != E_OK) {
            LOGW("GetDirListSpaceByPaths AddBlksRecurseMultiUids failed for %{public}s, ret=%{public}d",
                 path.c_str(), ret);
            // Continue with other paths even if this one failed
            continue;
        }

        // Convert blocks to bytes and create DirSpaceInfo for each UID
        for (size_t uidIdx = 0; uidIdx < uids.size(); ++uidIdx) {
            int64_t dirSize = blks[uidIdx] * BLOCK_BYTE;
            resultDirs.push_back({path, static_cast<uint32_t>(uids[uidIdx]), dirSize});
            LOGD("GetDirListSpaceByIds path=%{public}s, uid=%{public}d, size=%{public}lld",
                 path.c_str(), uids[uidIdx], static_cast<long long>(dirSize));
        }
    }
    if (stopScanFlag_.load(std::memory_order_relaxed)) {
        LOGE("GetDirListSpaceByPaths stopped by stopScanFlag after loop");
        std::vector<DirSpaceInfo>().swap(resultDirs);
        return E_ERR;
    }

    LOGI("GetDirListSpaceByPaths end, result dirs size=%{public}zu", resultDirs.size());
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
            LOGI("[L2:QuotaManager] ProcessDirWithUserId: stopped by stopScanFlag");
            std::vector<DirSpaceInfo>().swap(resultDirs);
            return;
        }
        std::string userPath = path;
        std::string_view uidPlaceHolder = "%d";
        auto pos = path.find(uidPlaceHolder);
        if (pos != std::string::npos) {
            userPath.replace(pos, uidPlaceHolder.size(), std::to_string(userId));
        }
        int64_t blks = 0;
        AddBlksRecurse(userPath, blks, uid);
        int64_t dirSize = blks * BLOCK_BYTE;
        resultDirs.push_back({userPath, uid, dirSize});
    }
}

int32_t QuotaManager::GetDirListSpace(std::vector<DirSpaceInfo> &dirs)
{
    LOGI("[L2:QuotaManager] GetDirListSpace: >>> ENTER <<< input dirs size=%{public}zu", dirs.size());

    std::vector<int32_t> userIds;
    GetAllUserIds(userIds);
    if (userIds.empty()) {
        userIds.push_back(StorageService::DEFAULT_USER_ID);
    }
    std::vector<DirSpaceInfo> resultDirs;
    for (const auto &dirInfo : dirs) {
        if (stopScanFlag_.load(std::memory_order_relaxed)) {
            LOGI("[L2:QuotaManager] GetDirListSpace: stopped by stopScanFlag");
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
    LOGI("[L2:QuotaManager] GetDirListSpace: <<< EXIT SUCCESS <<< result dirs size=%{public}zu", dirs.size());
    return E_OK;
}

void QuotaManager::SetStopScanFlag(bool stop)
{
    stopScanFlag_.store(stop);
    LOGI("[L2:QuotaManager] SetStopScanFlag: stop=%{public}d", stop);
}

void QuotaManager::GetAncoSizeData(std::string &extraData)
{
    LOGI("[L2:QuotaManager] GetAncoSizeData: >>> ENTER <<<");

    if (stopScanFlag_.load(std::memory_order_relaxed)) {
        LOGI("[L2:QuotaManager] GetAncoSizeData: stopped by stopScanFlag");
        return;
    }
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
    LOGI("[L2:QuotaManager] GetAncoSizeData: <<< EXIT SUCCESS <<<");
}

static std::string HumanReadableSize(long long size)
{
    if (size < BYTES_PRE_KB) {
        return std::to_string(size) + "B";
    } else if (size < BYTES_PRE_KB * BYTES_PRE_KB) {
        return std::to_string(static_cast<double>(size) / BYTES_PRE_KB) + "K";
    } else if (size < BYTES_PRE_KB * BYTES_PRE_KB * BYTES_PRE_KB) {
        return std::to_string(static_cast<double>(size) / (BYTES_PRE_KB * BYTES_PRE_KB)) + "M";
    } else {
        return std::to_string(static_cast<double>(size) / (BYTES_PRE_KB * BYTES_PRE_KB * BYTES_PRE_KB)) + "G";
    }
}

static bool IsExcludeDir(const char* path)
{
    return strcmp(path, "/data/app") == 0 || strcmp(path, "/data/hmos4") == 0 ||
        strcmp(path, "/data/hwbackup") == 0 || strcmp(path, "/data/virt_service") == 0 ||
        std::regex_match(path, std::regex(R"(/data/service/el2/\d+/hmdfs)"));
}

UserdataDirInfo QuotaManager::ScanDirRecurse(const std::string &path, std::vector<UserdataDirInfo> &scanDirs)
{
    struct stat statbuf;
    struct dirent *entry;
    DIR *dir;
    UserdataDirInfo dirInfo = {path, 0, 0};

    if (IsExcludeDir(path.c_str())) {
        LOGD("[L2:QuotaManager] ScanDirRecurse: skip excluded path=%{public}s", path.c_str());
        return dirInfo;
    }

    if (lstat(path.c_str(), &statbuf) != 0) {
        LOGE("[L2:QuotaManager] ScanDirRecurse: lstat failed, path=%{public}s, errno=%{public}d",
            path.c_str(), errno);
        StorageService::StorageRadar::ReportSpaceRadar("ScanDirRecurse", E_STATISTIC_STAT_FAILED,
            "path:" + path + ",errno:" + std::to_string(errno));
        return dirInfo;
    }

    dirInfo.totalSize_ = statbuf.st_blocks * BLOCK_BYTE;
    dirInfo.totalCnt_ = 1;
    if (!S_ISDIR(statbuf.st_mode)) {
        return dirInfo;
    }
    dir = opendir(path.c_str());
    if (dir == nullptr) {
        LOGE("[L2:QuotaManager] ScanDirRecurse: opendir failed, path=%{public}s, errno=%{public}d",
            path.c_str(), errno);
        return dirInfo;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        std::string fullPath = path + "/" + entry->d_name;
        UserdataDirInfo subDirInfo = ScanDirRecurse(fullPath, scanDirs);
        dirInfo.totalSize_ += subDirInfo.totalSize_;
        dirInfo.totalCnt_ += subDirInfo.totalCnt_;
    }

    closedir(dir);

    if (dirInfo.totalSize_ >= BYTES_PRE_KB * BYTES_PRE_KB * BYTES_PRE_KB) {
        scanDirs.push_back(dirInfo);
        std::string sizeStr = HumanReadableSize(dirInfo.totalSize_);
        LOGE("[L2:QuotaManager] ScanDirRecurse: large dir found, size=%{public}s, cnt=%{public}d, path=%{public}s",
            sizeStr.c_str(), dirInfo.totalCnt_, path.c_str());
    }
    return dirInfo;
}

int32_t QuotaManager::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    LOGI("[L2:QuotaManager] ListUserdataDirInfo: >>> ENTER <<<");

    ScanDirRecurse("/data", scanDirs);

    LOGI("[L2:QuotaManager] ListUserdataDirInfo: <<< EXIT SUCCESS <<< scanDirs size=%{public}zu",
        scanDirs.size());
    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
