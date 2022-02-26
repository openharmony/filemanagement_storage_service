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

#ifndef OHOS_STORAGE_DAEMON_DISK_MANAGER_H
#define OHOS_STORAGE_DAEMON_DISK_MANAGER_H

#include <list>
#include <memory>
#include <mutex>
#include <cstring>
#include <nocopyable.h>

#include <sys/types.h>

#include "disk/disk_config.h"
#include "disk/disk_info.h"
#include "netlink/netlink_data.h"

namespace OHOS {
namespace StorageDaemon {
class DiskManager final {
public:
    static DiskManager* Instance(void);

    virtual ~DiskManager();
    void CreateDisk(std::shared_ptr<DiskInfo> &diskInfo);
    void DestroyDisk(dev_t device);
    void ChangeDisk(dev_t device);
    std::shared_ptr<DiskInfo> GetDisk(dev_t device);
    void HandleDiskEvent(NetlinkData *data);
    int32_t HandlePartition(std::string diskId);
    void AddDiskConfig(std::shared_ptr<DiskConfig> &diskConfig);
    void ReplayUevent();
    std::shared_ptr<DiskInfo> MatchConfig(NetlinkData *data);

private:
    DiskManager() = default;

    std::mutex lock_;
    std::list<std::shared_ptr<DiskInfo>> disk_;
    std::list<std::shared_ptr<DiskConfig>> diskConfig_;
    static DiskManager* instance_;

    DISALLOW_COPY_AND_MOVE(DiskManager);

    const std::string sysBlockPath_ = "/sys/block";
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_MANAGER_H
