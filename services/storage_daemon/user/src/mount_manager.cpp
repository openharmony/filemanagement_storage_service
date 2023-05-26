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

#include "user/mount_manager.h"
#include <cstdlib>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include "ipc/istorage_daemon.h"
#include "parameter.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
#include "utils/mount_argument_utils.h"
#include "utils/string_utils.h"
#include "system_ability_definition.h"
#include "cloud_daemon_manager.h"


namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace OHOS::FileManagement::CloudFile;
constexpr int32_t UMOUNT_RETRY_TIMES = 3;
std::shared_ptr<MountManager> MountManager::instance_ = nullptr;

const std::string HMDFS_SYS_CAP = "const.distributed_file_property.enabled";
const int32_t HMDFS_VAL_LEN = 6;
const int32_t HMDFS_TRUE_LEN = 5;
MountManager::MountManager()
    : hmdfsDirVec_{{"/data/service/el2/%d/share", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/fuse", 0771, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/account", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/account/files", 02771, OID_USER_DATA_RW, OID_USER_DATA_RW},
                   {"/data/service/el2/%d/hmdfs/account/data", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/non_account/files", 0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
                   {"/data/service/el2/%d/hmdfs/non_account/data", 0711, OID_SYSTEM, OID_SYSTEM},
                   {"/data/service/el2/%d/hmdfs/cache", 0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache/account_cache", 0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/cache/non_account_cache", 0711, OID_DFS, OID_DFS},
                   {"/data/service/el2/%d/hmdfs/account/services", 0771, OID_DFS_SHARE, OID_DFS_SHARE}},
      virtualDir_{{"/storage/media/%d", 0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
                  {"/storage/media/%d/local", 0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
                  {"/storage/cloud", 0711, OID_ROOT, OID_ROOT},
                  {"/storage/cloud/%d", 0711, OID_USER_DATA_RW, OID_USER_DATA_RW},
                  {"/mnt/share/", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/share/%d/", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/account", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/non_account", 0711, OID_ROOT, OID_ROOT},
                  {"/mnt/hmdfs/%d/cloud", 0711, OID_ROOT, OID_ROOT}}
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

int32_t MountManager::SharefsUMount(int32_t userId)
{
    Utils::MountArgument sharefsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    int32_t ret = UMount2(sharefsMntArgs.GetShareDst().c_str(), MNT_DETACH);
    if (ret != E_OK) {
        LOGE("umount sharefs, errno %{public}d, sharefs dst %{public}s", errno,
             sharefsMntArgs.GetShareDst().c_str());
        return E_UMOUNT;
    }
    return E_OK;
}

int32_t MountManager::HmdfsMount(int32_t userId, std::string relativePath)
{
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    int ret = Mount(hmdfsMntArgs.GetFullSrc(), hmdfsMntArgs.GetFullDst(), "hmdfs",
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


int32_t MountManager::CloudMount(int32_t userId)
{
    int fd = -1;
    string opt;
    int ret;
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string path = cloudMntArgs.GetFullCloud();
    if (!cloudReady_) {
        LOGI("Cloud Service has not started");
        return E_MOUNT;
    }

    fd = open("/dev/fuse", O_RDWR);
    if (fd < 0) {
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

    ret = CloudDaemonManager::GetInstance().StartFuse(fd, path);
    if (ret) {
        LOGE("failed to connect fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        UMount(path.c_str());
    }
    close(fd);
    return ret;
}

int32_t MountManager::HmdfsMount(int32_t userId)
{
    int32_t ret = HmdfsTwiceMount(userId, "account");

    ret += HmdfsMount(userId, "non_account");
    if (ret != E_OK) {
        return E_MOUNT;
    }

    mountMutex_.lock();
    ret = CloudMount(userId);
    if (ret)
        activeUsers_.push_back(userId);
    mountMutex_.unlock();

    return E_OK;
}

void MountManager::MountCloudForUsers()
{
    for (uint32_t i = 0; i < activeUsers_.size(); i++) {
        CloudMount(activeUsers_[i]);
    }
}

void MountManager::SetCloudState(bool active)
{
    mountMutex_.lock();
    cloudReady_ = active;
    if (cloudReady_) {
        MountCloudForUsers();
    }
    mountMutex_.unlock();
}

int32_t MountManager::HmdfsTwiceUMount(int32_t userId, std::string relativePath)
{
    int32_t err = E_OK;
    // un bind mount
    Utils::MountArgument hmdfsMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, relativePath));
    err = UMount(hmdfsMntArgs.GetCommFullPath());
    if (err != E_OK) {
        LOGE("failed to un bind mount, errno %{public}d, ComDataDir_ dst %{public}s", errno,
             hmdfsMntArgs.GetCommFullPath().c_str());
    }
    err = UMount(hmdfsMntArgs.GetCloudFullPath());
    if (err != E_OK) {
        LOGE("failed to un bind mount, errno %{public}d, CloudDataDir dst %{public}s", errno,
             hmdfsMntArgs.GetCloudFullPath().c_str());
    }

    err = UMount2(hmdfsMntArgs.GetFullDst().c_str(), MNT_DETACH);
    if (err != E_OK) {
        LOGE("identical account hmdfs umount failed, errno %{public}d, hmdfs dst %{public}s", errno,
             hmdfsMntArgs.GetFullDst().c_str());
        return E_UMOUNT;
    }
    return E_OK;
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
    int32_t err = E_OK;
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string path = cloudMntArgs.GetFullCloud();

    err = UMount(path);
    if (err != E_OK) {
        LOGE("fuse umount failed, errno %{public}d, fuse dst %{public}s", errno, path.c_str());
        return E_UMOUNT;
    }
    return E_OK;
}

int32_t MountManager::HmdfsUMount(int32_t userId)
{
    int32_t ret = HmdfsTwiceUMount(userId, "account");
    ret += HmdfsUMount(userId, "non_account");
    if (ret != E_OK) {
        return E_UMOUNT;
    }
    CloudUMount(userId);
    return E_OK;
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

int32_t MountManager::MountByUser(int32_t userId)
{
    int ret = E_OK;
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
        return ret;
    }
    return E_OK;
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
        err = SharefsUMount(userId);
        if (err != E_OK) {
            LOGE("failed to umount sharefs, errno %{public}d", errno);
        }
        if (!SupportHmdfs()) {
            err = LocalUMount(userId);
        } else {
            err = HmdfsUMount(userId);
        }
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

int32_t MountManager::CreateVirtualDirs(int32_t userId)
{
    for (const DirInfo &dir : virtualDir_) {
        if (!PrepareDir(StringPrintf(dir.path.c_str(), userId), dir.mode, dir.uid, dir.gid)) {
            return E_PREPARE_DIR;
        }
    }

    return E_OK;
}

int32_t MountManager::DestroyHmdfsDirs(int32_t userId)
{
    bool err = true;

    for (const DirInfo &dir : hmdfsDirVec_) {
        if (IsEndWith(dir.path.c_str(), "%d")) {
            err = RmDirRecurse(StringPrintf(dir.path.c_str(), userId));
        }
    }

    return err ? E_OK : E_DESTROY_DIR;
}
} // namespace StorageDaemon
} // namespace OHOS
