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

#include <nocopyable.h>
#include <singleton.h>
#include "mtp/mtp_device_manager.h"

namespace OHOS {
namespace StorageDaemon {
class MtpDeviceMonitor : public NoCopyable  {
public:
    static MtpDeviceMonitor &GetInstance()
    {
        static MtpDeviceMonitor instance;
        return instance;
    }

    void StartMonitor();
    void UmountDetachedMtpDevice(uint8_t devNum, uint32_t busLoc);
    int32_t Mount(const std::string &id);
    int32_t Umount(const std::string &id);
    void MountMtpDeviceByBroadcast();
    
    void RegisterMTPParamListener();
    void RemoveMTPParamListener();
    static void OnMtpDisableParamChange(const char *key, const  char *value, void *context);
    static void OnEnterpriseParamChange(const char *key, const  char *value, void *context);
private:
    MtpDeviceMonitor();
    ~MtpDeviceMonitor();
    void MonitorDevice();
    void UmountAllMtpDevice();
    bool HasMounted(const MtpDeviceInfo &device);
    bool IsNeedDisableMtp();
    bool IsHwitDevice();
    int32_t HasMTPDevice(bool &hasMtp);
    int32_t MountMtpDevice(const std::vector<MtpDeviceInfo> &monitorDevices);
    int32_t GetMtpDevices(std::vector<MtpDeviceInfo> &devInfos);
    int32_t GetGphotoDevices(std::vector<MtpDeviceInfo> &devInfos);

private:
    std::mutex listMutex_;
    std::vector<MtpDeviceInfo> lastestMtpDevList_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_MTP_DEVICE_MONITOR_H