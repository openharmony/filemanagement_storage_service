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

#include <securec.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <sys/sysmacros.h>

#include "disk_manager/disk/dm_device.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {

constexpr const size_t DM_ALIGN_SIZE = 8;
constexpr const uint64_t SECTOR_SIZE = 512;

bool DmDevice::Create()
{
    if (stage_ == Stage::ACTIVE) {
        return true;
    }
    if (!paramsValid_) {
        return false;
    }
    if (stage_ != Stage::NONE && !RemoveDevice()) {
        return false;
    }

    if (Exists()) {
        return stage_ == Stage::ACTIVE;
    }

    if (!CreateDevice() || !LoadTable() || !ResumeDevice()) {
        return false;
    }

    return true;
}

dev_t DmDevice::GetDeviceDev()
{
    if (stage_ != Stage::ACTIVE) {
        return 0;
    }
    return dev_;
}

static constexpr size_t DmAlign(size_t x, size_t alignSize = DM_ALIGN_SIZE)
{
    return (x + alignSize - 1) & ~(alignSize - 1);
}

std::string DmDevice::GetDmName(const std::string &devPath)
{
    if (devPath.empty()) {
        LOGE("devPath is empty");
        return "";
    }
    std::string baseName = devPath.substr(devPath.find_last_of('/') + 1);
    if (baseName.empty()) {
        LOGE("baseName is empty");
        return "";
    }

    std::string dmName = std::string("dm_linear_") + baseName;
    return dmName;
}

uint64_t DmDevice::GetDeviceSectors()
{
    uint64_t sectors = 0;
    uint64_t bytes = 0;
    int fd = open(sourceDevPath_.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        LOGE("Failed to open device: %{public}s, %{public}s", sourceDevPath_.c_str(), strerror(errno));
        return 0;
    }

    if (ioctl(fd, BLKGETSIZE64, &bytes) != -1) {
        sectors = bytes / SECTOR_SIZE;
    } else {
        LOGE("BLKGETSIZE64 failed for %{public}s: %{public}s", sourceDevPath_.c_str(), strerror(errno));
    }

    close(fd);
    return sectors;
}

// buffer_ 内存对齐：[struct dm_ioctl] + [padding] + [struct dm_target_spec] + [params] + [padding]
void DmDevice::PrepareBuffer()
{
    params_ = sourceDevPath_ + " " + std::to_string(sourceStartSector_);
    size_t alignedTargetBlockSize = DmAlign(sizeof(struct dm_target_spec) + params_.length() + 1);
    ioctlSize_ = DmAlign(sizeof(struct dm_ioctl));
    totalSize_ = ioctlSize_ + alignedTargetBlockSize;
    buffer_ = std::make_unique<char[]>(totalSize_);
    dm_ = reinterpret_cast<struct dm_ioctl *>(buffer_.get());
}

bool DmDevice::InitDmIoctl(bool hasTargets)
{
    if (memset_s(dm_, totalSize_, 0, totalSize_) != EOK) {
        LOGE("memset_s failed");
        return false;
    }

    dm_->version[0] = DM_VERSION_MAJOR;
    dm_->version[1] = DM_VERSION_MINOR;
    dm_->version[2] = DM_VERSION_PATCHLEVEL;
    dm_->data_size = totalSize_;
    dm_->data_start = ioctlSize_;
    dm_->flags = 0;  // 只对 DM_DEV_SUSPEND ioctl 指令有影响
    dm_->target_count = hasTargets ? 1 : 0;

    if (strncpy_s(dm_->name, sizeof(dm_->name), dmName_.c_str(), sizeof(dm_->name) - 1) != EOK) {
        LOGE("strncpy_s failed");
        return false;
    }

    return true;
}

bool DmDevice::Exists()
{
    if (!InitDmIoctl(false)) {
        return true;
    }
    if (ioctl(fd_, DM_DEV_STATUS, dm_) < 0) {
        if (errno == ENXIO || errno == ENODEV) {
            return false;
        }
        LOGE("DM_DEV_STATUS failed: %{public}s", strerror(errno));
        return true;
    }
    dev_ = dm_->dev;
    stage_ = Stage::ACTIVE;
    return true;
}

bool DmDevice::CreateDevice()
{
    if (!InitDmIoctl()) {
        return false;
    }
    if (ioctl(fd_, DM_DEV_CREATE, dm_) < 0) {
        LOGE("DM_DEV_CREATE failed: %{public}s", strerror(errno));
        return false;
    }
    dev_ = dm_->dev;

    stage_ = Stage::DEV_CREATED;
    return true;
}

bool DmDevice::LoadTable()
{
    struct dm_target_spec *target = reinterpret_cast<struct dm_target_spec *>(buffer_.get() + ioctlSize_);
    char *targetParams = reinterpret_cast<char *>(target) + sizeof(struct dm_target_spec);
    size_t paramsSize = totalSize_ - ioctlSize_ - sizeof(struct dm_target_spec);

    if (!InitDmIoctl()) {  // 重新初始化内存（因为上一次 ioctl 会修改 dm 结构体的内容）
        return false;
    }
    target->status = 0;
    target->sector_start = 0;
    target->length = sectors_;
    target->next = 0;
    if (strncpy_s(target->target_type, sizeof(target->target_type), "linear", sizeof(target->target_type) - 1) != EOK) {
        LOGE("strncpy_s failed");
        return false;
    }
    if (strcpy_s(targetParams, paramsSize, params_.c_str()) != EOK) {
        LOGE("strcpy_s failed");
        return false;
    }

    if (ioctl(fd_, DM_TABLE_LOAD, dm_) < 0) {
        LOGE("DM_TABLE_LOAD failed: %{public}s", strerror(errno));
        return false;
    }

    stage_ = Stage::TABLE_LOADED;
    return true;
}

bool DmDevice::ResumeDevice()
{
    if (!InitDmIoctl(false)) {
        return false;
    }
    if (ioctl(fd_, DM_DEV_SUSPEND, dm_) < 0) {  // dm->flags=0 时 DM_DEV_SUSPEND 执行激活否则执行挂起
        LOGE("DM_DEV_SUSPEND(resume) failed: %{public}s", strerror(errno));
        return false;
    }

    stage_ = Stage::ACTIVE;
    return true;
}

bool DmDevice::RemoveDevice()
{
    if (stage_ == Stage::NONE) {
        return true;
    }
    if (!InitDmIoctl(false)) {
        LOGE("InitDmIoctl failed, cannot remove DM device");
        return false;
    }

    if (ioctl(fd_, DM_DEV_REMOVE, dm_) < 0) {
        LOGE("DM_DEV_REMOVE failed: %{public}s", strerror(errno));
        return false;
    }
    stage_ = Stage::NONE;
    return true;
}
}  // namespace StorageDaemon
}  // namespace OHOS
