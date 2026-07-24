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

#ifndef OHOS_STORAGE_DAEMON_DISK_INFO_H
#define OHOS_STORAGE_DAEMON_DISK_INFO_H

#include <list>
#include <string>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
class DiskInfo {
public:
    enum DiskType {
        SD_CARD = 1,
        USB_FLASH = 2,
        CD_DVD_BD = 3,
        DATA_DISK_SSD = 4,
        DATA_DISK_HDD = 5,
        UNKNOWN_DISK_TYPE = 255,
    };

    DiskInfo(std::string &diskName, std::string &sysPath_, std::string &devPath_, dev_t device, int diskType);
    virtual ~DiskInfo();
    dev_t GetDevice() const;
    std::string GetDiskId() const;
    std::string GetDevPath() const;
    std::string GetSysPath() const;
    int32_t GetDiskType() const;
    std::string GetDiskName() const;

private:
    std::string diskId_;
    std::string diskName_;
    std::string sysPath_;
    std::string devPath_;
    dev_t device_ {};
    DiskType diskType_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_INFO_H
