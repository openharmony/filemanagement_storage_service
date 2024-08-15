/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "utils/set_flag_utils.h"
#include "utils/file_utils.h"

#include <fcntl.h>
#include <filesystem>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "storage_service_log.h"

namespace OHOS {
namespace StorageService {
#define HMFS_MONITOR_FL 0x00000002
#define HMFS_IOCTL_HW_GET_FLAGS _IOR(0XF5, 70, unsigned int)
#define HMFS_IOCTL_HW_SET_FLAGS _IOR(0XF5, 71, unsigned int)

void SetFlagUtils::ParseDirPath(const std::string &path)
{
    if (!IsDir(path)) {
        LOGE("Input path is not a directory.");
        return;
    }
    SetDirDelFlags(path);
    std::filesystem::directory_iterator pathList(path);
    for (const auto& resPath : pathList) {
        if (IsDir(resPath.path())) {
            ParseDirPath(resPath.path().c_str());
        } else if (IsFile(resPath.path())) {
            SetFileDelFlags(resPath.path().c_str());
        } else {
            LOGE("Invalid file path.");
        }
    }
}

void SetFlagUtils::SetFileDelFlags(const std::string &filepath)
{
    LOGI("SetFlagUtils SetFileDelFlags for filepath=%{public}s start.", filepath.c_str());
    char absPath[PATH_MAX] = {0};
    if (realpath(filepath.c_str(), absPath) == nullptr) {
        LOGE("SetFlagUtils Failed to get realpath");
        return;
    }
    int32_t fd = open(absPath, O_RDWR);
    if (fd < 0) {
        LOGE("SetFlagUtils Failed to open dir, errno: %{public}d", errno);
        return;
    }
    unsigned int flags = 0;
    int32_t ret = ioctl(fd, HMFS_IOCTL_HW_GET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to get flags, errno: %{public}d", errno);
        close(fd);
        return;
    }
    if (flags & HMFS_MONITOR_FL) {
        LOGE("SetFlagUtils Delete control flag ia already set");
        close(fd);
        return;
    }
    flags |= HMFS_MONITOR_FL;
    ret  = ioctl(fd, HMFS_IOCTL_HW_SET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to set flags, errno: %{public}d", errno);
    }
    close(fd);
}

void SetFlagUtils::SetDirDelFlags(const std::string &dirpath)
{
    LOGI("SetFlagUtils SetDirDelFlags for dirpath=%{public}s start.", dirpath.c_str());
    char absPath[PATH_MAX] = {0};
    if (realpath(dirpath.c_str(), absPath) == nullptr) {
        LOGE("SetFlagUtils Failed to get realpath");
        return;
    }
    int32_t fd = open(absPath, O_DIRECTORY);
    if (fd < 0) {
        LOGE("SetFlagUtils Failed to open dir, errno: %{public}d", errno);
        return;
    }
    unsigned int flags = 0;
    int32_t ret = ioctl(fd, HMFS_IOCTL_HW_GET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to get flags, errno: %{public}d", errno);
        close(fd);
        return;
    }
    if (flags & HMFS_MONITOR_FL) {
        LOGE("SetFlagUtils Delete control flag ia already set");
        close(fd);
        return;
    }
    flags |= HMFS_MONITOR_FL;
    ret  = ioctl(fd, HMFS_IOCTL_HW_SET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to set flags, errno: %{public}d", errno);
    }
    close(fd);
}
}
}