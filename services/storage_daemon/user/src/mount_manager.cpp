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

#include "user/mount_manager.h"
#include <cstdlib>
#include <csignal>
#include <dirent.h>
#include <fcntl.h>
#include <set>
#include <sys/mount.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <regex>
#include <filesystem>
#include <cstdio>
#include "hisysevent.h"
#include "crypto/key_manager.h"
#include "utils/storage_radar.h"
#include "ipc/istorage_daemon.h"
#include "parameter.h"
#include "quota/quota_manager.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/mount_argument_utils.h"
#include "utils/string_utils.h"
#include "system_ability_definition.h"
#ifdef DFS_SERVICE
#include "cloud_daemon_manager.h"
#endif
#ifdef USE_LIBRESTORECON
#include "policycoreutils.h"
#endif

namespace OHOS {
namespace StorageDaemon {
using namespace std;
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif
using namespace OHOS::StorageService;
constexpr int32_t UMOUNT_RETRY_TIMES = 3;
constexpr int32_t ONE_KB = 1024;
constexpr int32_t DEFAULT_USERID = 100;
std::shared_ptr<MountManager> MountManager::instance_ = nullptr;

const string SANDBOX_ROOT_PATH = "/mnt/sandbox/";
const string CURRENT_USER_ID_FLAG = "<currentUserId>";
const string PACKAGE_NAME_FLAG = "<bundleName>";
const string MOUNT_POINT_INFO = "/proc/mounts";
const string MOUNT_POINT_TYPE_HMDFS = "hmdfs";
const string MOUNT_POINT_TYPE_HMFS = "hmfs";
const string MOUNT_POINT_TYPE_SHAREFS = "sharefs";
const string EL2_BASE = "/data/storage/el2/base/";
const string MOUNT_SUFFIX = "_locked";
const string APP_EL1_PATH = "/data/app/el1";
const set<string> SANDBOX_EXCLUDE_PATH = {
    "chipset",
    "system",
    "com.ohos.render"
};
const vector<string> CRYPTO_SANDBOX_PATH = {
    "/data/storage/el2/base/",
    "/data/storage/el2/database/",
    "/data/storage/el2/share/",
    "/data/storage/el2/log/",
    "/data/storage/el2/distributedfiles/",
    "/data/storage/el2/cloud/",
    "/data/storage/el3/base/",
    "/data/storage/el3/database/",
    "/data/storage/el4/base/",
    "/data/storage/el4/database/",
    "/data/storage/el5/base/",
    "/data/storage/el5/database/"
};
const vector<string> CRYPTO_SRC_PATH = {
    "/data/app/el2/<currentUserId>/base/<bundleName>/",
    "/data/app/el2/<currentUserId>/database/<bundleName>/",
    "/mnt/share/<currentUserId>/<bundleName>/",
    "/data/app/el2/<currentUserId>/log/<bundleName>/",
    "/mnt/hmdfs/<currentUserId>/account/merge_view/data/<bundleName>/",
    "/mnt/hmdfs/<currentUserId>/cloud/data/<bundleName>/",
    "/data/app/el3/<currentUserId>/base/<bundleName>/",
    "/data/app/el3/<currentUserId>/database/<bundleName>/",
    "/data/app/el4/<currentUserId>/base/<bundleName>/",
    "/data/app/el4/<currentUserId>/database/<bundleName>/",
    "/data/app/el5/<currentUserId>/base/<bundleName>/",
    "/data/app/el5/<currentUserId>/database/<bundleName>/"
};

const vector<string> APPDATA_DST_PATH = {
    "/mnt/user/<currentUserId>/nosharefs/appdata/el1/base/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/base/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/cloud/",
    "/mnt/user/<currentUserId>/nosharefs/appdata/el2/distributedfiles/"
};

const vector<string> APPDATA_SRC_PATH = {
    "/data/app/el1/<currentUserId>/base/",
    "/data/app/el2/<currentUserId>/base/",
    "/mnt/hmdfs/<currentUserId>/cloud/data/",
    "/mnt/hmdfs/<currentUserId>/account/merge_view/data/"
};

const std::string HMDFS_SYS_CAP = "const.distributed_file_property.enabled";
const int32_t HMDFS_VAL_LEN = 6;
const int32_t HMDFS_TRUE_LEN = 5;
const string SHARE_PATH = "/data/service/el1/public/storage_daemon/share/public";
static constexpr int MODE_0711 = 0711;
static constexpr int MODE_0771 = 0771;
static constexpr int MODE_02771 = 02771;
MountManager::MountManager() : hmdfsDirVec_(InitHmdfsDirVec()), virtualDir_(InitVirtualDir()),
    systemServiceDir_(InitSystemServiceDir()), fileManagerDir_(InitFileManagerDir()), appdataDir_(InitAppdataDir())
{
}

std::shared_ptr<MountManager> MountManager::GetInstance()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() { instance_ = std::make_shared<MountManager>(); });

    return instance_;
}

