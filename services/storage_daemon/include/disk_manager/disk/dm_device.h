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
#ifndef OHOS_STORAGE_DAEMON_DM_DEVICE_H
#define OHOS_STORAGE_DAEMON_DM_DEVICE_H

#include <unistd.h>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <linux/dm-ioctl.h>
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {

constexpr const char *DM_CONTROL = "/dev/mapper/control";

class DmDevice {
public:
    DmDevice(const std::string &sourceDevPath, uint64_t sourceStartSector, uint64_t sectorCount)
        : sourceDevPath_(sourceDevPath), dmName_(GetDmName(sourceDevPath)),
          sourceStartSector_(sourceStartSector), sectors_(sectorCount)
    {
        if (sourceDevPath_.empty() || sourceDevPath_.find(' ') != std::string::npos) {
            LOGE("invalid sourceDevPath: %{public}s", sourceDevPath_.c_str());
            return;
        }
        if (dmName_.empty()) {
            LOGE("dmName is empty");
            return;
        }
        uint64_t totalSectors = GetDeviceSectors();
        if (totalSectors == 0 || sectors_ == 0 || sourceStartSector_ >= totalSectors ||
            sectors_ > totalSectors - sourceStartSector_) {
            LOGE("invalid mapping: total=%{public}llu start=%{public}llu count=%{public}llu",
                 static_cast<unsigned long long>(totalSectors),
                 static_cast<unsigned long long>(sourceStartSector_),
                 static_cast<unsigned long long>(sectors_));
            return;
        }
        PrepareBuffer();
        fd_ = open(DM_CONTROL, O_RDWR);
        if (fd_ < 0) {
            LOGE("open dm control failed: %{public}s", strerror(errno));
            return;
        }
        paramsValid_ = true;
    }
    ~DmDevice()
    {
        if (stage_ > Stage::NONE && stage_ < Stage::ACTIVE) {
            RemoveDevice();
        }
        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }
    DmDevice(const DmDevice &) = delete;
    DmDevice &operator=(const DmDevice &) = delete;

    bool Create();
    dev_t GetDeviceDev();

private:
    const std::string sourceDevPath_;
    const std::string dmName_;
    const uint64_t sourceStartSector_;
    const uint64_t sectors_;

    enum class Stage { NONE, DEV_CREATED, TABLE_LOADED, ACTIVE };
    Stage stage_ = Stage::NONE;
    bool paramsValid_ = false;

    int fd_ = -1;
    dev_t dev_ = 0;
    std::unique_ptr<char[]> buffer_;
    struct dm_ioctl *dm_ = nullptr;
    size_t totalSize_ = 0;
    size_t ioctlSize_ = 0;
    std::string params_ = "";

    static std::string GetDmName(const std::string &devPath);
    uint64_t GetDeviceSectors();
    void PrepareBuffer();

    bool InitDmIoctl(bool hasTargets = true);
    bool Exists();
    bool CreateDevice();
    bool LoadTable();
    bool ResumeDevice();
    bool RemoveDevice();
};
}  // namespace StorageDaemon
}  // namespace OHOS
#endif  // OHOS_STORAGE_DAEMON_DM_DEVICE_H
