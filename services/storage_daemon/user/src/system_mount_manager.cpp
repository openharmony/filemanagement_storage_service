/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "user/system_mount_manager.h"

#include <sys/stat.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/mount_argument_utils.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#ifdef DFS_SERVICE
#include "cloud_daemon_manager.h"
#endif

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace OHOS::StorageService;
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif
SystemMountManager &SystemMountManager::GetInstance()
{
    static SystemMountManager instance;
    return instance;
}

int32_t SystemMountManager::MountCloudByUserId(int32_t userId)
{
    mountMutex_.lock();
    auto ret = CloudTwiceMount(userId);
    if (ret == E_OK) {
        fuseMountedUsers_.push_back(userId);
    } else {
        fuseToMountUsers_.push_back(userId);
    }
    mountMutex_.unlock();
    return ret;
}

int32_t SystemMountManager::UMountCloudByUserId(int32_t userId)
{
    mountMutex_.lock();
    auto ret = CloudTwiceUMount(userId);
    auto it = std::find(fuseMountedUsers_.begin(), fuseMountedUsers_.end(), userId);
    if (it != fuseMountedUsers_.end()) {
        fuseMountedUsers_.erase(it);
    }
    auto it2 = std::find(fuseToMountUsers_.begin(), fuseToMountUsers_.end(), userId);
    if (it2 != fuseToMountUsers_.end()) {
        fuseToMountUsers_.erase(it2);
    }
    mountMutex_.unlock();
    return ret;
}

void SystemMountManager::SetCloudState(bool active)
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

void SystemMountManager::MountCloudForUsers(void)
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

void SystemMountManager::UMountCloudForUsers(void)
{
    for (auto it = fuseMountedUsers_.begin(); it != fuseMountedUsers_.end();) {
        int32_t res = CloudTwiceUMount(*it);
        if (res == E_OK) {
            fuseToMountUsers_.push_back(*it);
            it = fuseMountedUsers_.erase(it);
        } else {
            it++;
        }
    }
}

int32_t SystemMountManager::CloudTwiceMount(int32_t userId)
{
    LOGI("mount cloud start");
    int32_t ret = E_OK;
#ifdef DFS_SERVICE
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    string cloudPath = cloudMntArgs.GetFullCloud();
    int32_t mountRet = E_OK;
    if (IsPathMounted(cloudPath)) {
        LOGI("path has mounted, %{public}s", cloudPath.c_str());
    } else {
        mountRet = CloudMount(userId, cloudPath);
        if (mountRet != E_OK && mountRet != E_CLOUD_NOT_READY) {
            std::string extraData = "dstPath=" + cloudPath + ",kernelCode=" + to_string(mountRet);
            StorageRadar::ReportUserManager("CloudTwiceMount", userId, E_MOUNT_CLOUD_FUSE, extraData);
            ret = E_MOUNT_CLOUD_FUSE;
        }
    }
    string cloudMediaPath = cloudMntArgs.GetFullMediaCloud();
    if (IsPathMounted(cloudMediaPath)) {
        LOGI("path has mounted, %{public}s", cloudMediaPath.c_str());
    } else {
        mountRet = CloudMount(userId, cloudMediaPath);
        if (mountRet != E_OK && mountRet != E_CLOUD_NOT_READY) {
            std::string extraData = "dstPath=" + cloudMediaPath + ",kernelCode=" + to_string(mountRet);
            StorageRadar::ReportUserManager("CloudTwiceMount", userId, E_MOUNT_CLOUD, extraData);
            ret = E_MOUNT_CLOUD;
        }
    }
    return ret;
#else
    return ret;
#endif
}

int32_t SystemMountManager::CloudTwiceUMount(int32_t userId)
{
#ifdef DFS_SERVICE
    int32_t err = E_OK;
    Utils::MountArgument cloudMntArgs(Utils::MountArgumentDescriptors::Alpha(userId, ""));
    const string cloudFusePath = cloudMntArgs.GetFullCloud();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    err = UMount2(cloudFusePath, MNT_DETACH);
    if (err != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("cloud fuse umount failed, errno is %{public}d.", errno);
        std::string extraData = "dstPath=" + cloudFusePath + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("CloudUMount", userId, E_UMOUNT_CLOUD_FUSE, extraData);
        return E_UMOUNT_CLOUD_FUSE;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT FULL COULD",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UMOUNT2: UMOUNT FULL COULD, delayTime = %{public}s", delay.c_str());

    startTime = StorageService::StorageRadar::RecordCurrentTime();
    const std::string cloudPath = cloudMntArgs.GetFullMediaCloud();
    err = UMount2(cloudPath, MNT_DETACH);
    if (err != E_OK && errno != ENOENT && errno != EINVAL) {
        LOGE("cloud umount failed, errno %{public}d", errno);
        std::string extraData = "dstPath=" + cloudPath + ",kernelCode=" + to_string(errno);
        StorageRadar::ReportUserManager("CloudUMount", userId, E_UMOUNT_CLOUD, extraData);
        return E_UMOUNT_CLOUD;
    }
    delay = StorageService::StorageRadar::ReportDuration("UMOUNT2: UMOUNT FULL MEDIA COULD",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: UMOUNT2: UMOUNT FULL MEDIA COULD, delayTime = %{public}s. cloud umount success", delay.c_str());
    return E_OK;
#else
    return E_OK;
#endif
}

int32_t SystemMountManager::CloudMount(int32_t userId, const string &path)
{
#ifdef DFS_SERVICE
    string opt;
    int ret;
    if (!cloudReady_) {
        LOGI("Cloud Service has not started");
        return E_CLOUD_NOT_READY;
    }
    FILE *f = fopen("/dev/fuse", "r+");
    if (f == nullptr) {
        LOGE("open /dev/fuse fail");
        return E_USER_MOUNT_ERR;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("open /dev/fuse fail");
        (void)fclose(f);
        return E_USER_MOUNT_ERR;
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
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    ret = Mount("/dev/fuse", path.c_str(), "fuse", MS_NOSUID | MS_NODEV | MS_NOEXEC | MS_NOATIME, opt.c_str());
    if (ret) {
        LOGE("failed to mount fuse, err %{public}d %{public}d %{public}s", errno, ret, path.c_str());
        (void)fclose(f);
        return ret;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("MOUNT: CLOUD MOUNT",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, userId);
    LOGI("SD_DURATION: CLOUD MOUNT, delayTime = %{public}s. start cloud daemon fuse.", delay.c_str());
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
} // namespace StorageDaemon
} // namespace OHOS