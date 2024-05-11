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
#include <dirent.h>
#include <fcntl.h>
#include <set>
#include <sys/mount.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <regex>
#include <filesystem>
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
std::shared_ptr<MountManager> MountManager::instance_ = nullptr;

const string SANDBOX_ROOT_PATH = "/mnt/sandbox/";
const string CURRENT_USER_ID_FLAG = "<currentUserId>";
const string PACKAGE_NAME_FLAG = "<bundleName>";
const string SCENE_BOARD_BUNDLE_NAME = "com.ohos.sceneboard";
const string PUBLIC_DIR_SANDBOX_PATH = "/storage/Users/currentUser";
const string PUBLIC_DIR_SRC_PATH = "/storage/media/<currentUserId>/local/files/Docs";
const string MOUNT_POINT_INFO = "/proc/mounts";
const string MOUNT_POINT_TYPE_HMDFS = "hmdfs";
const string MOUNT_POINT_TYPE_HMFS = "hmfs";
const string MOUNT_POINT_TYPE_SHAREFS = "sharefs";
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
    "/data/storage/el2/distributedfiles/"
};
const vector<string> CRYPTO_SRC_PATH = {
    "/data/app/el2/<currentUserId>/base/<bundleName>/",
    "/data/app/el2/<currentUserId>/database/<bundleName>/",
    "/mnt/share/<currentUserId>/<bundleName>/",
    "/data/app/el2/<currentUserId>/log/<bundleName>/",
    "/mnt/hmdfs/<currentUserId>/account/merge_view/data/<bundleName>/"
};
const std::string HMDFS_SYS_CAP = "const.distributed_file_property.enabled";
const int32_t HMDFS_VAL_LEN = 6;
const int32_t HMDFS_TRUE_LEN = 5;
const string SHARE_PATH = "/data/service/el1/public/storage_daemon/share/public";
static constexpr int MODE_0711 = 0711;
static constexpr int MODE_0771 = 0771;
static constexpr int MODE_02771 = 02771;
MountManager::MountManager()
    : hmdfsDirVec_{{"/data/service/el2/%d/share", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account/files", MODE_02771, OID_USER_DATA_RW, OID_USER_DATA_RW},
                   {"/data/service/el2/%d/hmdfs/account/data", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account/files", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
                   {"/data/service/el2/%d/hmdfs/non_account/data", MODE_0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/cloud", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cloud/data", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache/account_cache", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache/non_account_cache", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache/cloud_cache", MODE_0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/account/services", MODE_0771, OID_DFS_SHARE, OID_DFS_SHARE}},
      virtualDir_{{"/storage/media/%d", MODE_0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
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
                  {"/mnt/hmdfs/%d/non_account", MODE_0711, OID_ROOT, OID_ROOT}},
      systemServiceDir_{{"/data/service/el2/%d/tee", MODE_0711, OID_TEE, OID_TEE},
                  {"/data/service/el2/%d/deviceauth", MODE_0711, OID_DEVICE_AUTH, OID_DEVICE_AUTH},
                  {"/data/service/el2/%d/huks_service", MODE_0711, OID_HUKS, OID_HUKS},
                  {"/data/service/el4/%d/huks_service", MODE_0711, OID_HUKS, OID_HUKS},
                  {"/data/service/el2/%d/dlp_credential_service", MODE_0711, OID_DLP_CREDENTIAL, OID_DLP_CREDENTIAL}},
      fileManagerDir_{{"/data/service/el2/%d/hmdfs/account/files/Docs", MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
                   {"/data/service/el2/%d/hmdfs/account/files/Docs/Documents",
                   MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
                   {"/data/service/el2/%d/hmdfs/account/files/Docs/Download",
                   MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
                   {"/data/service/el2/%d/hmdfs/account/files/Docs/Desktop",
                   MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
                   {"/data/service/el2/%d/hmdfs/account/files/Docs/.Trash",
                   MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER},
                   {"/data/service/el2/%d/hmdfs/account/files/.Recent", MODE_02771, OID_FILE_MANAGER, OID_FILE_MANAGER}}
{
}

std::shared_ptr<MountManager> MountManager::GetInstance()
{
    static std::once_flag onceFlag;
    std::call_once(onceFlag, [&]() { instance_ = std::make_shared<MountManager>(); });

    return instance_;
}

int32_t MountManager::HmdfsTwiceMount(int32_t userId, std::string relativePath)
{
    int32_t ret = HmdfsMount(userId, relativePath);

    // bind mount
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    ret += Mount(hmdfsMntArgs.GetFullDst() + "/device_view/", hmdfsMntArgs.GetCommFullPath(),
                 nullptr, MS_BIND, nullptr);
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to bind mount device_view, err %{public}d", errno);
        return E_MOUNT;
    }
    ret += Mount(hmdfsMntArgs.GetFullDst() + "/cloud_merge_view/", hmdfsMntArgs.GetCloudFullPath(),
                 nullptr, MS_BIND, nullptr);
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to bind mount cloud_merge_view, err %{public}d", errno);
        return E_MOUNT;
    }

    if (Mount(hmdfsMntArgs.GetLocalDocsPath(), hmdfsMntArgs.GetCloudDocsPath(),
              nullptr, MS_BIND, nullptr)) {
        LOGE("failed to bind mount docs, err %{public}d", errno);
    }
    return E_OK;
}

int32_t MountManager::SharefsMount(int32_t userId)
{
    Utils::MountArgument sharefsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    int ret = Mount(sharefsMntArgs.GetShareSrc(), sharefsMntArgs.GetShareDst(), "sharefs",
                    sharefsMntArgs.GetFlags(), sharefsMntArgs.GetUserIdPara().c_str());
    if (ret != 0 && errno != EEXIST && errno != EBUSY) {
        LOGE("failed to mount sharefs, err %{public}d", errno);
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

    int ret = Mount(mountSrcPath, hmdfsMntArgs.GetFullDst(), "hmdfs",
                    hmdfsMntArgs.GetFlags(), hmdfsMntArgs.OptionsToString().c_str());
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

int32_t MountManager::CloudMount(int32_t userId, const string& path)
{
#ifdef DFS_SERVICE
    int fd = -1;
    string opt;
    int ret;
    if (!cloudReady_) {
        LOGI("Cloud Service has not started");
        return E_MOUNT;
    }

    fd = open("/dev/fuse", O_RDWR);
    if (fd < 0) {
        LOGE("open /dev/fuse fail");
        return E_MOUNT;
    }

    opt = StringPrintf("fd=%i,"
        "rootmode=40000,"
        "default_permissions,"
        "allow_other,"
        "user_id=0,group_id=0,"
        "context=\"u:object_r:hmdfs:s0\","
        "fscontext=u:object_r:hmdfs:s0",
        fd);
    ret = Mount("/dev/fuse", path.c_str(), "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("failed to mount fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        close(fd);
        return ret;
    }

    ret = CloudDaemonManager::GetInstance().StartFuse(userId, fd, path);
    if (ret) {
        LOGE("failed to connect fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        UMount(path.c_str());
    }
    LOGI("mount %{public}s success", path.c_str());
    close(fd);
    return ret;
#else
    return E_OK;
#endif
}

int32_t MountManager::CloudTwiceMount(int32_t userId)
{
#ifdef DFS_SERVICE
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string cloudPath = cloudMntArgs.GetFullCloud();
    int32_t ret = CloudMount(userId, cloudPath);
    if (ret != E_OK) {
        LOGE("failed to mount cloud disk fuse, err %{public}d path %{public}s", errno, cloudPath.c_str());
        return ret;
    }
    const string cloudMediaPath = cloudMntArgs.GetFullMediaCloud();
    ret = CloudMount(userId, cloudMediaPath);
    if (ret != E_OK) {
        LOGE("failed to mount cloud media fuse, err %{public}d path %{public}s", errno, cloudMediaPath.c_str());
        return ret;
    }
    return ret;
#else
    return E_OK;
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

int32_t MountManager::MountCryptoPathAgain(uint32_t userId)
{
    filesystem::path rootDir(SANDBOX_ROOT_PATH + "/" + to_string(userId));
    if (!exists(rootDir)) {
        LOGE("root path not exists, rootDir is %{public}s", SANDBOX_ROOT_PATH.c_str());
        return -ENOENT;
    }

    int32_t ret = 0;
    filesystem::directory_iterator bundleNameList(rootDir);
    for (const auto &bundleName : bundleNameList) {
        if (SANDBOX_EXCLUDE_PATH.find(bundleName.path().filename()) != SANDBOX_EXCLUDE_PATH.end()) {
            continue;
        }

        vector<string> cryptoSandboxPathVector = CRYPTO_SANDBOX_PATH;
        vector<string> cryptoSandboxSrcVector = CRYPTO_SRC_PATH;
        if (bundleName.path().filename().generic_string() == SCENE_BOARD_BUNDLE_NAME) {
            cryptoSandboxPathVector.push_back(PUBLIC_DIR_SANDBOX_PATH);
            cryptoSandboxSrcVector.push_back(PUBLIC_DIR_SRC_PATH);
        }

        for (size_t i = 0; i < cryptoSandboxPathVector.size(); i++) {
            string dstPath = bundleName.path().generic_string() + cryptoSandboxPathVector[i];
            string srcPath = cryptoSandboxSrcVector[i];
            ParseSandboxPath(srcPath, to_string(userId), bundleName.path().filename().generic_string());
            LOGD("mount crypto path, srcPath is %{public}s, dstPath is %{public}s", srcPath.c_str(), dstPath.c_str());
            ret = mount(srcPath.c_str(), dstPath.c_str(), nullptr, MS_BIND | MS_REC, nullptr);
            if (ret != 0) {
                LOGE("mount bind failed, srcPath is %{public}s dstPath is %{public}s errno is %{public}d",
                    srcPath.c_str(), dstPath.c_str(), errno);
                continue;
            }
            ret = mount(nullptr, dstPath.c_str(), nullptr, MS_SHARED, nullptr);
            if (ret != 0) {
                LOGE("mount to share failed, srcPath is %{public}s dstPath is %{public}s errno is %{public}d",
                    srcPath.c_str(), dstPath.c_str(), errno);
                continue;
            }
            LOGD("mount crypto path success, srcPath is %{public}s dstPath is %{public}s",
                srcPath.c_str(), dstPath.c_str());
        }
    }
    return ret;
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
    const string &sharefsPrefix = hmdfsMntArgs.GetShareSrc();
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
        return;
    }
    if (type == MOUNT_POINT_TYPE_HMFS) {
        if (dst.length() >= hmfsPrefix.length() && dst.substr(0, hmfsPrefix.length()) == hmfsPrefix) {
            hmfsList.push_front(dst);
        }
        return;
    }
    if (type == MOUNT_POINT_TYPE_SHAREFS) {
        if (src.length() >= sharefsPrefix.length() &&
            src.substr(0, sharefsPrefix.length()) == sharefsPrefix) {
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

int32_t MountManager::UMountAllPath(int32_t userId)
{
    std::map<std::string, std::list<std::string>> mountMap;
    int32_t res = FindMountPointsToMap(mountMap, userId);
    if (res != E_OK) {
        return res;
    }
    std::list<std::string> list = mountMap[MOUNT_POINT_TYPE_HMFS];
    int total = static_cast<int>(list.size());
    LOGI("unmount hmfs path start, total %{public}d.", total);
    for (const std::string &path: list) {
        LOGD("unmount hmfs path %{public}s.", path.c_str());
        res = UMount2(path.c_str(), MNT_DETACH);
        if (res != E_OK) {
            LOGE("failed to unmount hmfs path %{public}s, errno %{public}d.", path.c_str(), errno);
        }
    }
    list = mountMap[MOUNT_POINT_TYPE_SHAREFS];
    total = static_cast<int>(list.size());
    LOGI("unmount sharefs path start, total %{public}d.", total);
    for (const std::string &path: list) {
        LOGD("unmount sharefs path %{public}s.", path.c_str());
        res = UMount2(path.c_str(), MNT_DETACH);
        if (res != E_OK) {
            LOGE("failed to unmount sharefs path %{public}s, errno %{public}d.", path.c_str(), errno);
        }
    }
    list = mountMap[MOUNT_POINT_TYPE_HMDFS];
    total = static_cast<int>(list.size());
    LOGI("unmount hmdfs path start, total %{public}d.", total);
    for (const std::string &path: list) {
        LOGD("unmount hmdfs path %{public}s.", path.c_str());
        res = UMount2(path.c_str(), MNT_DETACH);
        if (res != E_OK) {
            LOGE("failed to unmount hmdfs path %{public}s, errno %{public}d.", path.c_str(), errno);
        }
    }
    return E_OK;
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
    mountMutex_.lock();
    cloudReady_ = active;
    if (cloudReady_) {
        MountCloudForUsers();
    } else {
        UMountCloudForUsers();
    }
    mountMutex_.unlock();
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
    if (!exists(rootDir)) {
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
    int ret = E_OK;
    // The Documnets and Download directories are managed by the File access framework,
    // and the UID GID is changed to filemanager
    std::thread thread(ClearRedundantResources, userId);
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
    SetFafQuotaProId(userId);

    if (CreateSystemServiceDirs(userId) != E_OK) {
        LOGE("create system service dir error");
        return E_PREPARE_DIR;
    }
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
    int32_t count = 0;
    while (count < UMOUNT_RETRY_TIMES) {
        int32_t err = E_OK;
        if (!SupportHmdfs()) {
            err = LocalUMount(userId);
        } else {
            err = UMountAllPath(userId);
        }
        err = CloudUMount(userId);
        if (err == E_OK) {
            break;
        } else if (errno == EBUSY) {
            count++;
            continue;
        }
        LOGE("failed to umount hmdfs, errno %{public}d", errno);
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
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
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

int32_t MountManager::RestoreconSystemServiceDirs(int32_t userId)
{
    int32_t err = E_OK;
#ifdef USE_LIBRESTORECON
    for (const DirInfo &dir : systemServiceDir_) {
        std::string path = StringPrintf(dir.path.c_str(), userId);
        RestoreconRecurse(path.c_str());
        LOGD("systemServiceDir_ RestoreconRecurse path is %{private}s ", path.c_str());
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
} // namespace StorageDaemon
} // namespace OHOS