std::vector<DirInfo> MountManager::InitHmdfsDirVec()
{
    return {{"/data/service/el2/%d/share", MODE_0711, OID_SYSTEM, OID_SYSTEM},
            {"/data/service/el2/%d/hmdfs", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/account", MODE_0711, OID_SYSTEM, OID_SYSTEM},
            {"/data/service/el2/%d/hmdfs/account/files", MODE_02771, OID_USER_DATA_RW, OID_USER_DATA_RW},
            {"/data/service/el2/%d/hmdfs/account/data", MODE_0711, OID_SYSTEM, OID_SYSTEM},
            {"/data/service/el2/%d/hmdfs/non_account", MODE_0711, OID_SYSTEM, OID_SYSTEM},
            {"/data/service/el2/%d/hmdfs/non_account/files", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
            {"/data/service/el2/%d/hmdfs/non_account/data", MODE_0711, OID_SYSTEM, OID_SYSTEM},
            {"/data/service/el2/%d/hmdfs/cloud", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cloud/data", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cache", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cloudfile_manager", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cache/account_cache", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cache/non_account_cache", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/cache/cloud_cache", MODE_0711, OID_DFS, OID_DFS},
            {"/data/service/el2/%d/hmdfs/account/services", MODE_0771, OID_DFS_SHARE, OID_DFS_SHARE}};
}

std::vector<DirInfo> MountManager::InitVirtualDir()
{
    return {{"/storage/media/%d", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
            {"/storage/media/%d/local", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
            {"/storage/cloud", MODE_0711, OID_ROOT, OID_ROOT},
            {"/storage/cloud/%d", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
            {"/mnt/share/", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/share/%d/", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/data/%d/", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/data/%d/cloud", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/data/%d/cloud_fuse", MODE_0711, OID_DFS, OID_DFS},
            {"/mnt/data/%d/hmdfs", MODE_0711, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/mnt/hmdfs/", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/hmdfs/%d/", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/hmdfs/%d/cloud", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/hmdfs/%d/account", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/hmdfs/%d/non_account", MODE_0711, OID_ROOT, OID_ROOT}};
}

std::vector<DirInfo> MountManager::InitSystemServiceDir()
{
    return {{"/data/service/el2/%d/tee", MODE_0711, OID_TEE, OID_TEE},
            {"/data/service/el2/%d/cloud_backup_service", MODE_0711, OID_CLOUD_BACK, OID_CLOUD_BACK},
            {"/data/service/el2/%d/deviceauth", MODE_0711, OID_DEVICE_AUTH, OID_DEVICE_AUTH},
            {"/data/service/el3/%d/device_standby", MODE_0711, OID_RSS, OID_RSS},
            {"/data/service/el2/%d/hwid_service", MODE_0711, OID_HWID, OID_HWID},
            {"/data/service/el2/%d/healthsport", MODE_0711, OID_HEALTH_SPORT, OID_HEALTH_SPORT},
            {"/data/service/el2/%d/huks_service", MODE_0711, OID_HUKS, OID_HUKS},
            {"/data/service/el2/%d/parentcontrol", MODE_0711, OID_PARENT_CONTROL, OID_PARENT_CONTROL},
            {"/data/service/el4/%d/huks_service", MODE_0711, OID_HUKS, OID_HUKS},
            {"/data/service/el2/%d/asset_service", MODE_0711, OID_ASSET, OID_ASSET},
            {"/data/service/el2/%d/account", MODE_0711, OID_ACCOUNT, OID_ACCOUNT},
            {"/data/service/el2/%d/dlp_credential_service", MODE_0711, OID_DLP_CREDENTIAL, OID_DLP_CREDENTIAL},
            {"/data/service/el2/%d/xpower", MODE_0711, OID_HIVIEW, OID_HIVIEW},
            {"/data/service/el2/%d/iShare", MODE_0711, OID_COLLABORATION_FWK, OID_COLLABORATION_FWK},
            {"/data/service/el2/%d/av_session", MODE_0711, OID_AV_SESSION, OID_AV_SESSION},
            {"/data/service/el2/%d/file_transfer_service", MODE_0711, 7017, 7017},
            {"/data/service/el2/%d/print_service", MODE_0711, OID_PRINT, OID_PRINT}};
}

std::vector<DirInfo> MountManager::InitFileManagerDir()
{
    return {{"/data/service/el2/%d/hmdfs/account/files/Docs", MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/data/service/el2/%d/hmdfs/account/files/Docs/Documents",
                                                              MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/data/service/el2/%d/hmdfs/account/files/Docs/Download",
                                                              MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/data/service/el2/%d/hmdfs/account/files/Docs/Desktop",
                                                              MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/data/service/el2/%d/hmdfs/account/files/Docs/.Trash",
                                                              MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
            {"/data/service/el2/%d/hmdfs/account/files/.Recent", MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER}};
}

std::vector<DirInfo> MountManager::InitAppdataDir()
{
    return {{"/mnt/user", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/docs", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/docs/currentUser", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el1", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el1/base", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el2", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el2/base", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el2/cloud", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/nosharefs/appdata/el2/distributedfiles", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/sharefs", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/sharefs/docs", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/sharefs/docs/currentUser", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/currentUser", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/currentUser/filemgr", MODE_0711, OID_ROOT, OID_ROOT},
            {"/mnt/user/%d/currentUser/other", MODE_0711, OID_ROOT, OID_ROOT}};
}

int32_t MountManager::HmdfsTwiceMount(int32_t userId, const std::string &relativePath)
{
    int32_t ret = HmdfsMount(userId, relativePath);
    // bind mount
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    std::string dst = hmdfsMntArgs.GetCommFullPath();
    if (IsPathMounted(dst)) {
        LOGI("path has mounted, %{public}s", dst.c_str());
    } else {
        ret += Mount(hmdfsMntArgs.GetFullDst() + "/device_view/", dst, nullptr, MS_BIND, nullptr);
        if (ret != 0 && errno != EEXIST && errno != EBUSY) {
            LOGE("failed to bind mount device_view, err %{public}d", errno);
            return E_MOUNT;
        }
    }
    dst = hmdfsMntArgs.GetCloudFullPath();
    if (IsPathMounted(dst)) {
        LOGI("path has mounted, %{public}s", dst.c_str());
    } else {
        ret += Mount(hmdfsMntArgs.GetFullDst() + "/cloud_merge_view/", dst, nullptr, MS_BIND, nullptr);
        if (ret != 0 && errno != EEXIST && errno != EBUSY) {
            LOGE("failed to bind mount cloud_merge_view, err %{public}d", errno);
            return E_MOUNT;
        }
    }
    dst = hmdfsMntArgs.GetCloudDocsPath();
    if (IsPathMounted(dst)) {
        LOGI("path has mounted, %{public}s", dst.c_str());
    } else {
        ret += Mount(hmdfsMntArgs.GetLocalDocsPath(), dst, nullptr, MS_BIND, nullptr);
        if (ret != 0 && errno != EEXIST && errno != EBUSY) {
            LOGE("failed to bind mount docs, err %{public}d", errno);
        }
    }
    return E_OK;
}

int32_t MountManager::SharefsMount(int32_t userId)
{
    Utils::MountArgument sharefsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    std::string dst = sharefsMntArgs.GetShareDst();
    if (IsPathMounted(dst)) {
        LOGI("path has mounted, %{public}s", dst.c_str());
    } else {
        int ret = Mount(sharefsMntArgs.GetShareSrc(), dst, "sharefs", sharefsMntArgs.GetFlags(),
                        sharefsMntArgs.GetUserIdPara().c_str());
        if (ret != 0 && errno != EEXIST && errno != EBUSY) {
            LOGE("failed to mount sharefs, err %{public}d", errno);
            return E_MOUNT;
        }
    }
    return E_OK;
}

int32_t MountManager::HmSharefsMount(int32_t userId, std::string &srcPath, std::string &dstPath)
{
    if (IsDir(srcPath)) {
        LOGE("srcPath not exist, %{public}s", srcPath.c_str());
        return ENOENT;
    }
    if (IsDir(dstPath)) {
        LOGE("srcPath not exist, %{public}s", dstPath.c_str());
        return ENOENT;
    }
    if (IsPathMounted(dstPath)) {
        LOGI("path has mounted, %{public}s", dstPath.c_str());
        return E_OK;
    }
    Utils::MountArgument sharefsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    int ret = Mount(srcPath, dstPath, "sharefs", sharefsMntArgs.GetFlags(),
                    sharefsMntArgs.GetHmUserIdPara().c_str());
    if (ret != 0) {
        LOGE("failed to mount hmSharefs, err %{public}d", errno);
        return E_MOUNT;
    }
    return E_OK;
}

int32_t MountManager::HmdfsMount(int32_t userId, std::string relativePath, bool mountCloudDisk)
{
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    std::string mountSrcPath = hmdfsMntArgs.GetFullSrc();
    if (mountCloudDisk) {
        hmdfsMntArgs.enableCloudDisk_ = true;
        hmdfsMntArgs.useCloudDir_ = false;
        hmdfsMntArgs.enableMergeView_ = false;
        mountSrcPath = hmdfsMntArgs.GetFullCloud();
    }

    std::string dst = hmdfsMntArgs.GetFullDst();
    if (IsPathMounted(dst)) {
        LOGI("path has mounted, %{public}s", dst.c_str());
        return E_OK;
    }
    int ret = Mount(mountSrcPath, dst, "hmdfs", hmdfsMntArgs.GetFlags(), hmdfsMntArgs.OptionsToString().c_str());
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to mount hmdfs, err %{public}d", errno);
        return E_MOUNT;
    }

    ret = chown(hmdfsMntArgs.GetCtrlPath().c_str(), OID_DFS, OID_SYSTEM);
    if (ret != 0) {
        LOGE("failed to chown hmdfs sysfs node, err %{public}d", errno);
    }
    return E_OK;
}

int32_t MountManager::FindAndKillProcess(int32_t userId, std::list<std::string> &mountFailList)
{
    if (userId <= 0) {
        return E_OK;
    }
    LOGI("FindAndKillProcess start, userId is %{public}d", userId);
    auto procDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir("/proc"), closedir);
    if (!procDir) {
        LOGE("failed to open dir proc, err %{public}d", errno);
        return -errno;
    }
    std::vector<ProcessInfo> processInfos;
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
            LOGE("failed to get process info, pid is %{public}s.", name.c_str());
            continue;
        }
        if (info.name == "(storage_daemon)") {
            continue;
        }
        Utils::MountArgument argument(Utils::MountArgumentDescriptors::Alpha(userId, ""));
        const string &prefix = argument.GetMountPointPrefix();
        std::string pidPath = "/proc/" + name;
        if (PidUsingFlag(pidPath, prefix, mountFailList)) {
            LOGE("find a link pid is %{public}d, processName is %{public}s.", info.pid, info.name.c_str());
            processInfos.push_back(info);
        }
    }
    LOGI("FindAndKillProcess end, total find %{public}d", static_cast<int>(processInfos.size()));
    KillProcess(processInfos);
    UmountFailRadar(processInfos);
    return E_OK;
}

void MountManager::UmountFailRadar(std::vector<ProcessInfo> &processInfos)
{
    if (processInfos.empty()) {
        return;
    }
    std::string ss;
    int32_t ret = E_UMOUNT;
    for (const auto &item:processInfos) {
        ss += item.name + ",";
    }
    if (StorageService::StorageRadar::GetInstance().RecordKillProcessResult(ss, ret)) {
        LOGI("StorageRadar record FindAndKillProcess result success, ret = %{public}d, process is %{public}s",
             ret, ss.c_str());
    }
}

bool MountManager::PidUsingFlag(std::string &pidPath, const std::string &prefix, std::list<std::string> &mountFailList)
{
    bool found = false;
    found |= CheckMaps(pidPath + "/maps", prefix, mountFailList);
    found |= CheckSymlink(pidPath + "/cwd", prefix, mountFailList);
    found |= CheckSymlink(pidPath + "/root", prefix, mountFailList);
    found |= CheckSymlink(pidPath + "/exe", prefix, mountFailList);

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
            found |= CheckSymlink(fdPath + "/" +fdDirent->d_name, prefix, mountFailList);
        }
    }
    return found;
}

void MountManager::KillProcess(std::vector<ProcessInfo> &processInfo)
{
    if (processInfo.empty()) {
        return;
    }
    for (const auto &item: processInfo) {
        LOGI("KILL PID %{public}d", item.pid);
        kill(item.pid, SIGKILL);
    }
}

bool MountManager::GetProcessInfo(const std::string &filename, ProcessInfo &info)
{
    if (filename.empty()) {
        return false;
    }
    LOGE("GetProcessInfo path is %{public}s", filename.c_str());
    std::ifstream inputStream(filename.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open %{public}s, err %{public}d", filename.c_str(), errno);
        return false;
    }
    std::string line;
    std::getline(inputStream, line);
    if (line.empty()) {
        LOGE("line is empty");
        inputStream.close();
        return false;
    }
    std::stringstream ss(line);
    std::string pid;
    ss >> pid;
    std::string processName;
    ss >> processName;
    info.pid = std::stoi(pid);
    info.name = processName;
    LOGE("GetProcessInfo pid is %{public}s and name is %{public}s", pid.c_str(), processName.c_str());
    inputStream.close();
    return true;
}

bool MountManager::CheckMaps(const std::string &path, const std::string &prefix, std::list<std::string> &mountFailList)
{
    if (path.empty()) {
        return false;
    }
    std::ifstream inputStream(path.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open %{public}s, err %{public}d", path.c_str(), errno);
        return false;
    }
    LOGE("CheckMaps path is %{public}s", path.c_str());
    std::string tmpLine;
    while (std::getline(inputStream, tmpLine)) {
        std::string::size_type pos = tmpLine.find("/");
        if (pos == std::string::npos) {
            continue;
        }
        tmpLine = tmpLine.substr(pos);
        if (tmpLine.find(prefix) == 0) {
            LOGE("find a fd %{public}s", tmpLine.c_str());
            inputStream.close();
            return true;
        }
        for (const auto &item: mountFailList) {
            if (tmpLine.find(item) == 0) {
                LOGE("find a fd %{public}s", tmpLine.c_str());
                inputStream.close();
                return true;
            }
        }
    }
    inputStream.close();
    return false;
}

bool MountManager::CheckSymlink(const std::string &path,
                                const std::string &prefix, std::list<std::string> &mountFailList)
{
    if (path.empty()) {
        return false;
    }
    char realPath[ONE_KB];
    int res = readlink(path.c_str(), realPath, sizeof(realPath) - 1);
    if (res < 0 || res >= ONE_KB) {
        return false;
    }
    realPath[res] = '\0';
    std::string realPathStr(realPath);
    if (realPathStr.find(prefix) == 0) {
        LOGE("find a fd %{public}s", realPathStr.c_str());
        return true;
    }
    for (const auto &item: mountFailList) {
        if (realPathStr.find(item) == 0) {
            LOGE("find a fd %{public}s", realPathStr.c_str());
            return true;
        }
    }
    return false;
}

int32_t MountManager::CloudMount(int32_t userId, const string& path)
{
    LOGI("cloud mount start");
#ifdef DFS_SERVICE
    string opt;
    int ret;
    if (!cloudReady_) {
        LOGI("Cloud Service has not started");
        return E_MOUNT;
    }

    FILE *f = fopen("/dev/fuse", "r+");
    if (f == nullptr) {
        LOGE("open /dev/fuse fail");
        return E_MOUNT;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("open /dev/fuse fail");
        return E_MOUNT;
    }
    LOGI("open fuse end");
    opt = StringPrintf("fd=%i,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=\"u:object_r:hmdfs:s0\","
        "fscontext=u:object_r:hmdfs:s0",
        fd);
    LOGI("start to mount fuse");
    ret = Mount("/dev/fuse", path.c_str(), "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("failed to mount fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        (void)fclose(f);
        return ret;
    }
    LOGI("start cloud daemon fuse");
    ret = CloudDaemonManager::GetInstance().StartFuse(userId, fd, path);
    if (ret) {
        LOGE("failed to connect fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        UMount(path.c_str());
    }
    LOGI("mount %{public}s success", path.c_str());
    (void)fclose(f);
    return ret;
#else
    return E_OK;
#endif
}

int32_t MountManager::CloudTwiceMount(int32_t userId)
{
    LOGI("mount cloud twice start");
    int32_t ret = E_OK;
#ifdef DFS_SERVICE
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    string cloudPath = cloudMntArgs.GetFullCloud();
    string cloudMediaPath = cloudMntArgs.GetFullMediaCloud();
    int32_t mountRet = E_OK;
    if (IsPathMounted(cloudPath)) {
        LOGI("path has mounted, %{public}s", cloudPath.c_str());
    } else {
        mountRet = CloudMount(userId, cloudPath);
        if (mountRet != E_OK) {
            ret = mountRet;
        }
    }
    if (IsPathMounted(cloudMediaPath)) {
        LOGI("path has mounted, %{public}s", cloudMediaPath.c_str());
    } else {
        mountRet = CloudMount(userId, cloudMediaPath);
        if (mountRet != E_OK) {
            ret = mountRet;
        }
    }
    return ret;
#else
    return ret;
#endif
}

int32_t MountManager::HmdfsMount(int32_t userId)
{
    int32_t err = PrepareHmdfsDirs(userId);
    if (err != E_OK) {
        LOGE("Prepare fileManager dir error");
    }

    int32_t ret = HmdfsTwiceMount(userId, "account");

    ret += HmdfsMount(userId, "non_account");
    if (ret != E_OK) {
        return E_MOUNT;
    }

    LOGI("ready to mount cloud");
    mountMutex_.lock();
    ret = CloudTwiceMount(userId);
    if (ret == E_OK) {
        fuseMountedUsers_.push_back(userId);
    } else {
        fuseToMountUsers_.push_back(userId);
    }
    mountMutex_.unlock();

    ret = HmdfsMount(userId, "cloud", true);
    if (ret != E_OK) {
        LOGE("mount cloud to hmdfs failed!");
    }

    return E_OK;
}

static void ParseSandboxPath(string &path, const string &userId, const string &bundleName)
{
    size_t pos = path.find(CURRENT_USER_ID_FLAG);
    if (pos != string::npos) {
        path = path.replace(pos, CURRENT_USER_ID_FLAG.length(), userId);
    }

    pos = path.find(PACKAGE_NAME_FLAG);
    if (pos != string::npos) {
        path = path.replace(pos, PACKAGE_NAME_FLAG.length(), bundleName);
    }
}

bool MountManager::CheckPathValid(const std::string &bundleNameStr, uint32_t userId)
{
    string completePath =
        SANDBOX_ROOT_PATH + to_string(userId) + "/" + bundleNameStr + EL2_BASE;
    if (!IsDir(completePath)) {
        LOGE("Invalid directory path: %{public}s", completePath.c_str());
        return false;
    }

    if (!std::filesystem::is_empty(completePath)) {
        LOGE("The directory has been mounted, path is %{public}s", completePath.c_str());
        return false;
    }
    return true;
}

int32_t MountManager::MountCryptoPathAgain(uint32_t userId)
{
    filesystem::path rootDir(SANDBOX_ROOT_PATH + to_string(userId));
    std::error_code errCode;
    if (!exists(rootDir, errCode)) {
        LOGE("root path not exists, rootDir is %{public}s", SANDBOX_ROOT_PATH.c_str());
        return -ENOENT;
    }

    int32_t ret = 0;
    filesystem::directory_iterator bundleNameList(rootDir);
    for (const auto &bundleName : bundleNameList) {
        if (SANDBOX_EXCLUDE_PATH.find(bundleName.path().filename()) != SANDBOX_EXCLUDE_PATH.end()) {
            continue;
        }
        std::string bundleNameStr = bundleName.path().filename().generic_string();
        int32_t point = bundleNameStr.find(MOUNT_SUFFIX);
        if (point == -1) {
            LOGI("bundleName do not need to mount: %{public}s", bundleNameStr.c_str());
            continue;
        }
        bundleNameStr = bundleNameStr.substr(0, point);
        if (!CheckPathValid(bundleNameStr, userId)) {
            continue;
        }
        vector<string> dstPaths = CRYPTO_SANDBOX_PATH;
        vector<string> srcPaths = CRYPTO_SRC_PATH;
        MountSandboxPath(srcPaths, dstPaths, bundleNameStr, to_string(userId));
    }
    LOGI("mount crypto path success, userId is %{public}d", userId);
    return ret;
}

void MountManager::MountSandboxPath(const std::vector<std::string> &srcPaths, const std::vector<std::string> &dstPaths,
    const std::string &bundleName, const std::string &userId)
{
    int srcCnt = static_cast<int>(srcPaths.size());
    int dstCnt = static_cast<int>(dstPaths.size());
    if (srcCnt == 0 || dstCnt == 0 || srcCnt != dstCnt) {
        LOGE("invalid params, srcPaths total %{public}d, dstPaths total %{public}d", srcCnt, dstCnt);
        return;
    }
    for (int i = 0; i < dstCnt; i++) {
        std::string dstPath = SANDBOX_ROOT_PATH;
        dstPath = dstPath.append(userId).append("/").append(bundleName).append(dstPaths[i]);
        string srcPath = srcPaths[i];
        ParseSandboxPath(srcPath, userId, bundleName);
        if (!IsDir(dstPath)) {
            LOGE("dstPath is not a dir: %{public}s", dstPath.c_str());
            continue;
        }
        if (!IsDir(srcPath)) {
            LOGE("srcPath is not a dir: %{public}s", srcPath.c_str());
            continue;
        }
        LOGD("mount crypto path, srcPath is %{public}s, dstPath is %{public}s", srcPath.c_str(), dstPath.c_str());
        int32_t ret = mount(srcPath.c_str(), dstPath.c_str(), nullptr, MS_BIND | MS_REC, nullptr);
        if (ret != E_OK && errno == EBUSY) {
            ret = mount(srcPath.c_str(), dstPath.c_str(), nullptr, MS_BIND | MS_REC, nullptr);
            LOGI("mount again dstPath is %{public}s, ret is %{public}d.", dstPath.c_str(), ret);
        }
        if (ret != 0) {
            LOGE("mount bind failed, srcPath is %{public}s dstPath is %{public}s errno is %{public}d",
                 srcPath.c_str(), dstPath.c_str(), errno);
            continue;
        }
        LOGI("bind mount path, srcPath is %{public}s, dstPath is %{public}s", srcPath.c_str(), dstPath.c_str());
        ret = mount(nullptr, dstPath.c_str(), nullptr, MS_SHARED, nullptr);
        if (ret != 0) {
            LOGE("mount to share failed, srcPath is %{public}s dstPath is %{public}s errno is %{public}d",
                 srcPath.c_str(), dstPath.c_str(), errno);
            continue;
        }
        LOGI("shared mount success, dstPath is %{public}s", dstPath.c_str());
    }
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
    if (type == MOUNT_POINT_TYPE_HMFS) {
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
    std::ifstream inputStream(MOUNT_POINT_INFO.c_str(), std::ios::in);
    if (!inputStream.is_open()) {
        LOGE("unable to open /proc/mounts, errno is %{public}d", errno);
        return -errno;
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
    return E_OK;
}

int32_t MountManager::UMountAllPath(int32_t userId, std::list<std::string> &mountFailList)
{
    std::map<std::string, std::list<std::string>> mountMap;
    int32_t res = FindMountPointsToMap(mountMap, userId);
    if (res != E_OK) {
        return res;
    }
    int32_t result = 0;
    std::list<std::string> list = mountMap[MOUNT_POINT_TYPE_SHAREFS];
    int total = static_cast<int>(list.size());
    LOGI("unmount sharefs path start, total %{public}d.", total);
    res = UMountByList(list, mountFailList);
    if (res != E_OK) {
        result = res;
    }

    list = mountMap[MOUNT_POINT_TYPE_HMFS];
    total = static_cast<int>(list.size());
    LOGI("unmount hmfs path start, total %{public}d.", total);
    res = UMountByList(list, mountFailList);
    if (res != E_OK) {
        result = res;
    }
    UmountMntUserTmpfs(userId);

    list = mountMap[MOUNT_POINT_TYPE_HMDFS];
    total = static_cast<int>(list.size());
    LOGI("unmount hmdfs path start, total %{public}d.", total);
    res = UMountByList(list, mountFailList);
    if (res != E_OK) {
        result = res;
    }

    if (result != E_OK) {
        for (const auto &item: mountFailList) {
            res = UMount2(item.c_str(), MNT_DETACH);
            if (res != E_OK && errno == EBUSY) {
                LOGE("failed to unmount with detach, path %{public}s, errno %{public}d.", item.c_str(), errno);
            }
        }
        return result;
    }
    LOGI("UMountAllPath success");
    return E_OK;
}

int32_t MountManager::UMountByList(std::list<std::string> &list, std::list<std::string> &mountFailList)
{
    if (list.empty()) {
        return E_OK;
    }
    int32_t result = E_OK;
    for (const std::string &path: list) {
        LOGD("umount path %{public}s.", path.c_str());
        int32_t res = UMount(path);
        if (res != E_OK) {
            LOGE("failed to unmount path %{public}s, errno %{public}d.", path.c_str(), errno);
            result = errno;
            mountFailList.push_back(path);
        }
    }
    LOGI("UMountByList result is %{public}d.", result);
    return result;
}

void MountManager::MountCloudForUsers(void)
{
    for (auto it = fuseToMountUsers_.begin(); it != fuseToMountUsers_.end();) {
        int32_t res = CloudTwiceMount(*it);
        if (res == E_OK) {
            fuseMountedUsers_.push_back(*it);
            it = fuseToMountUsers_.erase(it);
        } else {
            it++;
        }
    }
}

void MountManager::UMountCloudForUsers(void)
{
    for (auto it = fuseMountedUsers_.begin(); it != fuseMountedUsers_.end();) {
        int32_t res = CloudUMount(*it);
        if (res == E_OK) {
            fuseToMountUsers_.push_back(*it);
            it = fuseMountedUsers_.erase(it);
        } else {
            it++;
        }
    }
}

void MountManager::SetCloudState(bool active)
{
    LOGI("set cloud state start, active is %{public}d", active);
    mountMutex_.lock();
    cloudReady_ = active;
    if (cloudReady_) {
        MountCloudForUsers();
    } else {
        UMountCloudForUsers();
    }
    mountMutex_.unlock();
    LOGI("set cloud state end");
}

int32_t MountManager::HmdfsUMount(int32_t userId, std::string relativePath)
{
    Utils::MountArgument hmdfsAuthMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    int32_t ret = UMount2(hmdfsAuthMntArgs.GetFullDst().c_str(), MNT_DETACH);
    if (ret != E_OK) {
        LOGE("umount auth hmdfs, errno %{public}d, auth hmdfs dst %{public}s", errno,
             hmdfsAuthMntArgs.GetFullDst().c_str());
        return E_UMOUNT;
    }
    return E_OK;
}

int32_t MountManager::CloudUMount(int32_t userId)
{
#ifdef DFS_SERVICE
    int32_t err = E_OK;
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string path = cloudMntArgs.GetFullCloud();
    const string mediaCloudPath = cloudMntArgs.GetFullMediaCloud();

    HmdfsUMount(userId, "cloud");

    err = UMount2(path, MNT_DETACH);
    if (err != E_OK) {
        LOGE("fuse umount2 failed, errno %{public}d, fuse dst %{public}s", errno, path.c_str());
        return E_UMOUNT;
    }

    err = UMount2(mediaCloudPath, MNT_DETACH);
    if (err != E_OK) {
        LOGE("fuse umount2 failed, errno %{public}d, fuse dst %{public}s", errno, mediaCloudPath.c_str());
        return E_UMOUNT;
    }
    LOGI("umount2 media cloud path:%{public}s  cloud path:%{public}s success", mediaCloudPath.c_str(), path.c_str());
    return E_OK;
#else
    return E_OK;
#endif
}

bool MountManager::SupportHmdfs()
{
    char hmdfsEnable[HMDFS_VAL_LEN + 1] = {"false"};
    int ret = GetParameter(HMDFS_SYS_CAP.c_str(), "", hmdfsEnable, HMDFS_VAL_LEN);
    LOGI("GetParameter hmdfsEnable %{public}s, ret %{public}d", hmdfsEnable, ret);
    if (strncmp(hmdfsEnable, "true", HMDFS_TRUE_LEN) == 0) {
        return true;
    }
    return false;
}

int32_t MountManager::LocalMount(int32_t userId)
{
    Utils::MountArgument LocalMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, "account"));
    if (Mount(LocalMntArgs.GetFullSrc(), LocalMntArgs.GetCommFullPath() + "local/",
              nullptr, MS_BIND, nullptr)) {
        LOGE("failed to bind mount, err %{public}d", errno);
        return E_MOUNT;
    }
    if (Mount(LocalMntArgs.GetFullSrc(), LocalMntArgs.GetCloudFullPath(),
              nullptr, MS_BIND, nullptr)) {
        LOGE("failed to bind mount, err %{public}d", errno);
        return E_MOUNT;
    }
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
    bool isCeEncrypt = false;
    int ret = KeyManager::GetInstance()->GetFileEncryptStatus(userId, isCeEncrypt);
    if (ret != E_OK || isCeEncrypt) {
        LOGE("User %{public}d de has not decrypt.", userId);
        return E_KEY_NOT_ACTIVED;
    }
    // The Documnets and Download directories are managed by the File access framework,
    // and the UID GID is changed to filemanager
    std::thread thread([userId]() { ClearRedundantResources(userId); });
    thread.detach();
    PrepareFileManagerDir(userId);
    if (CreateVirtualDirs(userId) != E_OK) {
        LOGE("create hmdfs virtual dir error");
        return E_PREPARE_DIR;
    }

    if (!SupportHmdfs()) {
        ret = LocalMount(userId);
    } else {
        ret = HmdfsMount(userId);
    }

    if (ret != E_OK) {
        LOGE("hmdfs mount error");
        return ret;
    }

    ret = SharefsMount(userId);
    if (ret != E_OK) {
        LOGE("sharefs mount error");
    }
    MountAppdataAndSharefs(userId);
    SetFafQuotaProId(userId);
    if (CreateSystemServiceDirs(userId) != E_OK) {
        LOGE("create system service dir error");
        return E_PREPARE_DIR;
    }
    LOGI("MountByUser success, userId is %{public}d.", userId);
    return E_OK;
}

static uid_t GetFileManagerUid(uid_t uid, int32_t userId)
{
    return USER_ID_BASE * userId + uid;
}

void MountManager::PrepareFileManagerDir(int32_t userId)
{
    std::string filesPath = StringPrintf("/data/service/el2/%d/hmdfs/account/files/", userId);
    // move file manager dir
    MoveFileManagerData(filesPath);
    for (const DirInfo &dir : fileManagerDir_) {
        uid_t dirUid = GetFileManagerUid(dir.uid, userId);
        std::string path = StringPrintf(dir.path.c_str(), userId);
        int ret = IsSameGidUid(path, dirUid, dir.gid);
        LOGD("prepareDir %{public}s ret %{public}d, dirUid: %{public}d", path.c_str(), ret, dirUid);
        // Dir exist and same uid, gid
        if (ret == E_OK) {
            continue;
        }
        // system error
        if (ret == E_SYS_ERR) {
            LOGE("system err %{public}s ", path.c_str());
            continue;
        }
        // Dir exist and different uid, gid
        if (ret == E_DIFF_UID_GID) {
            ChownRecursion(path, dirUid, OID_FILE_MANAGER);
            continue;
        }
        // Dir not exist
        if (ret == E_NON_EXIST && !PrepareDir(path, dir.mode, dirUid, dir.gid)) {
            LOGE("failed to prepareDir %{public}s ", path.c_str());
        }
    }
}

int32_t MountManager::LocalUMount(int32_t userId)
{
    Utils::MountArgument LocalMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, "account"));
    int err = UMount(LocalMntArgs.GetCommFullPath() + "local/");
    if (err != E_OK) {
        LOGE("failed to un bind mount, errno %{public}d, ComDataDir dst %{public}s", errno,
             LocalMntArgs.GetCommFullPath().c_str());
    }
    err = UMount(LocalMntArgs.GetCloudFullPath());
    if (err != E_OK) {
        LOGE("failed to un bind mount, errno %{public}d, CloudDataDir dst %{public}s", errno,
             LocalMntArgs.GetCloudFullPath().c_str());
    }
    return err;
}

int32_t MountManager::UmountByUser(int32_t userId)
{
    LOGI("umount hmdfs mount point start.");
    int32_t err = E_OK;
    if (!SupportHmdfs()) {
        err = LocalUMount(userId);
        if (err != E_OK) {
            LOGE("failed to umount locally, err is %{public}d", err);
        }
    } else {
        std::list<std::string> mountFailList;
        err = UMountAllPath(userId, mountFailList);
        if (err != E_OK) {
            LOGE("failed to umount hmdfs mount point, err is %{public}d", err);
            FindAndKillProcess(userId, mountFailList);
        }
    }

    LOGI("umount cloud mount point start.");
    int32_t count = 0;
    while (count < UMOUNT_RETRY_TIMES) {
        err = CloudUMount(userId);
        if (err == E_OK) {
            break;
        } else if (errno == EBUSY) {
            count++;
            continue;
        }
        LOGE("failed to umount cloud mount point, err %{public}d", err);
        return E_UMOUNT;
    }
    return E_OK;
}

int32_t MountManager::PrepareHmdfsDirs(int32_t userId)
{
    for (const DirInfo &dir : hmdfsDirVec_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }
    return E_OK;
}

int32_t MountManager::PrepareFileManagerDirs(int32_t userId)
{
    for (const DirInfo &dir : fileManagerDir_) {
        uid_t dirUid = GetFileManagerUid(dir.uid, userId);
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dirUid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t MountManager::CreateVirtualDirs(int32_t userId)
{
    for (const DirInfo &dir : virtualDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        if (CloudDirFlag(path) && IsDir(path)) {
            continue;
        }
        if (!PrepareDir(path, dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }
    return E_OK;
}

int32_t MountManager::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("MountManager::MountDfsDocs start.");
    std::string dstPath = StringPrintf("/mnt/data/%d/hmdfs/%s/", userId, deviceId.c_str());
    if (!PrepareDir(dstPath, MODE_0711, OID_FILE_MANAGER, OID_FILE_MANAGER)) {
        return E_PREPARE_DIR;
    }

    std::regex pathRegex("^[a-zA-Z0-9_/]+$");
    if (relativePath.empty() || relativePath.length() > PATH_MAX || !std::regex_match(relativePath, pathRegex)) {
        LOGE("[MountDfsDocs]invalid relativePath");
        return E_MOUNT;
    }

    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    std::string srcPath = hmdfsMntArgs.GetFullDst() + "/device_view/" + networkId + "/files/Docs/";
    int32_t ret = Mount(srcPath, dstPath, nullptr, MS_BIND, nullptr);
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("MountDfsDocs mount bind failed, srcPath is %{public}s dstPath is %{public}s errno is %{public}d",
            srcPath.c_str(), dstPath.c_str(), errno);
        return E_MOUNT;
    }
    return E_OK;
}

int32_t MountManager::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    LOGI("MountManager::UMountDfsDocs start.");

    std::regex pathRegex("^[a-zA-Z0-9_/]+$");
    if (relativePath.empty() || relativePath.length() > PATH_MAX || !std::regex_match(relativePath, pathRegex)) {
        LOGE("[UMountDfsDocs]invalid relativePath");
        return E_UMOUNT;
    }

    std::string dstPath = StringPrintf("/mnt/data/%d/hmdfs/%s", userId, deviceId.c_str());
    sync();
    int32_t ret = UMount2(dstPath, MNT_FORCE);
    if (ret != E_OK) {
        LOGE("UMountDfsDocs unmount bind failed, srcPath is %{public}s errno is %{public}d",
             dstPath.c_str(), errno);
        return E_UMOUNT;
    }
    LOGI("MountManager::UMountDfsDocs end.");
    if (!filesystem::is_empty(dstPath)) {
        LOGE("[UMountDfsDocs] Failed to umount");
        return E_UMOUNT;
    }
    if (!RmDirRecurse(dstPath)) {
        LOGE("Failed to remove dir %{public}s", dstPath.c_str());
    }
    return E_OK;
}

int32_t MountManager::RestoreconSystemServiceDirs(int32_t userId)
{
    int32_t err = E_OK;
#ifdef USE_LIBRESTORECON
    for (const DirInfo &dir : systemServiceDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        RestoreconRecurse(path.c_str());
        LOGD("systemServiceDir_ RestoreconRecurse path is %{public}s ", path.c_str());
    }
#endif
    return err;
}

int32_t MountManager::CreateSystemServiceDirs(int32_t userId)
{
    int32_t err = E_OK;
    for (const DirInfo &dir : systemServiceDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        if (!PrepareDir(path, dir.mode, dir.uid, dir.gid)) {
            LOGE("failed to prepareDir %{public}s ", path.c_str());
            err = E_PREPARE_DIR;
        }
    }
    return err;
}

int32_t MountManager::DestroySystemServiceDirs(int32_t userId)
{
    bool err = true;
    for (const DirInfo &dir : systemServiceDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        err = err && RmDirRecurse(path);
    }
    return err ? E_OK : E_DESTROY_DIR;
}

int32_t MountManager::DestroyHmdfsDirs(int32_t userId)
{
    bool err = true;

    for (const DirInfo &dir : hmdfsDirVec_) {
        if (IsEndWith(dir.path.c_str(), "%d")) {
            err = err && RmDirRecurse(StringPrintf(dir.path.c_str(), userId));
        }
    }

    return err ? E_OK : E_DESTROY_DIR;
}


int32_t MountManager::DestroyFileManagerDirs(int32_t userId)
{
    bool err = true;

    for (const DirInfo &dir : fileManagerDir_) {
        if (IsEndWith(dir.path.c_str(), "%d")) {
            err = err && RmDirRecurse(StringPrintf(dir.path.c_str(), userId));
        }
    }

    return err ? E_OK : E_DESTROY_DIR;
}

int32_t MountManager::SetFafQuotaProId(int32_t userId)
{
    int32_t prjId = 0;
    for (const DirInfo &dir: fileManagerDir_) {
        QuotaManager::GetInstance()->SetQuotaPrjId(StringPrintf(dir.path.c_str(), userId), prjId, true);
    }
    QuotaManager::GetInstance()->SetQuotaPrjId(StringPrintf(SHARE_PATH.c_str(), userId), prjId, true);
    return E_OK;
}

bool MountManager::CheckMountFileByUser(int32_t userId)
{
    for (const DirInfo &dir : virtualDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        if (CloudDirFlag(path)) {
            continue;
        }
        if (access(path.c_str(), 0) != 0) {
            LOGI("VirtualDir : %{public}s is not exists", path.c_str());
            return false;
        }
    }
    LOGI("MountFile is exists");
    return true;
}

bool MountManager::CloudDirFlag(const std::string &path)
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

int32_t MountManager::SharedMount(const std::string &path)
{
    if (path.empty() || !IsDir(path)) {
        LOGE("path invalid, %{public}s", path.c_str());
        return ENOENT;
    }
    errno = 0;
    int32_t ret = Mount(path, path, nullptr, MS_BIND | MS_REC, nullptr);
    if (ret != 0) {
        LOGE("SharedMount failed, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        return ret;
    }
    ret = Mount(nullptr, path, nullptr, MS_SHARED, nullptr);
    if (ret != 0) {
        LOGE("SharedMount shared failed, path is %{public}s, errno is %{public}d.", path.c_str(), errno);
        return ret;
    }
    return E_OK;
}

int32_t MountManager::BindAndRecMount(std::string &srcPath, std::string &dstPath, bool isUseSlave)
{
    if (srcPath.empty() || !IsDir(srcPath)) {
        LOGE("path invalid, %{public}s", srcPath.c_str());
        return ENOENT;
    }
    if (dstPath.empty() || !IsDir(dstPath)) {
        LOGE("path invalid, %{public}s", dstPath.c_str());
        return ENOENT;
    }
    if (IsPathMounted(dstPath)) {
        LOGE("path has mounted, %{public}s", dstPath.c_str());
        return E_OK;
    }
    int32_t ret = Mount(srcPath, dstPath, nullptr, MS_BIND | MS_REC, nullptr);
    if (ret != 0) {
        LOGE("bind and rec mount failed, srcPath is %{public}s, dstPath is %{public}s, errno is %{public}d.",
             srcPath.c_str(), dstPath.c_str(), errno);
        return ret;
    }
    if (isUseSlave) {
        ret = Mount(nullptr, dstPath, nullptr, MS_SLAVE, nullptr);
        if (ret != 0) {
            LOGE("mount to slave failed, path is %{public}s, errno is %{public}d.", dstPath.c_str(), errno);
            return ret;
        }
    }
    return E_OK;
}

void MountManager::GetAllUserId(std::vector<int32_t> &userIds)
{
    const std::string &path = APP_EL1_PATH;
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
        int32_t userId = stoi(subPath);
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

int32_t MountManager::MountAppdata(const std::string &userId)
{
    std::vector<std::string> appdataSrc = APPDATA_SRC_PATH;
    std::vector<std::string> appdataDst = APPDATA_DST_PATH;
    int count = static_cast<int>(appdataSrc.size());
    for (int i = 1; i < count; i++) {
        std::string src = appdataSrc[i];
        std::string dst = appdataDst[i];
        ParseSandboxPath(src, userId, "");
        ParseSandboxPath(dst, userId, "");
        BindAndRecMount(src, dst);
    }
    return E_OK;
}

int32_t MountManager::MountSharefsAndNoSharefs(int32_t userId)
{
    Utils::MountArgument mountArgument(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    std::string path = mountArgument.GetNoSharefsDocPath();
    SharedMount(path);
    path = mountArgument.GetSharefsDocPath();
    SharedMount(path);

    std::string src = APPDATA_SRC_PATH[0];
    std::string dst = APPDATA_DST_PATH[0];
    ParseSandboxPath(src, to_string(userId), "");
    ParseSandboxPath(dst, to_string(userId), "");
    BindAndRecMount(src, dst);
    return E_OK;
}

int32_t MountManager::PrepareAppdataDirByUserId(int32_t userId)
{
    for (const DirInfo &dir: appdataDir_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }
    MountSharefsAndNoSharefs(userId);
    return E_OK;
}

int32_t MountManager::MountAppdataAndSharefs(int32_t userId)
{
    LOGI("mount appdata start, userId is %{public}d.", userId);
    MountAppdata(to_string(userId));

    LOGI("mount currentUser/other");
    Utils::MountArgument mountArgument(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    std::string mediaDocPath = mountArgument.GetMediaDocsPath();
    std::string curOtherPath = mountArgument.GetCurOtherPath();
    BindAndRecMount(mediaDocPath, curOtherPath);

    LOGI("mount currentUser/other/appdata");
    std::string noSharefsAppdataPath = mountArgument.GetNoSharefsAppdataPath();
    std::string curOtherAppdataPath = mountArgument.GetCurOtherAppdataPath();
    MkDir(curOtherAppdataPath, MODE_0711);
    BindAndRecMount(noSharefsAppdataPath, curOtherAppdataPath);

    LOGI("mount currentUser/filemgr");
    std::string curFileMgrPath = mountArgument.GetCurFileMgrPath();
    BindAndRecMount(mediaDocPath, curFileMgrPath);

    LOGI("mount currentUser/filemgr/appdata");
    std::string curFileMgrAppdataPath = mountArgument.GetCurFileMgrAppdataPath();
    MkDir(curFileMgrAppdataPath, MODE_0711);
    HmSharefsMount(userId, noSharefsAppdataPath, curFileMgrAppdataPath);

    LOGI("mount sharefs/docs/currentUser");
    std::string sharefsDocCurPath = mountArgument.GetSharefsDocCurPath();
    BindAndRecMount(curOtherPath, sharefsDocCurPath);

    LOGI("mount nosharefs/docs/currentUser");
    std::string noSharefsDocCurPath = mountArgument.GetNoSharefsDocCurPath();
    BindAndRecMount(curFileMgrPath, noSharefsDocCurPath);
    return E_OK;
}

int32_t MountManager::PrepareAppdataDir(int32_t userId)
{
    if (userId == 0) {
        std::vector<int32_t> userIds;
        GetAllUserId(userIds);
        if (userIds.empty()) {
            return E_OK;
        }
        for (const int32_t &item: userIds) {
            PrepareAppdataDirByUserId(item);
        }
    } else {
        PrepareAppdataDirByUserId(userId);
    }
    return E_OK;
}

int32_t MountManager::UmountMntUserTmpfs(int32_t userId)
{
    Utils::MountArgument mountArgument(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    std::string path = mountArgument.GetSharefsDocCurPath() + "/appdata";
    int32_t res = UMount2(path, MNT_DETACH);
    if (res != E_OK) {
        LOGE("failed to umount with detach, path %{public}s, errno {public}d.", path.c_str(), errno);
    }
    path = mountArgument.GetCurOtherAppdataPath();
    res = UMount2(path, MNT_DETACH);
    if (res != E_OK) {
        LOGE("failed to umount with detach, path %{public}s, errno {public}d.", path.c_str(), errno);
    }
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
