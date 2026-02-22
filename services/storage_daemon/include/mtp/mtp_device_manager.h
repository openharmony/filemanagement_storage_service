/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_STORAGE_DAEMON_MTP_DEVICE_MANAGER_H
#define OHOS_STORAGE_DAEMON_MTP_DEVICE_MANAGER_H

#include <nocopyable.h>
#include <singleton.h>
#include <string>
#include <sys/types.h>
#include <vector>

namespace OHOS {
namespace StorageDaemon {

struct MtpDeviceInfo {
    std::string id;
    std::string uuid;
    std::string path;
    std::string vendor;
    std::string product;
    std::string type;
    uint8_t devNum;
    uint32_t busLocation;
    uint16_t vendorId;
    uint16_t productId;
};

class MtpDeviceManager : public NoCopyable  {
public:
    static MtpDeviceManager &GetInstance()
    {
        static MtpDeviceManager instance;
        return instance;
    }

    int32_t MountDevice(const MtpDeviceInfo &device);
    int32_t UmountDevice(const MtpDeviceInfo &device, bool needNotify, bool isBadRemove);
    int32_t PrepareMtpMountPath(const std::string &path);

private:
    MtpDeviceManager();
    ~MtpDeviceManager();

    bool isMounting = false;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif