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

#include "user/mount_manager.h"
#include <dirent.h>
#include <fcntl.h>
#include <regex>
#include "crypto/key_manager.h"
#include "utils/disk_utils.h"
#include "utils/storage_radar.h"
#include "parameter.h"
#include "parameters.h"
#include "quota/quota_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "user/mount_constant.h"
#include "utils/mount_argument_utils.h"
#include "utils/string_utils.h"
#include "user/user_path_resolver.h"
#include "user/system_mount_manager.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
#define HMDFS_IOC 0xf2
#define HMDFS_IOC_FORBID_OPEN _IO(HMDFS_IOC, 12)
using namespace OHOS::StorageService;
constexpr int32_t PATH_MAX_FOR_LINK = 4096;
constexpr int32_t DEFAULT_USERID = 100;
constexpr int32_t ERROR_FILE_NOT_FOUND = 22;
const std::string CONSTRAINT = "constraint.distributed.transmission.outgoing";
MountManager &MountManager::GetInstance()
{
    static MountManager instance_;
    return instance_;
}

int32_t MountManager::FindProcess(std::list<std::string> &unMountFailList, std::vector<ProcessInfo> &proInfos,
    std::list<std::string> &excludeProcess)
{
    LOGI("[L2:MountManager] FindProcess: >>> ENTER <<< unMountFailList.size()=%{public}zu", unMountFailList.size());
    auto procDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir("/proc"), closedir);
    if (!procDir) {
        LOGE("[L2:MountManager] FindProcess: <<< EXIT FAILED <<< failed to open dir proc, err=%{public}d", errno);
        return E_UMOUNT_PROC_OPEN;
    }
    struct dirent *entry;
    while ((entry = readdir(procDir.get())) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        std::string name = entry->d_name;
        if (!StringIsNumber(name)) {
            continue;
        }
        ProcessInfo info;
        std::string filename = "/proc/" + name + "/stat";
        if (!GetProcessInfo(filename, info)) {
            LOGE("[L2:MountManager] FindProcess: failed to get process info, pid is %{public}s.", name.c_str());
            continue;
        }
        if (IsStringExist(excludeProcess, info.name)) {
            continue;
        }
        std::string pidPath = "/proc/" + name;
        LOGD("[L2:MountManager] FindProcess: check pid using start, pid is %{public}d, processName is %{public}s.",
            info.pid, info.name.c_str());
        if (PidUsingFlag(pidPath, unMountFailList)) {
            proInfos.push_back(info);
        }
    }
    std::string info = ProcessToString(proInfos);
    int count = static_cast<int>(proInfos.size());
    LOGI("[L2:MountManager] FindProcess: <<< EXIT SUCCESS <<< total find=%{public}d, process is: %{public}s",
        count, info.c_str());
    return E_OK;
}

bool MountManager::PidUsingFlag(std::string &pidPath, std::list<std::string> &mountFailList)
{
    std::string fdPath = pidPath + "/fd";
    auto fdDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(fdPath.c_str()), closedir);
    if (!fdDir) {
        LOGE("unable to open %{public}s, err %{public}d", fdPath.c_str(), errno);
    } else {
        struct dirent* fdDirent;
        while ((fdDirent = readdir(fdDir.get())) != nullptr) {
            if (fdDirent->d_type != DT_LNK) {
                continue;
            }
            if (CheckSymlink(fdPath + "/" +fdDirent->d_name, mountFailList)) {
                return true;
            }
        }
    }
    if (CheckMaps(pidPath + "/maps", mountFailList)) {
        return true;
    }
    if (CheckSymlink(pidPath + "/cwd", mountFailList)) {
        return true;
    }
    if (CheckSymlink(pidPath + "/root", mountFailList)) {
        return true;
    }
    if (CheckSymlink(pidPath + "/exe", mountFailList)) {
        return true;
    }
    return false;
}

bool MountManager::GetProcessInfo(const std::string &filename, ProcessInfo &info)
{
    LOGI("[L2:MountManager] GetProcessInfo: >>> ENTER <<< filename=%{public}s", filename.c_str());
    if (filename.empty()) {
        LOGE("[L2:MountManager] GetProcessInfo: <<< EXIT FAILED <<< filename is empty");
        return false;
    }
    std::ifstream inputStream(filename.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("[L2:MountManager] GetProcessInfo: <<< EXIT FAILED <<< unable to open, err=%{public}d", errno);
        return false;
    }
    std::string line;
    std::getline(inputStream, line);
    if (line.empty()) {
        LOGE("[L2:MountManager] GetProcessInfo: <<< EXIT FAILED <<< line is empty");
        inputStream.close();
        return false;
    }
    std::stringstream ss(line);
    std::string pidStr;
    ss >> pidStr;
    if (!ConvertStringToInt32(pidStr, info.pid)) {
        LOGE("[L2:MountManager] GetProcessInfo: <<< EXIT FAILED <<< ConvertStringToInt32 failed");
            inputStream.close();
        return false;
    }
    std::string processName;
    ss >> processName;
    info.name = processName;
    inputStream.close();
    LOGI("[L2:MountManager] GetProcessInfo: <<< EXIT SUCCESS <<< pid=%{public}d, name=%{public}s",
        info.pid, info.name.c_str());
    return true;
}

bool MountManager::CheckMaps(const std::string &path, std::list<std::string> &mountFailList)
{
    if (path.empty()) {
        return false;
    }
    std::ifstream inputStream(path.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open %{public}s, err %{public}d", path.c_str(), errno);
        return false;
    }
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        std::string::size_type pos = tmpLine.find('/');
        if (pos == std::string::npos) {
            continue;
        }
        tmpLine = tmpLine.substr(pos);
        for (const auto &item: mountFailList) {
            if (tmpLine.find(item) == 0) {
                LOGE("find a fd from maps, %{public}s", tmpLine.c_str());
                inputStream.close();
                return true;
            }
        }
    }
    inputStream.close();
    return false;
}

bool MountManager::CheckSymlink(const std::string &path, std::list<std::string> &mountFailList)
{
    if (path.empty()) {
        return false;
    }
    char realPath[PATH_MAX_FOR_LINK];
    int res = readlink(path.c_str(), realPath, sizeof(realPath) - 1);
    if (res < 0) {
        LOGE("readlink failed for path, errno is %{public}d.", errno);
        return false;
    }
    realPath[res] = '\0';
    std::string realPathStr(realPath);
    for (const auto &item: mountFailList) {
        if (realPathStr.find(item) == 0) {
            LOGE("find a fd from link, %{public}s", realPathStr.c_str());
            return true;
        }
    }
    return false;
}


bool MountManager::CheckPathValid(const std::string &bundleNameStr, uint32_t userId)
{
    LOGI("[L2:MountManager] CheckPathValid: >>> ENTER <<< bundleName=%{public}s, userId=%{public}u",
        bundleNameStr.c_str(), userId);
    string completePath =
        SANDBOX_ROOT_PATH + to_string(userId) + "/" + bundleNameStr + EL2_BASE;
    if (!IsDir(completePath)) {
        LOGE("[L2:MountManager] CheckPathValid: <<< EXIT FAILED <<< Invalid directory path, path=%{public}s",
            completePath.c_str());
        return false;
    }

    if (!std::filesystem::is_empty(completePath)) {
        LOGE("[L2:MountManager] CheckPathValid: <<< EXIT FAILED <<< directory has been mounted, path=%{public}s",
            completePath.c_str());
        return false;
    }
    LOGI("[L2:MountManager] CheckPathValid: <<< EXIT SUCCESS <<< bundleName=%{public}s, userId=%{public}u",
        bundleNameStr.c_str(), userId);
    return true;
}

