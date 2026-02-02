/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "storage_service_log.h"
#include "utils/set_flag_utils.h"

#include <fcntl.h>
#include <filesystem>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace OHOS {
namespace StorageService {

#define FDSAN_TAG 1
#define HMFS_MONITOR_FL 0x00000002
#define HMFS_IOCTL_HW_GET_FLAGS _IOR(0xF5, 70, unsigned int)
#define HMFS_IOCTL_HW_SET_FLAGS _IOR(0xF5, 71, unsigned int)

void SetFlagUtils::SetDelFlagsRecursive(const std::string& path)
{
    LOGI("SetFlagUtils SetDelFlagsRecursive start.");
    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) {
        LOGE("SetFlagUtils path does not exist or is not a directory: %{public}s", path.c_str());
        return;
    }
    SetDirDelFlags(path);
    std::filesystem::directory_iterator pathList(path);
    for (const auto& resPath : pathList) {
        if (std::filesystem::is_directory(resPath)) {
            SetDelFlagsRecursive(resPath.path().c_str());
        } else {
            SetFileDelFlags(resPath.path().c_str());
        }
    }
}

void SetFlagUtils::SetDelFlagsImpl(const std::string& path, int openFlags)
{
    LOGI("SetFlagUtils SetDelFlagsImpl for path start.");
    char absPath[PATH_MAX] = {0};
    if (realpath(path.c_str(), absPath) == nullptr) {
        LOGE("SetFlagUtils Failed to get realpath, errno: %{public}d", errno);
        return;
    }
    uint64_t newTag = static_cast<uint64_t>(LOG_DOMAIN) << 32 | FDSAN_TAG;
    int32_t fd = open(absPath, openFlags);
    if (fd < 0) {
        LOGE("SetFlagUtils Failed to open path, errno: %{public}d", errno);
        return;
    }
    fdsan_exchange_owner_tag(fd, 0, newTag);
    unsigned int flags = 0;
    int32_t ret = ioctl(fd, HMFS_IOCTL_HW_GET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to get flags, errno: %{public}d", errno);
        fdsan_close_with_tag(fd, newTag);
        return;
    }
    if (flags & HMFS_MONITOR_FL) {
        LOGE("SetFlagUtils Delete control flag is already set");
        fdsan_close_with_tag(fd, newTag);
        return;
    }
    flags |= HMFS_MONITOR_FL;
    ret = ioctl(fd, HMFS_IOCTL_HW_SET_FLAGS, &flags);
    if (ret < 0) {
        LOGE("SetFlagUtils Failed to set flags, errno: %{public}d", errno);
    }
    fdsan_close_with_tag(fd, newTag);
}

void SetFlagUtils::SetFileDelFlags(const std::string& filepath)
{
    LOGI("SetFlagUtils SetFileDelFlags for filepath start.");
    SetDelFlagsImpl(filepath, O_RDWR);
}

void SetFlagUtils::SetDirDelFlags(const std::string& dirpath)
{
    LOGI("SetFlagUtils SetDirDelFlags for dirpath start.");
    SetDelFlagsImpl(dirpath, O_DIRECTORY);
}

}  // namespace StorageService
}  // namespace OHOS
