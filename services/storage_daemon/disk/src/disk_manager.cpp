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

#include <cerrno>
#include <sys/sysmacros.h>
#include <cinttypes>
#include <cstdlib>

#include "disk/disk_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

DiskManager &DiskManager::Instance(void)
{
    static DiskManager instance;
    return instance;
}

DiskManager::~DiskManager()
{
    LOGI("[L2:DiskManager] ~DiskManager: >>> ENTER <<<");
}

std::shared_ptr<DiskInfo> DiskManager::MatchConfig(NetlinkData *data)
{
    LOGI("[L2:DiskManager] MatchConfig: >>> ENTER <<<");
    if (data == nullptr) {
        LOGE("[L2:DiskManager] MatchConfig: <<< EXIT FAILED <<< data is nullptr");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(lock_);
    std::string sysPath = data->GetSyspath();
    std::string devPath = data->GetDevpath();
    errno = 0;
    char *endptr = nullptr;
    std::string majorStr = data->GetParam("MAJOR");
    long majorLong = strtol(majorStr.c_str(), &endptr, 10);
    if (errno == ERANGE || endptr == majorStr.c_str() || *endptr != '\0' || majorLong < 0) {
        LOGE("[L2:DiskManager] MatchConfig: invalid MAJOR='%{public}s'", majorStr.c_str());
        return nullptr;
    }
    std::string minorStr = data->GetParam("MINOR");
    long minorLong = strtol(minorStr.c_str(), &endptr, 10);
    if (errno == ERANGE || endptr == minorStr.c_str() || *endptr != '\0' || minorLong < 0) {
        LOGE("[L2:DiskManager] MatchConfig: invalid MINOR='%{public}s'", minorStr.c_str());
        return nullptr;
    }
    unsigned int major = static_cast<unsigned int>(majorLong);
    unsigned int minor = static_cast<unsigned int>(minorLong);
    dev_t device = makedev(major, minor);

    for (auto config : diskConfig_) {
        if ((config != nullptr) && config->IsMatch(devPath)) {
            uint32_t flag = static_cast<uint32_t>(config->GetFlag());
            if (major == DISK_MMC_MAJOR) {
                flag |= DiskInfo::DiskType::SD_CARD;
            } else if (major == DISK_CD_MAJOR) {
                flag |= DiskInfo::DiskType::CD_DVD_BD;
            } else {
                flag |= DiskInfo::DiskType::USB_FLASH;
            }
            std::string diskName = data->GetDiskName();
            auto diskInfo =  std::make_shared<DiskInfo>(diskName, sysPath, devPath, device, static_cast<int>(flag));
            LOGI("[L2:DiskManager] MatchConfig: <<< EXIT SUCCESS <<< devPath=%{public}s, matched", devPath.c_str());
            return diskInfo;
        }
    }

    LOGI("[L2:DiskManager] MatchConfig: <<< EXIT SUCCESS <<< No matching configuration found");
    return nullptr;
}

void DiskManager::AddDiskConfig(std::shared_ptr<DiskConfig> &diskConfig)
{
    LOGI("[L2:DiskManager] AddDiskConfig: >>> ENTER <<<");
    std::lock_guard<std::mutex> lock(lock_);
    if (diskConfig != nullptr) {
        diskConfig_.push_back(diskConfig);
    }
    LOGI("[L2:DiskManager] AddDiskConfig: <<< EXIT SUCCESS <<<");
}

void DiskManager::ReplayUevent()
{
    LOGI("[L2:DiskManager] ReplayUevent: >>> ENTER <<<");
    TraverseDirUevent(sysBlockPath_, true);
    LOGI("[L2:DiskManager] ReplayUevent: <<< EXIT SUCCESS <<<");
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