int32_t MountManager::MountCryptoPathAgain(uint32_t userId)
{
    LOGI("[L2:MountManager] MountCryptoPathAgain: >>> ENTER <<< userId=%{public}u", userId);
    filesystem::path rootDir(SANDBOX_ROOT_PATH + to_string(userId));
    std::error_code errCode;
    if (!exists(rootDir, errCode)) {
        LOGE("[L2:MountManager] MountCryptoPathAgain: <<< EXIT FAILED <<< root path not exists, rootDir=%{public}s",
            SANDBOX_ROOT_PATH);
        StorageRadar::ReportUserManager("MountCryptoPathAgain", userId, -ENOENT, "root path not exists");
        return -ENOENT;
    }

    InfoList<MountNodeInfo> sandboxMountNodeList;
    auto ret = UserPathResolver::GetSandboxMountNodeList(static_cast<int32_t>(userId), sandboxMountNodeList.data);
    if (ret != E_OK) {
        StorageRadar::ReportUserManager("MountCryptoPathAgain", userId, ret, "GetSandboxMountNodeList failed");
        LOGE("[L2:MountManager] MountCryptoPathAgain: <<< EXIT FAILED <<< GetSandboxMountNodeList failed,"
            "ret=%{public}d", ret);
        return ret;
    }

    filesystem::directory_iterator bundleNameList(rootDir);
    for (const auto &bundleName : bundleNameList) {
        const auto& filename = bundleName.path().filename();
        if (SANDBOX_EXCLUDE_PATH.find(filename) != SANDBOX_EXCLUDE_PATH.end()) {
            continue;
        }
        std::string bundleNameStr = filename.generic_string();
        std::string::size_type point = bundleNameStr.find(MOUNT_SUFFIX);
        if (point == string::npos) {
            continue;
        }
        bundleNameStr = bundleNameStr.substr(0, point);
        if (!CheckPathValid(bundleNameStr, userId)) {
            continue;
        }
        MountSandboxPath(userId, sandboxMountNodeList.data, bundleNameStr);
    }
    ClearSecondMountMap(userId);
    LOGI("[L2:MountManager] MountCryptoPathAgain: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
    return ret;
}

void MountManager::MountSandboxPath(uint32_t userId, const std::vector<MountNodeInfo> &sandboxMountNodeInfo,
    const std::string &bundleName)
{
    LOGI("[L2:MountManager] MountSandboxPath: >>> ENTER <<< userId=%{public}u, bundleName=%{public}s",
        userId, bundleName.c_str());
    std::string sanboxRootPath = SANDBOX_ROOT_PATH + to_string(userId) + '/' + bundleName;
    for (size_t i = 0; i < sandboxMountNodeInfo.size(); ++i) {
        const auto& nodeInfo = sandboxMountNodeInfo[i];
        auto dstPath = sanboxRootPath + nodeInfo.dstPath;
        auto srcPath = nodeInfo.srcPath;
        ReplaceAndCount(srcPath, PACKAGE_NAME_FLAG, bundleName);
        auto ret = nodeInfo.MountDir(srcPath, dstPath);
        if (ret != E_OK && ret != E_NON_EXIST) {
            std::string extraData = "dstPath=" + nodeInfo.dstPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("MountSandboxPath", userId, E_MOUNT_SANDBOX, extraData);
        }
    }
    LOGI("[L2:MountManager] MountSandboxPath: <<< EXIT SUCCESS <<< userId=%{public}u, bundleName=%{public}s",
        userId, bundleName.c_str());
}

void MountManager::MountPointToList(std::list<std::string> &hmdfsList, std::list<std::string> &hmfsList,
    std::list<std::string> &sharefsList, std::string &line, int32_t userId)
{
    if (line.empty()) {
        return;
    }
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string &hmdfsPrefix = hmdfsMntArgs.GetMountPointPrefix();
    const string &hmfsPrefix = hmdfsMntArgs.GetSandboxPath();
    const string &mntUserPrefix = hmdfsMntArgs.GetMntUserPath();
    const string &sharefsPrefix = hmdfsMntArgs.GetShareSrc();
    const string &cloudPrefix = hmdfsMntArgs.GetFullCloud();
    std::stringstream ss(line);
    std::string src;
    ss >> src;
    std::string dst;
    ss >> dst;
    std::string type;
    ss >> type;
    if (type == MOUNT_POINT_TYPE_HMDFS) {
        if (src.length() >= hmdfsPrefix.length() && src.substr(0, hmdfsPrefix.length()) == hmdfsPrefix) {
            hmdfsList.push_front(dst);
        }
        if (src.length() >= cloudPrefix.length() && src.substr(0, cloudPrefix.length()) == cloudPrefix) {
            hmdfsList.push_front(dst);
        }
        return;
    }
    if (type == MOUNT_POINT_TYPE_HMFS || type == MOUNT_POINT_TYPE_F2FS) {
        if (dst.length() >= hmfsPrefix.length() && dst.substr(0, hmfsPrefix.length()) == hmfsPrefix) {
            hmfsList.push_front(dst);
        }
        if (dst.length() >= mntUserPrefix.length() && dst.substr(0, mntUserPrefix.length()) == mntUserPrefix) {
            hmfsList.push_front(dst);
        }
        return;
    }
    if (type == MOUNT_POINT_TYPE_SHAREFS) {
        if (src.length() >= sharefsPrefix.length() && src.substr(0, sharefsPrefix.length()) == sharefsPrefix) {
            sharefsList.push_front(dst);
        }
        if (src.length() >= mntUserPrefix.length() && src.substr(0, mntUserPrefix.length()) == mntUserPrefix) {
            sharefsList.push_front(dst);
        }
        return;
    }
}

int32_t MountManager::FindMountPointsToMap(std::map<std::string, std::list<std::string>> &mountMap, int32_t userId)
{
    LOGI("[L2:MountManager] FindMountPointsToMap: >>> ENTER <<< userId=%{public}d", userId);
    std::ifstream inputStream(MOUNT_POINT_INFO, std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("[L2:MountManager] FindMountPointsToMap: <<< EXIT FAILED <<< unable to open /proc/mounts,"
            "errno=%{public}d", errno);
        return E_UMOUNT_PROC_MOUNTS_OPEN;
    }
    std::list<std::string> hmdfsList;
    std::list<std::string> hmfsList;
    std::list<std::string> sharefsList;
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        MountPointToList(hmdfsList, hmfsList, sharefsList, tmpLine, userId);
    }
    inputStream.close();
    mountMap[MOUNT_POINT_TYPE_HMDFS] = hmdfsList;
    mountMap[MOUNT_POINT_TYPE_HMFS] = hmfsList;
    mountMap[MOUNT_POINT_TYPE_SHAREFS] = sharefsList;
    hmdfsList.clear();
    hmfsList.clear();
    sharefsList.clear();
    LOGI("[L2:MountManager] FindMountPointsToMap: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::UMountAllPath(int32_t userId, std::list<std::string> &unMountFailList)
{
    LOGI("[L2:MountManager] UMountAllPath: >>> ENTER <<< userId=%{public}d", userId);
    std::map<std::string, std::list<std::string>> mountMap;
    int32_t res = FindMountPointsToMap(mountMap, userId);
    if (res != E_OK) {
        LOGE("[L2:MountManager] UMountAllPath: <<< EXIT FAILED <<< FindMountPointsToMap failed, res=%{public}d", res);
        return res;
    }
    int32_t result = E_OK;
    std::list<std::string> list = mountMap[MOUNT_POINT_TYPE_SHAREFS];
    int total = static_cast<int>(list.size());
    LOGI("[L2:MountManager] UMountAllPath: unmount sharefs path start, total=%{public}d", total);
    res = UMountByList(list, unMountFailList);
    if (res != E_OK) {
        LOGE("[L2:MountManager] UMountAllPath: failed to umount sharefs mount point, res=%{public}d", res);
        result = E_UMOUNT_SHAREFS;
    }

    list = mountMap[MOUNT_POINT_TYPE_HMFS];
    total = static_cast<int>(list.size());
    LOGI("[L2:MountManager] UMountAllPath: unmount hmfs path start, total=%{public}d", total);
    res = UMountByList(list, unMountFailList);
    if (res != E_OK) {
        LOGE("[L2:MountManager] UMountAllPath: failed to umount hmfs mount point, res=%{public}d", res);
        result = E_UMOUNT_HMFS;
    }
    UmountMntUserTmpfs(userId);

    list = mountMap[MOUNT_POINT_TYPE_HMDFS];
    total = static_cast<int>(list.size());
    LOGI("[L2:MountManager] UMountAllPath: unmount hmdfs path start, total=%{public}d", total);
    res = UMountHmdfsByList(userId, list, unMountFailList);
    if (res != E_OK) {
        LOGE("[L2:MountManager] UMountAllPath: failed to umount hmdfs mount point, res=%{public}d", res);
        result = E_UMOUNT_HMDFS;
    }
    if (!unMountFailList.empty()) {
        std::string extraData = "dstPath=" + ListToString(unMountFailList) + ",kernelCode=" + to_string(result);
        StorageRadar::ReportUserManager("UMountAllPath", userId, E_UMOUNT_ALL_PATH, extraData);
    }
    std::map<std::string, std::list<std::string>>().swap(mountMap);
    LOGI("[L2:MountManager] UMountAllPath: <<< EXIT SUCCESS <<< userId=%{public}d, result=%{public}d", userId, result);
    return result;
}

int32_t MountManager::UMountHmdfsByList(int32_t userId, std::list<std::string> &list,
    std::list<std::string> &unMountFailList)
{
    LOGI("[L2:MountManager] UMountHmdfsByList: >>> ENTER <<< userId=%{public}d, list.size()=%{public}zu",
        userId, list.size());
    if (list.empty()) {
        LOGI("[L2:MountManager] UMountHmdfsByList: <<< EXIT SUCCESS <<< list is empty");
        return E_OK;
    }
    int32_t result = E_OK;
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    for (std::string &path: list) {
        LOGD("[L2:MountManager] UMountHmdfsByList: umount path %{public}s", path.c_str());
        if (IsSysMountPoint(userId, path)) {
            continue;
        }
        int32_t res = UMount(path);
        if (res != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("[L2:MountManager] UMountHmdfsByList: failed to unmount path, errno=%{public}d", errno);
            result = errno;
            unMountFailList.push_back(path);
        }
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT: UMOUNT HMDFS BY LIST",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UMOUNT: UMOUNT HMDFS BY LIST, delayTime = %{public}s", delay.c_str());
    LOGI("[L2:MountManager] UMountHmdfsByList: <<< EXIT SUCCESS <<< userId=%{public}d, result=%{public}d",
        userId, result);
    return result;
}

bool MountManager::IsSysMountPoint(const int32_t userId, std::string &path)
{
    auto count = static_cast<int32_t>(SYS_PATH.size());
    for (int i = 0; i < count; i++) {
        std::string tempPath = SYS_PATH[i];
        ReplaceAndCount(tempPath, CURRENT_USER_ID_FLAG, to_string(userId));
        if (path == tempPath) {
            return true;
        }
    }
    return false;
}

int32_t MountManager::UMountByList(std::list<std::string> &list, std::list<std::string> &unMountFailList)
{
    LOGI("[L2:MountManager] UMountByList: >>> ENTER <<< list.size()=%{public}zu", list.size());
    if (list.empty()) {
        LOGI("[L2:MountManager] UMountByList: <<< EXIT SUCCESS <<< list is empty");
        return E_OK;
    }
    int32_t result = E_OK;
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    for (const std::string &path: list) {
        LOGD("[L2:MountManager] UMountByList: umount path %{public}s", path.c_str());
        int32_t res = UMount(path);
        if (res != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("[L2:MountManager] UMountByList: failed to unmount path, errno=%{public}d", errno);
            result = errno;
            unMountFailList.push_back(path);
        }
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT: UMOUNT BY LIST",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, StorageService::DEFAULT_USERID);
    LOGI("[L2:MountManager] UMountByList: <<< EXIT SUCCESS <<< result=%{public}d", result);
    return result;
}

int32_t MountManager::UMountByListWithDetach(std::list<std::string> &list)
{
    LOGI("[L2:MountManager] UMountByListWithDetach: >>> ENTER <<< list.size()=%{public}zu", list.size());
    if (list.empty()) {
        LOGI("[L2:MountManager] UMountByListWithDetach: <<< EXIT SUCCESS <<< list is empty");
        return E_OK;
    }
    int32_t result = E_OK;
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    for (const std::string &path: list) {
        LOGD("[L2:MountManager] UMountByListWithDetach: umount path %{public}s", path.c_str());
        int32_t res = UMount2(path, MNT_DETACH);
        if (res != E_OK && errno != ENOENT && errno != EINVAL) {
            LOGE("[L2:MountManager] UMountByListWithDetach: failed to unmount path, errno=%{public}d", errno);
            result = errno;
        }
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT LIST WITH DETACH",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, StorageService::DEFAULT_USERID);
    LOGI("[L2:MountManager] UMountByListWithDetach: <<< EXIT SUCCESS <<< result=%{public}d", result);
    return result;
}

bool MountManager::SupportHmdfs()
{
    LOGI("[L2:MountManager] SupportHmdfs: >>> ENTER <<<");
    char hmdfsEnable[HMDFS_VAL_LEN + 1] = {"false"};
    int ret = GetParameter(HMDFS_SYS_CAP, "", hmdfsEnable, HMDFS_VAL_LEN);
    LOGI("[L2:MountManager] SupportHmdfs: GetParameter hmdfsEnable %{public}s, ret %{public}d", hmdfsEnable, ret);
    if (strncmp(hmdfsEnable, "true", HMDFS_TRUE_LEN) == 0) {
        return true;
    }
    return false;
}

int32_t MountManager::LocalMount(int32_t userId, const std::vector<MountNodeInfo> &hmdfsMountNodeList)
{
    LOGI("[L2:MountManager] LocalMount: >>> ENTER <<< userId=%{public}d", userId);
    for (const auto &nodeInfo : hmdfsMountNodeList) {
        const auto &options = nodeInfo.options;
        if (options.find("local_hmdfs") == options.end()) {
            continue;
        }
        if (nodeInfo.MountDir() != E_OK) {
            std::string extraData = "dstPath=" + nodeInfo.dstPath + ",kernelCode = " + to_string(errno);
            StorageRadar::ReportUserManager("LocalMount", userId, E_MOUNT_HMDFS, extraData);
            LOGE("[L2:MountManager] LocalMount: <<< EXIT FAILED <<< MountDir failed");
            return E_MOUNT_HMDFS;
        }
    }
    LOGI("[L2:MountManager] LocalMount: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

static void RmExistDir(const std::string &dirPath)
{
    if (access(dirPath.c_str(), 0) == 0) {
        if (!RmDirRecurse(dirPath)) {
            LOGE("Failed to remove dir %{public}s", dirPath.c_str());
        }
    }
}

static void ClearRedundantResources(int32_t userId)
{
    std::string sharePath = StringPrintf("/data/service/el2/%d/share", userId);
    filesystem::path rootDir(sharePath);
    std::error_code errCode;
    if (!exists(rootDir, errCode)) {
        LOGE("Bundles share path not exists, rootDir is %{public}s", sharePath.c_str());
        return;
    }

    filesystem::directory_iterator bundleNameList(rootDir);
    for (const auto &bundleName : bundleNameList) {
        RmExistDir(bundleName.path().generic_string() + "/r");
        RmExistDir(bundleName.path().generic_string() + "/rw");
    }
}

int32_t MountManager::MountByUser(int32_t userId)
{
    LOGI("[L2:MountManager] MountByUser: >>> ENTER <<< userId=%{public}d", userId);
    bool isCeEncrypt = false;
    int ret = KeyManager::GetInstance().GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("[L2:MountManager] MountByUser: <<< EXIT FAILED <<< User %{public}d de has not decrypt", userId);
        return E_KEY_NOT_ACTIVED;
    }
    std::thread thread([userId]() { ClearRedundantResources(userId); });
    thread.detach();
    CreateVirtualDirs(userId);

    int32_t mountHmdfsRes = MountFileSystem(userId);
    if (mountHmdfsRes != E_OK) {
        LOGE("[L2:MountManager] MountByUser: <<< EXIT FAILED <<< MountFileSystem failed, ret=%{public}d",
            mountHmdfsRes);
        return mountHmdfsRes;
    }
    QuotaManager::GetInstance().SetQuotaPrjId(StringPrintf(SHARE_PATH, userId), 0, true);
    LOGI("[L2:MountManager] MountByUser: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::MountFileSystem(int32_t userId)
{
    LOGI("[L2:MountManager] MountFileSystem: >>> ENTER <<< userId=%{public}d", userId);
    auto ret = MountHmdfs(userId);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] MountFileSystem: <<< EXIT FAILED <<< MountHmdfs failed, ret=%{public}d", ret);
        return ret;
    }
    MountSharefs(userId);
    MountAppdata(userId, false);
    LOGI("[L2:MountManager] MountFileSystem: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::MountHmdfs(int32_t userId)
{
    LOGI("[L2:MountManager] MountHmdfs: >>> ENTER <<< userId=%{public}d", userId);
    InfoList<MountNodeInfo> hmdfsMountNodeList;
    auto ret = UserPathResolver::GetHmdfsMountNodeList(userId, hmdfsMountNodeList.data);
    if (ret != E_OK) return ret;
    
    if (!SupportHmdfs()) {
        return LocalMount(userId, hmdfsMountNodeList.data);
    }

    SystemMountManager::GetInstance().MountCloudByUserId(userId);

    for (const auto &nodeInfo : hmdfsMountNodeList.data) {
        const auto &options = nodeInfo.options;
        if (options.find("local_hmdfs") != options.end()) {
            continue;
        }
        if (nodeInfo.MountDir() != E_OK) {
            std::string extraData = "dstPath=" + nodeInfo.dstPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("MountHmdfs", userId, E_MOUNT_HMDFS, extraData);
            LOGE("[L2:MountManager] MountHmdfs: <<< EXIT FAILED <<< MountDir failed");
            return E_MOUNT_HMDFS;
        }
    }

    for (auto relativePath : HMDFS_SUFFIX) {
        Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
        ret = chown(hmdfsMntArgs.GetCtrlPath().c_str(), OID_DFS, OID_SYSTEM);
        if (ret != 0) {
            LOGE("[L2:MountManager] MountHmdfs: failed to chown hmdfs sysfs node, err=%{public}d", errno);
        }
    }
    LOGI("[L2:MountManager] MountHmdfs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::MountSharefs(int32_t userId)
{
    LOGI("[L2:MountManager] MountSharefs: >>> ENTER <<< userId=%{public}d", userId);
    Utils::MountArgument sharefsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));

    MountNodeInfo sharefsMountNode {
        .srcPath = sharefsMntArgs.GetShareSrc(),
        .dstPath = sharefsMntArgs.GetShareDst(),
        .fsType = "sharefs",
        .mountFlags = MS_NODEV,
        .data = "user_id=" + std::to_string(userId),
    };
    if (IsPathMounted(sharefsMountNode.dstPath)) {
        LOGI("[L2:MountManager] MountSharefs: <<< EXIT SUCCESS <<< path has mounted, %{public}s",
            sharefsMountNode.dstPath.c_str());
        return E_OK;
    }
    if (sharefsMountNode.MountDir() != E_OK) {
        std::string extraData = "dstPath=" + sharefsMountNode.dstPath + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("SharefsMount", userId, E_MOUNT_SHAREFS, extraData);
        LOGE("[L2:MountManager] MountSharefs: <<< EXIT FAILED <<< MountDir failed");
        return E_MOUNT_SHAREFS;
    }
    LOGI("[L2:MountManager] MountSharefs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::LocalUMount(int32_t userId)
{
    LOGI("[L2:MountManager] LocalUMount: >>> ENTER <<< userId=%{public}d", userId);
    InfoList<MountNodeInfo> hmdfsMountNodeList;
    auto ret = UserPathResolver::GetHmdfsMountNodeList(userId, hmdfsMountNodeList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] LocalUMount: <<< EXIT FAILED <<< GetHmdfsMountNodeList failed, ret=%{public}d", ret);
        return ret;
    }

    for (const auto &nodeInfo : hmdfsMountNodeList.data) {
        if (nodeInfo.options.find("local_hmdfs") == nodeInfo.options.end()) {
            continue;
        }
        auto err = UMount(nodeInfo.dstPath);
        if (err != E_OK && errno != ENOENT && errno != EINVAL) {
            ret = E_UMOUNT_LOCAL_CLOUD;
        }
    }
    LOGI("[L2:MountManager] LocalUMount: <<< EXIT SUCCESS <<< userId=%{public}d, ret=%{public}d", userId, ret);
    return ret;
}

int32_t MountManager::UmountByUser(int32_t userId)
{
    LOGI("[L2:MountManager] UmountByUser: >>> ENTER <<< userId=%{public}d", userId);
    int32_t res = E_OK;
    if (!SupportHmdfs() && LocalUMount(userId) != E_OK) {
        res = E_UMOUNT_LOCAL;
    } else {
        int unMount = UmountFileSystem(userId);
        if (unMount != E_OK) {
            res = unMount;
        }
    }

    LOGI("[L2:MountManager] UmountByUser: umount cloud mount point start.");
    int32_t cloudUMount = SystemMountManager::GetInstance().UMountCloudByUserId(userId);
    res = (cloudUMount != E_OK) ? cloudUMount : res;
    UMountMediaFuse(userId);
    FindSaFd(userId);
    LOGI("[L2:MountManager] UmountByUser: <<< EXIT SUCCESS <<< userId=%{public}d, res=%{public}d", userId, res);
    return res;
}

int32_t MountManager::FindSaFd(int32_t userId)
{
    LOGI("[L2:MountManager] FindSaFd: >>> ENTER <<< userId=%{public}d", userId);
    std::list<std::string> list;
    for (const std::string &item: FD_PATH) {
        std::string temp = item;
        ReplaceAndCount(temp, CURRENT_USER_ID_FLAG, to_string(userId));
        list.push_back(temp);
    }
    std::vector<ProcessInfo> proInfos;
    std::list<std::string> excludeProcess;
    FindProcess(list, proInfos, excludeProcess);
    if (!proInfos.empty()) {
        std::string extraData = "process=" + ProcessToString(proInfos);
        StorageRadar::ReportUserManager("FindSaFd", userId, E_UMOUNT_FIND_FD, extraData);
    }
    LOGI("[L2:MountManager] FindSaFd: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::UmountFileSystem(int32_t userId)
{
    LOGI("[L2:MountManager] UmountFileSystem: >>> ENTER <<< userId=%{public}d", userId);
    std::list<std::string> unMountFailList;
    int32_t unMountRes = UMountAllPath(userId, unMountFailList);
    for (const auto &item: HMDFS_SUFFIX) {
        Utils::MountArgument mountArg(Utils::MountArgumentDescriptors::Alpha(userId, item));
        unMountFailList.push_back(mountArg.GetFullDst());
    }
    if (CheckSysFs(userId) || unMountRes != E_OK) {
        ForbidOpen(userId);
        LOGE("[L2:MountManager] UmountFileSystem: force umount failed, try to kill process, res=%{public}d",
            unMountRes);
        FindAndKillProcess(userId, unMountFailList, unMountRes);
    }
    LOGE("[L2:MountManager] UmountFileSystem: try to force umount again.");
    std::list<std::string> tempList;
    int32_t unMountAgain = UMountByList(unMountFailList, tempList);
    if (unMountAgain == E_OK) {
        std::list<std::string>().swap(unMountFailList);
        LOGI("[L2:MountManager] UmountFileSystem: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
        return E_OK;
    }
    LOGE("[L2:MountManager] UmountFileSystem: force umount again failed, try to kill process again, res=%{public}d",
        unMountAgain);
    FindAndKillProcess(userId, unMountFailList, unMountAgain);
    LOGE("[L2:MountManager] UmountFileSystem: try to umount by detach.");
    auto ret = UMountByListWithDetach(unMountFailList) == E_OK ? E_OK : E_UMOUNT_DETACH;
    std::list<std::string>().swap(unMountFailList);
    LOGI("[L2:MountManager] UmountFileSystem: <<< EXIT SUCCESS <<< userId=%{public}d, ret=%{public}d", userId, ret);
    return ret;
}

bool MountManager::CheckSysFs(int32_t userId)
{
    for (const auto &item: HMDFS_SUFFIX) {
        Utils::MountArgument mountArg(Utils::MountArgumentDescriptors::Alpha(userId, item));
        std::string path = mountArg.GetFullDst();
        if (IsSysFsInUse(path)) {
            return true;
        }
    }
    return false;
}

bool MountManager::IsSysFsInUse(const std::string &path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == nullptr) {
        LOGE("sys fs fopen fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        return true;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("sys fs fileno fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        (void)fclose(f);
        return true;
    }
    int inUse = -1;
    int cmd = _IOR(0xAC, 77, int);
    if (ioctl(fd, cmd, &inUse) < 0) {
        LOGE("sys fs ioctl fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        (void)fclose(f);
        return true;
    }
    if (inUse > 0) {
        LOGE("sys fs is in use, path is %{public}s, use count is %{public}d.", path.c_str(), inUse);
        (void)fclose(f);
        return true;
    }
    LOGE("sys fs is not use, path is %{public}s.", path.c_str());
    (void)fclose(f);
    return false;
}

void MountManager::ForbidOpen(int32_t userId)
{
    for (const auto &item: HMDFS_SUFFIX) {
        Utils::MountArgument mountArg(Utils::MountArgumentDescriptors::Alpha(userId, item));
        std::string path = mountArg.GetFullDst();
        FILE *f = fopen(path.c_str(), "r");
        if (f == nullptr) {
            LOGE("forbid fopen fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
            return;
        }
        int fd = fileno(f);
        if (fd < 0) {
            LOGE("forbid fileno fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
            (void)fclose(f);
            return;
        }
        if (ioctl(fd, HMDFS_IOC_FORBID_OPEN) < 0) {
            LOGE("forbid ioctl fail, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        } else {
            LOGE("forbid ioctl success, path is %{public}s.", path.c_str());
        }
        (void)fclose(f);
    }
}

int32_t MountManager::FindAndKillProcess(int32_t userId, std::list<std::string> &unMountFailList, int32_t radar)
{
    LOGI("[L2:MountManager] FindAndKillProcess: >>> ENTER <<< userId=%{public}d, unMountFailList.size()=%{public}zu",
        userId, unMountFailList.size());
    std::vector<ProcessInfo> processInfos;
    std::list<std::string> excludeProcess = {"(storage_daemon)"};
    FindProcess(unMountFailList, processInfos, excludeProcess);
    if (processInfos.empty()) {
        LOGE("[L2:MountManager] FindAndKillProcess: <<< EXIT FAILED <<< no process find");
        return E_UMOUNT_NO_PROCESS_FIND;
    }
    std::string extraData = "process=" + ProcessToString(processInfos) + ",kernelCode=" + to_string(radar);
    StorageRadar::ReportUserManager("FindAndKillProcess", userId, E_UMOUNT_FIND_PROCESS, extraData);

    std::vector<ProcessInfo> killFailList;
    KillProcess(processInfos, killFailList);
    if (!killFailList.empty()) {
        std::string info = ProcessToString(killFailList);
        LOGE("[L2:MountManager] FindAndKillProcess: <<< EXIT FAILED <<< kill process failed");
        return E_UMOUNT_PROCESS_KILL;
    }
    LOGI("[L2:MountManager] FindAndKillProcess: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::CreateVirtualDirs(int32_t userId)
{
    LOGI("[L2:MountManager] CreateVirtualDirs: >>> ENTER <<< userId=%{public}d", userId);
    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetVirtualPath(userId, dirInfoList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] CreateVirtualDirs: <<< EXIT FAILED <<< GetVirtualPath failed, ret=%{public}d", ret);
        return ret;
    }
    for (const auto &dirInfo : dirInfoList.data) {
        if (dirInfo.MakeDir() != E_OK) {
            std::string extraData = "dirPath=" + dirInfo.path + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("CreateVirtualDirs", userId, E_CREATE_DIR_VIRTUAL, extraData);
            ret = E_CREATE_DIR_VIRTUAL;
        }
    }
    LOGI("[L2:MountManager] CreateVirtualDirs: <<< EXIT SUCCESS <<< userId=%{public}d, ret=%{public}d", userId, ret);
    return ret;
}

int32_t MountManager::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("[L2:MountManager] MountDfsDocs: >>> ENTER <<< userId=%{public}d, relativePath=%{public}s",
        userId, relativePath.c_str());
    std::string dstPath = StringPrintf("/mnt/data/%d/hmdfs/%s/", userId, deviceId.c_str());
    if (!PrepareDir(dstPath, MODE_0711, OID_FILE_MANAGER, OID_FILE_MANAGER)) {
        LOGE("[L2:MountManager] MountDfsDocs: <<< EXIT FAILED <<< PrepareDir failed");
        return E_PREPARE_DIR;
    }

    std::regex pathRegex("^[a-zA-Z0-9_/]+$");
    if (relativePath.empty() || relativePath.length() > PATH_MAX || !std::regex_match(relativePath, pathRegex)) {
        LOGE("[L2:MountManager] MountDfsDocs: <<< EXIT FAILED <<< invalid relativePath");
        return E_PARAMS_INVALID;
    }

    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    std::string srcPath = hmdfsMntArgs.GetFullDst() + "/device_view/" + networkId + "/files/Docs/";
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int32_t ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("[L2:MountManager] MountDfsDocs: <<< EXIT FAILED <<< mount bind failed, errno=%{public}d", errno);
        return E_USER_MOUNT_ERR;
    }
    auto delay = StorageService::StorageRadar::ReportDuration(" MOUNT: MOUNT_DFS_DOCS",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] MountDfsDocs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("MountManager::UMountDfsDocs start.");

    std::string dstPath = StringPrintf("/mnt/data/%d/hmdfs/%s", userId, deviceId.c_str());
    sync();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int32_t ret = UMount2(dstPath, MNT_DETACH);
    if (ret != E_OK && errno != ERROR_FILE_NOT_FOUND) {
        LOGE("[L2:MountManager] UMountDfsDocs: UMountDfsDocs unmount bind failed, srcPath is"
            "%{public}s errno is %{public}d", dstPath.c_str(), errno);
        return errno;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT DFS DOCS",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] UMountDfsDocs: SD_DURATION: delayTime=%{public}s", delay.c_str());
    if (!filesystem::is_empty(dstPath)) {
        LOGE("[L2:MountManager] UMountDfsDocs: <<< EXIT FAILED <<< Failed to umount");
        return E_NOT_EMPTY_TO_UMOUNT;
    }
    if (!rmdir(dstPath.c_str())) {
        LOGE("[L2:MountManager] UMountDfsDocs: Failed to remove dir %{public}s", dstPath.c_str());
    }
    LOGI("[L2:MountManager] UMountDfsDocs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

bool MountManager::CheckMountFileByUser(int32_t userId)
{
    LOGI("[L2:MountManager] CheckMountFileByUser: >>> ENTER <<< userId=%{public}d", userId);
    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetVirtualPath(userId, dirInfoList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] CheckMountFileByUser: <<< EXIT FAILED <<< GetVirtualPath failed, ret=%{public}d", ret);
        return false;
    }
    for (const DirInfo &dir : dirInfoList.data) {
        const auto &path = dir.path;
        if (CloudAndFuseDirFlag(path) || MediaFuseDirFlag(path)) {
            continue;
        }
        if (access(path.c_str(), 0) != 0) {
            LOGI("[L2:MountManager] CheckMountFileByUser: <<< EXIT SUCCESS <<< exists=false, virtualDir=%{public}s",
                path.c_str());
            return false;
        }
    }
    LOGI("[L2:MountManager] CheckMountFileByUser: <<< EXIT SUCCESS <<< exists=true");
    return true;
}

bool MountManager::CloudAndFuseDirFlag(const std::string &path)
{
    if (path.empty()) {
        return true;
    }
    std::regex cloudPattern("\\/mnt\\/data.*cloud");
    if (std::regex_match(path.c_str(), cloudPattern)) {
        return true;
    }
    std::regex cloudFusePattern("\\/mnt\\/data.*cloud_fuse");
    if (std::regex_match(path.c_str(), cloudFusePattern)) {
        return true;
    }
    return false;
}

bool MountManager::MediaFuseDirFlag(const std::string &path)
{
    if (path.empty()) {
        return true;
    }
    std::regex mediaFusePattern("\\/mnt\\/data.*media_fuse/Photo");
    if (std::regex_match(path.c_str(), mediaFusePattern)) {
        return true;
    }
    return false;
}

void MountManager::GetAllUserId(std::vector<int32_t> &userIds)
{
    const std::string path(APP_EL1_PATH);
    if (!DirExist(path)) {
        return;
    }
    for (const auto &entry : filesystem::directory_iterator(path)) {
        if (!entry.is_directory()) {
            continue;
        }
        std::string subPath = entry.path().filename().string();
        if (!StringIsNumber(subPath)) {
            continue;
        }
        int32_t userId = atoi(subPath.c_str());
        if (userId < DEFAULT_USERID) {
            continue;
        }
        userIds.push_back(userId);
    }
}

bool MountManager::DirExist(const std::string &dir)
{
    filesystem::path filePath(dir);
    std::error_code errCode;
    if (!exists(filePath, errCode)) {
        LOGE("dir not exists, %{public}s", dir.c_str());
        return false;
    }
    return true;
}

int32_t MountManager::PrepareAppdataDirByUserId(int32_t userId)
{
    LOGI("[L2:MountManager] PrepareAppdataDirByUserId: >>> ENTER <<< userId=%{public}d", userId);
    InfoList<DirInfo> dirInfoList;
    auto ret = UserPathResolver::GetAppdataPath(userId, dirInfoList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] PrepareAppdataDirByUserId: <<< EXIT FAILED <<< GetAppdataPath failed, ret=%{public}d",
            ret);
        return ret;
    }
    for (const auto &dirInfo : dirInfoList.data) {
        if (dirInfo.MakeDir() != E_OK) {
            std::string extraData = "dstPath=" + dirInfo.path + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("PrepareAppdataDirByUserId", userId, E_CREATE_DIR_APPDATA, extraData);
            LOGE("[L2:MountManager] PrepareAppdataDirByUserId: <<< EXIT FAILED <<< MakeDir failed");
            return E_CREATE_DIR_APPDATA;
        }
    }
    MountAppdata(userId, true);
    LOGI("[L2:MountManager] PrepareAppdataDirByUserId: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::MountAppdata(int32_t userId, bool beforeStartup)
{
    LOGI("[L2:MountManager] MountAppdata: >>> ENTER <<< userId=%{public}d, beforeStartup=%{public}d",
        userId, beforeStartup);
    InfoList<MountNodeInfo> appdataMountNodeList;
    auto ret = UserPathResolver::GetAppDataMountNodeList(userId, appdataMountNodeList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] MountAppdata: <<< EXIT FAILED <<< GetAppDataMountNodeList failed, ret=%{public}d", ret);
        return ret;
    }
    for (const auto &nodeInfo : appdataMountNodeList.data) {
        const auto &options = nodeInfo.options;
        bool isBeforeStartupMountNode = (options.find("before_startup") != options.end());
        if (isBeforeStartupMountNode != beforeStartup) {
            continue;
        }
        if (nodeInfo.MountDir() != E_OK) {
            std::string extraData = "dstPath=" + nodeInfo.dstPath + ",kernelCode=" + to_string(errno);
            StorageRadar::ReportUserManager("MountAppdata", userId, E_MOUNT_BIND_AND_REC, extraData);
        }
    }
    LOGI("[L2:MountManager] MountAppdata: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::PrepareAppdataDir(int32_t userId)
{
    LOGI("[L2:MountManager] PrepareAppdataDir: >>> ENTER <<< userId=%{public}d", userId);
    if (userId == 0) {
        std::vector<int32_t> userIds;
        GetAllUserId(userIds);
        if (userIds.empty()) {
            LOGI("[L2:MountManager] PrepareAppdataDir: <<< EXIT SUCCESS <<< userIds empty");
            return E_OK;
        }
        for (const int32_t &item: userIds) {
            PrepareAppdataDirByUserId(item);
        }
    } else {
        PrepareAppdataDirByUserId(userId);
    }
    LOGI("[L2:MountManager] PrepareAppdataDir: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::UmountMntUserTmpfs(int32_t userId)
{
    LOGI("[L2:MountManager] UmountMntUserTmpfs: >>> ENTER <<< userId=%{public}d", userId);
    Utils::MountArgument mountArgument(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    std::string path = mountArgument.GetSharefsDocCurPath() + "/appdata";
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int32_t res = UMount2(path, MNT_DETACH);
    if (res != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("[L2:MountManager] UmountMntUserTmpfs: failed to umount with detach, errno=%{public}d", errno);
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT SHARE FS DOC CUR APPDATA",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UMOUNT2: UMOUNT SHARE FS DOC CUR APPDATA, delayTime = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    path = mountArgument.GetCurOtherAppdataPath();
    res = UMount2(path, MNT_DETACH);
    if (res != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("[L2:MountManager] UmountMntUserTmpfs: failed to umount with detach, errno=%{public}d", errno);
    }
    delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT OTHER TEMP CUR APPDATA",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] UmountMntUserTmpfs: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::MountMediaFuse(int32_t userId, int32_t &devFd)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("[L2:MountManager] MountMediaFuse: >>> ENTER <<< userId=%{public}d", userId);
    UMountMediaFuse(userId);
    Utils::MountArgument mediaMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string path = mediaMntArgs.GetFullMediaFuse();
    // open fuse
    devFd = open("/dev/fuse", O_RDWR);
    if (devFd < 0) {
        LOGE("[L2:MountManager] MountMediaFuse: <<< EXIT FAILED <<< open /dev/fuse fail, errno=%{public}d", errno);
        return E_USER_MOUNT_ERR;
    }
    // mount fuse mountpoint
    string opt = StringPrintf("fd=%i,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=\"u:object_r:hmdfs:s0\","
        "fscontext=u:object_r:hmdfs:s0",
        devFd);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = Mount("/dev/fuse", path.c_str(), "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("[L2:MountManager] MountMediaFuse: <<< EXIT FAILED <<< mount fuse failed, ret=%{public}d,"
            "errno=%{public}d", ret, errno);
        close(devFd);
        std::string extraData = "dstPath=" + path + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("MountMediaFuse", userId, E_MOUNT_MEDIA_FUSE, extraData);
        return E_MOUNT_MEDIA_FUSE;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("MOUNT: MOUNT MEDIA FUSE",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] MountMediaFuse: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
#endif
    return E_OK;
}

int32_t MountManager::UMountMediaFuse(int32_t userId)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("[L2:MountManager] UMountMediaFuse: >>> ENTER <<< userId=%{public}d", userId);
    int32_t err = E_OK;
    Utils::MountArgument mediaMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string path = mediaMntArgs.GetFullMediaFuse();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    err = UMount2(path, MNT_DETACH);
    if (err != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("[L2:MountManager] UMountMediaFuse: <<< EXIT FAILED <<< umount failed, errno=%{public}d", errno);
        std::string extraData = "dstPath=" + path + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("UMountMediaFuse", userId, E_UMOUNT_MEDIA_FUSE, extraData);
        return E_UMOUNT_MEDIA_FUSE;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT MEDIA FUSE",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] UMountMediaFuse: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
#endif
    return E_OK;
}

int32_t MountManager::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    LOGI("[L2:MountManager] MountFileMgrFuse: >>> ENTER <<< userId=%{public}d, path=%{public}s", userId, path.c_str());
    fuseFd = open("/dev/fuse", O_RDWR);
    if (fuseFd < 0) {
        LOGE("[L2:MountManager] MountFileMgrFuse: <<< EXIT FAILED <<< open /dev/fuse fail, errno=%{public}d", errno);
        return E_OPEN_FUSE;
    }
    LOGI("[L2:MountManager] MountFileMgrFuse: open fuse end.");
    string opt = StringPrintf("fd=%i,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=\"u:object_r:hmdfs:s0\","
        "fscontext=u:object_r:hmdfs:s0",
        fuseFd);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = Mount("/dev/fuse", path.c_str(), "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("[L2:MountManager] MountFileMgrFuse: <<< EXIT FAILED <<< mount fuse failed, ret=%{public}d,"
            "errno=%{public}d", ret, errno);
        close(fuseFd);
        std::string extraData = "dstPath=" + path + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("MountFileMgrFuse", userId, E_MOUNT_FILE_MGR_FUSE, extraData);
        return E_MOUNT_FILE_MGR_FUSE;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("MOUNT: FILE MGR FUSE",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] MountFileMgrFuse: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    LOGI("[L2:MountManager] UMountFileMgrFuse: >>> ENTER <<< userId=%{public}d, path=%{public}s", userId, path.c_str());
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int32_t ret = UMount2(path, MNT_DETACH);
    if (ret != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("[L2:MountManager] UMountFileMgrFuse: <<< EXIT FAILED <<< umount failed, ret=%{public}d,"
            "errno=%{public}d", ret, errno);
        std::string extraData = "dstPath=" + path + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("UMountFileMgrFuse", userId, E_UMOUNT_FILE_MGR_FUSE, extraData);
        return E_UMOUNT_FILE_MGR_FUSE;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT FILE MGR FUSE",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("[L2:MountManager] UMountFileMgrFuse: <<< EXIT SUCCESS <<< userId=%{public}d", userId);
    return E_OK;
}

int32_t MountManager::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    LOGI("[L2:MountManager] IsFileOccupied: >>> ENTER <<< path=%{public}s, inputList.size()=%{public}zu",
        path.c_str(), inputList.size());
    if (path.empty()) {
        LOGE("[L2:MountManager] IsFileOccupied: <<< EXIT FAILED <<< path is empty");
        return E_PARAMS_INVALID;
    }
    if (inputList.empty() && path.back() == FILE_SEPARATOR_CHAR && path != FILE_MGR_ROOT_PATH) {
        LOGI("[L2:MountManager] IsFileOccupied: only modify dir.");
        return OpenProcForPath(path, isOccupy, true);
    }
    if (inputList.empty() && path.back() != FILE_SEPARATOR_CHAR) {
        LOGI("[L2:MountManager] IsFileOccupied: only modify file.");
        return OpenProcForPath(path, isOccupy, false);
    }
    if (path == FILE_MGR_ROOT_PATH || (!inputList.empty() && path.back() == FILE_SEPARATOR_CHAR)) {
        LOGI("[L2:MountManager] IsFileOccupied: multi select file, input size=%{public}zu", inputList.size());
        std::set<std::string> occupyFiles;
        int32_t ret = OpenProcForMulti(path, occupyFiles);
        if (ret != E_OK) {
            LOGE("[L2:MountManager] IsFileOccupied: <<< EXIT FAILED <<< open proc failed, ret=%{public}d", ret);
            return ret;
        }
        if (occupyFiles.empty()) {
            LOGI("[L2:MountManager] IsFileOccupied: there has no occupy.");
            isOccupy = false;
            LOGI("[L2:MountManager] IsFileOccupied: <<< EXIT SUCCESS <<< isOccupy=false");
            return E_OK;
        }
        if (path == FILE_MGR_ROOT_PATH) {
            for (const std::string &item: occupyFiles) {
                outputList.push_back(item);
            }
            isOccupy = !outputList.empty();
            LOGI("[L2:MountManager] IsFileOccupied: <<< EXIT SUCCESS <<< output.size=%{public}zu", outputList.size());
            return E_OK;
        }
        for (const std::string &item: inputList) {
            if (occupyFiles.find(item) != occupyFiles.end()) {
                outputList.push_back(item);
            }
        }
        isOccupy = !outputList.empty();
        LOGI("[L2:MountManager] IsFileOccupied: <<< EXIT SUCCESS <<< output.size=%{public}zu", outputList.size());
        return E_OK;
    }
    LOGE("[L2:MountManager] IsFileOccupied: <<< EXIT FAILED <<< param is invalid");
    return E_PARAMS_INVALID;
}

int32_t MountManager::OpenProcForMulti(const std::string &path, std::set<std::string> &occupyFiles)
{
    LOGI("[L2:MountManager] OpenProcForMulti: >>> ENTER <<< path=%{public}s", path.c_str());
    auto procDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(PID_PROC), closedir);
    if (!procDir) {
        LOGE("[L2:MountManager] OpenProcForMulti: <<< EXIT FAILED <<< failed to open dir proc, err=%{public}d", errno);
        return E_UMOUNT_PROC_OPEN;
    }
    struct dirent *entry;
    while ((entry = readdir(procDir.get())) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        std::string name = entry->d_name;
        if (!StringIsNumber(name)) {
            continue;
        }
        std::string pidPath = std::string(PID_PROC) + FILE_SEPARATOR_CHAR + name;
        FindProcForMulti(pidPath, path, occupyFiles);
    }
    LOGI("[L2:MountManager] OpenProcForMulti: <<< EXIT SUCCESS <<< occupyFiles.size()=%{public}zu", occupyFiles.size());
    return E_OK;
}

int32_t MountManager::OpenProcForPath(const std::string &path, bool &isOccupy, bool isDir)
{
    LOGI("[L2:MountManager] OpenProcForPath: >>> ENTER <<< path=%{public}s, isDir=%{public}d", path.c_str(), isDir);
    auto procDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(PID_PROC), closedir);
    if (!procDir) {
        LOGE("[L2:MountManager] OpenProcForPath: <<< EXIT FAILED <<< failed to open dir proc, err=%{public}d", errno);
        return E_UMOUNT_PROC_OPEN;
    }
    struct dirent *entry;
    while ((entry = readdir(procDir.get())) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        std::string name = entry->d_name;
        if (!StringIsNumber(name)) {
            continue;
        }
        std::string pidPath = std::string(PID_PROC) + FILE_SEPARATOR_CHAR + name;
        if (FindProcForPath(pidPath, path, isDir)) {
            isOccupy = true;
            break;
        }
    }
    LOGI("[L2:MountManager] OpenProcForPath: <<< EXIT SUCCESS <<< isOccupy=%{public}d", isOccupy);
    return E_OK;
}

bool MountManager::FindProcForPath(const std::string &pidPath, const std::string &path, bool isDir)
{
    if (CheckSymlinkForPath(pidPath + FILE_SEPARATOR_CHAR + PID_CWD, path, isDir)) {
        return true;
    }
    if (CheckSymlinkForPath(pidPath + FILE_SEPARATOR_CHAR + PID_EXE, path, isDir)) {
        return true;
    }
    if (CheckSymlinkForPath(pidPath + FILE_SEPARATOR_CHAR + PID_ROOT, path, isDir)) {
        return true;
    }
    std::string fdPath = pidPath + FILE_SEPARATOR_CHAR + PID_FD;
    auto fdDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(fdPath.c_str()), closedir);
    if (!fdDir) {
        LOGE("unable to open %{public}s, err %{public}d", fdPath.c_str(), errno);
        return false;
    }
    struct dirent* fdDirent;
    while ((fdDirent = readdir(fdDir.get())) != nullptr) {
        if (fdDirent->d_type != DT_LNK) {
            continue;
        }
        if (CheckSymlinkForPath(fdPath + FILE_SEPARATOR_CHAR + fdDirent->d_name, path, isDir)) {
            return true;
        }
    }
    return false;
}

bool MountManager::CheckSymlinkForPath(const std::string &fdPath, const std::string &path, bool isDir)
{
    char realPath[PATH_MAX_FOR_LINK];
    int res = readlink(fdPath.c_str(), realPath, sizeof(realPath) - 1);
    if (res < 0) {
        LOGE("readlink failed for path, errno is %{public}d.", errno);
        return false;
    }
    realPath[res] = '\0';
    std::string realPathStr(realPath);
    if (isDir) {
        if (realPathStr.find(UN_REACHABLE) == 0) {
            realPathStr = realPathStr.substr(strlen(UN_REACHABLE)) + FILE_SEPARATOR_CHAR;
        }
        if (realPathStr.find(path) == 0) {
            LOGE("find a fd from link for dir, %{public}s", realPathStr.c_str());
            return true;
        }
    } else {
        if (realPathStr.find(UN_REACHABLE) == 0) {
            realPathStr = realPathStr.substr(strlen(UN_REACHABLE));
        }
        if (realPathStr == path) {
            LOGE("find a fd from link for file, %{public}s", realPathStr.c_str());
            return true;
        }
    }
    return false;
}

void MountManager::FindProcForMulti(const std::string &pidPath, const std::string &path,
    std::set<std::string> &occupyFiles)
{
    CheckSymlinkForMulti(pidPath + FILE_SEPARATOR_CHAR + PID_CWD, path, occupyFiles);
    CheckSymlinkForMulti(pidPath + FILE_SEPARATOR_CHAR + PID_EXE, path, occupyFiles);
    CheckSymlinkForMulti(pidPath + FILE_SEPARATOR_CHAR + PID_ROOT, path, occupyFiles);
    std::string fdPath = pidPath + FILE_SEPARATOR_CHAR + PID_FD;
    auto fdDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(fdPath.c_str()), closedir);
    if (!fdDir) {
        LOGE("unable to open %{public}s, err %{public}d", fdPath.c_str(), errno);
        return;
    }
    struct dirent* fdDirent;
    while ((fdDirent = readdir(fdDir.get())) != nullptr) {
        if (fdDirent->d_type != DT_LNK) {
            continue;
        }
        CheckSymlinkForMulti(fdPath + FILE_SEPARATOR_CHAR + fdDirent->d_name, path, occupyFiles);
    }
}

int32_t MountManager::ClearSecondMountPoint(uint32_t userId, const std::string &bundleName)
{
    LOGI("[L2:MountManager] ClearSecondMountPoint: >>> ENTER <<< userId=%{public}u, bundle=%{public}s",
        userId, bundleName.c_str());
    int32_t ret = IsBundleNeedClear(userId, bundleName);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] ClearSecondMountPoint: <<< EXIT FAILED <<< IsBundleNeedClear failed, ret=%{public}d",
            ret);
        return ret;
    }
    InfoList<MountNodeInfo> sandboxMountNodeList;
    ret = UserPathResolver::GetSandboxMountNodeList(static_cast<int32_t>(userId), sandboxMountNodeList.data);
    if (ret != E_OK) {
        LOGE("[L2:MountManager] ClearSecondMountPoint: <<< EXIT FAILED <<< GetSandboxMountNodeList failed,"
            "ret=%{public}d", ret);
        return ret;
    }
    std::vector<MountNodeInfo> mountNodeInfos = sandboxMountNodeList.data;
    std::string sandboxRootPath = SANDBOX_ROOT_PATH + to_string(userId) + '/' + bundleName;
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    for (const auto &mountNodeInfo : mountNodeInfos) {
        std::string path = sandboxRootPath + mountNodeInfo.dstPath;
        if (path.back() == '/') {
            path.pop_back();
        }
        auto startUmountTime = StorageService::StorageRadar::RecordCurrentTime();
        int32_t res = UMount(path);
        int32_t tmpErrno = errno;
        if (res != E_OK && tmpErrno != ENOENT && tmpErrno != EINVAL) {
            std::string extraData = "path=" + path + ",kernelCode=" + to_string(tmpErrno);
            StorageRadar::ReportUserManager("ClearSecondMountPoint", userId, E_UMOUNT_SANDBOX, extraData);
            LOGE("failed to unmount path is %{public}s, errno is %{public}d, res is %{public}d.", path.c_str(),
                tmpErrno, res);
            return E_UMOUNT_SANDBOX;
        }
        StorageService::StorageRadar::ReportDuration("umount-" + path, startUmountTime, userId);
    }
    RemoveBundleNameFromMap(userId, bundleName);
    auto delay = StorageService::StorageRadar::ReportDuration("ClearSecondMountPoint", startTime, userId);
    LOGI("SD_DURATION: clear second mount point success, delayTime is %{public}s.", delay.c_str());
    LOGI("[L2:MountManager] ClearSecondMountPoint: <<< EXIT SUCCESS <<< userId=%{public}u, bundle=%{public}s",
        userId, bundleName.c_str());
    return E_OK;
}

int32_t MountManager::InitSecondMountBundleName(uint32_t userId)
{
    LOGI("[L2:MountManager] InitSecondMountBundleName: >>> ENTER <<< userId=%{public}u", userId);
    filesystem::path sandboxRootDir(SANDBOX_ROOT_PATH + to_string(userId));
    std::error_code errCode;
    if (!exists(sandboxRootDir, errCode)) {
        std::string extraData = "sandbox dir not exist,kernelCode=" + to_string(errCode.value());
        StorageRadar::ReportUserManager("InitSecondMountBundleName", DEFAULT_USERID, E_UMOUNT_SANDBOX, extraData);
        LOGE("[L2:MountManager] InitSecondMountBundleName: <<< EXIT FAILED <<< root path not exists, errno=%{public}d",
            errCode.value());
        return E_ERR;
    }
    filesystem::directory_iterator bundleNameList(sandboxRootDir);
    std::vector<std::string> bundles;
    std::string bundleSuffix = MOUNT_SUFFIX;
    for (const auto &bundle : bundleNameList) {
        std::string bundleName = bundle.path().filename().generic_string();
        if (bundleName.length() <= bundleSuffix.length()) {
            continue;
        }
        std::string::size_type point = bundleName.rfind(bundleSuffix);
        if (point != bundleName.length() - bundleSuffix.length()) {
            continue;
        }
        bundleName = bundleName.substr(0, point);
        if (std::find(bundles.begin(), bundles.end(), bundleName) != bundles.end()) {
            continue;
        }
        bundles.emplace_back(bundleName);
    }
    secondMountBundleNameMap_[userId] = bundles;
    LOGI("[L2:MountManager] InitSecondMountBundleName: <<< EXIT SUCCESS <<< userId=%{public}u,"
        "bundles.size()=%{public}zu", userId, bundles.size());
    return E_OK;
}

int32_t MountManager::IsBundleNeedClear(uint32_t userId, const std::string &bundleName)
{
    LOGI("[L2:MountManager] IsBundleNeedClear: >>> ENTER <<< userId=%{public}u, bundle=%{public}s",
        userId, bundleName.c_str());
    std::lock_guard<std::mutex> lock(secondMountMutex_);
    if (secondMountBundleNameMap_.find(userId) == secondMountBundleNameMap_.end()) {
        if (InitSecondMountBundleName(userId) != E_OK) {
            LOGE("[L2:MountManager] IsBundleNeedClear: <<< EXIT FAILED <<< InitSecondMountBundleName failed");
            return E_UMOUNT_SANDBOX;
        }
    }
    std::vector<std::string> bundles = secondMountBundleNameMap_.at(userId);
    if (std::find(bundles.begin(), bundles.end(), bundleName) == bundles.end()) {
        LOGE("[L2:MountManager] IsBundleNeedClear: <<< EXIT SUCCESS <<< bundle not need clear");
        return E_NOT_NEED_CLEAR_SECOND_MOUNT_POINT;
    }
    LOGI("[L2:MountManager] IsBundleNeedClear: <<< EXIT SUCCESS <<< bundle needs clear");
    return E_OK;
}

void MountManager::RemoveBundleNameFromMap(uint32_t userId, const std::string &bundleName)
{
    LOGI("[L2:MountManager] RemoveBundleNameFromMap: >>> ENTER <<< userId=%{public}u, bundle=%{public}s",
        userId, bundleName.c_str());
    std::lock_guard<std::mutex> lock(secondMountMutex_);
    if (secondMountBundleNameMap_.find(userId) == secondMountBundleNameMap_.end()) {
        LOGI("[L2:MountManager] RemoveBundleNameFromMap: <<< EXIT SUCCESS <<< userId not found in map");
        return;
    }
    std::vector<std::string> bundles = secondMountBundleNameMap_.at(userId);
    bundles.erase(std::remove(bundles.begin(), bundles.end(), bundleName), bundles.end());
    secondMountBundleNameMap_[userId] = bundles;
    LOGI("[L2:MountManager] RemoveBundleNameFromMap: <<< EXIT SUCCESS <<< userId=%{public}u, bundle=%{public}s",
        userId, bundleName.c_str());
}

void MountManager::ClearSecondMountMap(uint32_t userId)
{
    LOGI("[L2:MountManager] ClearSecondMountMap: >>> ENTER <<< userId=%{public}u", userId);
    std::lock_guard<std::mutex> lock(secondMountMutex_);
    if (secondMountBundleNameMap_.find(userId) != secondMountBundleNameMap_.end()) {
        LOGE("[L2:MountManager] ClearSecondMountMap: clearing second mount map, userId=%{public}d", userId);
        secondMountBundleNameMap_.erase(userId);
    }
    LOGI("[L2:MountManager] ClearSecondMountMap: <<< EXIT SUCCESS <<< userId=%{public}u", userId);
}
} // namespace StorageDaemon
} // namespace OHOS
