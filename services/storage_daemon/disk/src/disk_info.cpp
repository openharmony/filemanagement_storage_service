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

#include <sys/sysmacros.h>

#include "disk/disk_info.h"
#include "storage_service_log.h"
#include "utils/string_utils.h"

namespace OHOS {
namespace StorageDaemon {

DiskInfo::DiskInfo(std::string &diskName, std::string &sysPath, std::string &devPath, dev_t device, int diskType)
{
    diskId_ = StringPrintf("disk-%d-%d", major(device), minor(device));
    diskName_ = diskName;
    sysPath_ = sysPath;
    devPath_ = StringPrintf("/dev/block/%s", diskId_.c_str());
    device_ = device;
    diskType_ = static_cast<DiskType>(diskType);
}

dev_t DiskInfo::GetDevice() const
{
    return device_;
}

std::string DiskInfo::GetDiskId() const
{
    return diskId_;
}

std::string DiskInfo::GetDevPath() const
{
    return devPath_;
}

std::string DiskInfo::GetSysPath() const
{
    return sysPath_;
}

int32_t DiskInfo::GetDiskType() const
{
    return diskType_;
}

std::string DiskInfo::GetDiskName() const
{
    return diskName_;
}

DiskInfo::~DiskInfo() {}

} // namespace STORAGE_DAEMON
} // namespace OHOS
