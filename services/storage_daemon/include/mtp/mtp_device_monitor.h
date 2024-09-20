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
#ifndef OHOS_STORAGE_DAEMON_MTP_DEVICE_MONITOR_H
#define OHOS_STORAGE_DAEMON_MTP_DEVICE_MONITOR_H
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include "libudev.h"
#include "singleton.h"
#include "simple-mtpfs-fuse.h"
#include "simple-mtpfs-util.h"
namespace OHOS {
namespace StorageDaemon {
class MtpMonitor  : public Singleton<MtpMonitor> {
public:
    struct UdevInfo {
        std::string id;
        std::string action;
        std::string major;
        std::string minor;
        std::string idMtpDevice;
        std::string devlinks;
        std::string path;
    };

    static bool Monitor();
   
    static std::vector<MtpMonitor::UdevInfo> addMtpDevices;
    static void GetUdevInfoList(std::vector<MtpMonitor::UdevInfo> &infos);
private:
    static bool udevExit;
    const static std::string MTP_ROOT_PATH;

    static std::vector<MtpMonitor::UdevInfo> umountFailDevices;
    static void SigHandler(int signum);
    static void MonitorDevice(struct udev &udev);
    static bool ContainMtpInfo(const std::string &devlinks, const std::vector<MtpMonitor::UdevInfo> &mtpInfos);
    static void GetUdevInfos(struct udev_device &device,
                             std::vector<MtpMonitor::UdevInfo> &mtpInfos,
                             std::vector<MtpMonitor::UdevInfo> &removeInfos);
    static bool MtpDeviceControl(const std::vector<MtpMonitor::UdevInfo> &mtpDevices,
                                 const std::vector<MtpMonitor::UdevInfo> &removeDevices);
    static bool MountMtpDevice(const std::vector<MtpMonitor::UdevInfo> &udevInfos);
    static bool UnMountMtpDevice(const std::vector<MtpMonitor::UdevInfo> &udevInfos);
    static bool HadMount(const UdevInfo &info, const std::vector<MtpMonitor::UdevInfo> &infos);
    static bool UmountDevices();
    static void MonitorMtp(bool &success);
};
} // namespace StorageDaemon
} // namespace OHOS
#endif