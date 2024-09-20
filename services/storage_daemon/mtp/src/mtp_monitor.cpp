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
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <thread>
#include "mtp/mtp_monitor.h"
#include "mtp/mtp_brocast.h"
#include "mtp/mtp_device_manager.h"
#include "mtp/mtp_manager_service.h"
#include "storage_service_log.h"
#include "utils/cmd_utils.h"
#include "utils/file_utils.h"
using namespace std;
namespace OHOS {
namespace StorageDaemon {
bool MtpMonitor::udevExit = false;
vector<MtpMonitor::UdevInfo> MtpMonitor::addMtpDevices;
vector<MtpMonitor::UdevInfo> MtpMonitor::umountFailDevices;

const string MtpMonitor::MTP_ROOT_PATH = "/mnt/external/";
void MtpMonitor::SigHandler(int signum)
{
    if (signum == SIGTERM) {
        udevExit = true;
        cout << "signum == SIGTERM" << endl;
    }
}

bool MtpMonitor::ContainMtpInfo(const string &devlinks, const vector<MtpMonitor::UdevInfo> &mtpInfos)
{
    for (auto iter = mtpInfos.begin(); iter != mtpInfos.end(); iter++) {
        if (iter->devlinks.compare(devlinks)) {
            return true;
        }
    }
    return false;
}

void MtpMonitor::GetUdevInfos(struct udev_device &device,
                              vector<MtpMonitor::UdevInfo> &mtpInfos,
                              vector<MtpMonitor::UdevInfo> &removeInfos)
{
    string action;
    string major;
    string minor;
    string idMtpDevice;
    string devlinks;
    string uuid;
    bool isAdd = false;
    bool isRemove = false;
    bool isAddMtpDevice = false;
    struct udev_list_entry *list_entry;
    udev_list_entry_foreach(list_entry, udev_device_get_properties_list_entry(&device))
    {
        string keyName = udev_list_entry_get_name(list_entry);
        string keyValue = udev_list_entry_get_value(list_entry);
        if (keyName.compare("ACTION") == 0) {
            action = udev_list_entry_get_value(list_entry);
            if (action.compare("add") == 0) {
                isAdd = true;
            } else if (action.compare("remove") == 0) {
                isRemove = true;
            }
            string cmd = "cat /proc/sys/kernel/random/uuid";
            vector<string> uuids;
            CmdUtils::GetInstance().RunCmd(cmd, uuids);
            uuid = uuids.front();
        } else if (keyName.compare("ID_MTP_DEVICE") == 0) {
            idMtpDevice = keyValue;
            isAddMtpDevice = true;
        } else if (keyName.compare("MAJOR") == 0) {
            major = keyValue;
        } else if (keyName.compare("MINOR") == 0) {
            minor = keyValue;
        } else if (keyName.compare("DEVLINKS") == 0) {
            devlinks = keyValue;
        }
    }
    UdevInfo udevInfo;
    if (isAdd || isRemove) {
        udevInfo.id = uuid;
        udevInfo.action = action;
        udevInfo.major = major;
        udevInfo.minor = minor;
        udevInfo.devlinks = devlinks;
        udevInfo.idMtpDevice = idMtpDevice;
    }
    if (isAddMtpDevice && isAdd) {
        if (!ContainMtpInfo(devlinks, mtpInfos)) {
            mtpInfos.push_back(udevInfo);
        }
    } else if (!isAddMtpDevice && isRemove) {
        if (!major.empty() && !minor.empty()) {
            removeInfos.push_back(udevInfo);
        }
    }
}

bool MtpMonitor::MtpDeviceControl(const std::vector<MtpMonitor::UdevInfo> &mtpDevices,
                                  const std::vector<MtpMonitor::UdevInfo> &removeDevices)
{
    if (mtpDevices.size() > 0) {
        return MountMtpDevice(mtpDevices);
    } else {
        return UnMountMtpDevice(removeDevices);
    }
}

bool MtpMonitor::HadMount(const UdevInfo &info, const std::vector<MtpMonitor::UdevInfo> &infos)
{
    for (auto iter = infos.begin(); iter != infos.end(); iter++) {
        if (iter->major.compare(info.major) == 0 && iter->minor.compare(info.minor) == 0) {
            return true;
        }
    }
    return false;
}

bool MtpMonitor::MountMtpDevice(const std::vector<MtpMonitor::UdevInfo> &udevInfos)
{
    bool success = false;
    LOGI("MountMtpDevice udevInfos.size:%{public}lu", udevInfos.size());
    for (auto udevInfo : udevInfos) {
        string major = udevInfo.major;
        string minor = udevInfo.minor;
        if (addMtpDevices.size() == 0 || !HadMount(udevInfo, addMtpDevices)) {
            string devlinks = udevInfo.devlinks;
            string id = udevInfo.id;
            LOGI("MountMtpDevice devlinks:%{public}s", devlinks.c_str());
            string path = MTP_ROOT_PATH + "mtp-" + udevInfo.major + "-" + udevInfo.minor;
            success = MtpDeviceManager::GetInstance().MountMtp(id, devlinks, path);
            if (success) {
                udevInfo.path = path;
                addMtpDevices.push_back(udevInfo);
            } else {
                LOGE("MountMtpDevice fail");
            }
        }
    }
    return success;
}

bool MtpMonitor::UnMountMtpDevice(const std::vector<MtpMonitor::UdevInfo> &udevInfos)
{
    LOGI("mtp UnMountMtpDevice udevInfos.size:%{public}lu", udevInfos.size());
    bool success = false;
    if (addMtpDevices.size() <= 0) {
        return true;
    }
    for (auto iter = udevInfos.begin(); iter != udevInfos.end(); iter++) {
        for (auto addIter = addMtpDevices.begin(); addIter != addMtpDevices.end(); addIter++) {
            if (addIter->major.compare(iter->major) == 0 && addIter->minor.compare(iter->minor) == 0) {
                string path = MTP_ROOT_PATH + "mtp-" + iter->major + "-" + iter->minor;
                string id = iter->id;
                success = MtpDeviceManager::GetInstance().UnMountMtp(id, path);
                addMtpDevices.erase(addIter);
                if (success) {
                    for (auto umountIter = umountFailDevices.begin(); umountIter != umountFailDevices.end();
                         umountIter++) {
                        if (umountIter->devlinks.compare(iter->devlinks)) {
                            umountFailDevices.erase(umountIter);
                            break;
                        }
                    }
                } else {
                    umountFailDevices.push_back(*iter);
                }
                break;
            }
        }
    }
    return false;
}

void MtpMonitor::MonitorDevice(struct udev &udev)
{
    struct sigaction act;
    act.sa_handler = 0;
    act.sa_flags = 0;
    fd_set readfds;
    struct udev_monitor *kernelMonitor = nullptr;
    if (getuid() != 0) {
        cout << "root privileges needed to subscribe to kernel events:" << stderr << endl;
        udev_monitor_unref(kernelMonitor);
        return;
    } else {
        act.sa_handler = SigHandler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_RESTART;
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGTERM, &act, NULL);
        kernelMonitor = udev_monitor_new_from_netlink(&udev, "udev");
        if (kernelMonitor == nullptr) {
            udev_monitor_unref(kernelMonitor);
            return;
        }
        if (udev_monitor_enable_receiving(kernelMonitor) < 0) {
            udev_monitor_unref(kernelMonitor);
            return;
        }
        while (!udevExit) {
            vector<MtpMonitor::UdevInfo> mtpDevices;
            vector<MtpMonitor::UdevInfo> removeDevices;
            int fdCount;
            FD_ZERO(&readfds);
            if (kernelMonitor != nullptr) {
                FD_SET(udev_monitor_get_fd(kernelMonitor), &readfds);
            }
            fdCount = select(udev_monitor_get_fd(kernelMonitor) + 1, &readfds, NULL, NULL, NULL);
            if (fdCount < 0) {
                if (errno != EINTR) {
                    cout << "error receiving uevent message:" << stderr << endl;
                }
                continue;
            }
            if ((kernelMonitor != nullptr) && FD_ISSET(udev_monitor_get_fd(kernelMonitor), &readfds)) {
                struct udev_device *device;
                device = udev_monitor_receive_device(kernelMonitor);
                if (device == nullptr) {
                    continue;
                }
                GetUdevInfos(*device, mtpDevices, removeDevices);
                MtpDeviceControl(mtpDevices, removeDevices);
                udev_device_unref(device);
            }
        }
    }
}

void MtpMonitor::GetUdevInfoList(std::vector<MtpMonitor::UdevInfo> &infos)
{
    infos.assign(addMtpDevices.begin(), addMtpDevices.end());
}

void MtpMonitor::MonitorMtp(bool &success){
    struct udev *udev = udev_new();
    if (udev != nullptr) {
        MonitorDevice(*udev);
        success = true;
    }
    udev_unref(udev);
    LOGI("Mtp Monitor end");
}

bool MtpMonitor::Monitor()
{
    LOGI("Mtp Monitor begin");
    bool success = false;
    MtpDeviceManager::GetInstance().UnMountMtpAll(MTP_ROOT_PATH);
    std::thread t(MonitorMtp,ref(success));
    t.detach();
    return success;
}
} // namespace StorageDaemon
} // namespace OHOS