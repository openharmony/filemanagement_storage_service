/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
 
#ifndef OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_EXT_H
#define OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_EXT_H
 
#include <mutex>
#include "volume_external.h"

namespace OHOS {
namespace StorageManager {
 
class FuseExtInterface {
public:
    virtual ~FuseExtInterface() = default;
    virtual bool NotifyExternalVolumeFuseMount(int fd, std::string volumeId, std::string fsUuid) = 0;
    virtual bool NotifyExternalVolumeFuseUMount(std::string volumeId) = 0;
    virtual bool IsUsbFuseByType(std::string fsType, bool enabled) = 0;
};
 
class VolumeManagerServiceExt {
public:
    static VolumeManagerServiceExt &GetInstance(void)
    {
        static VolumeManagerServiceExt instance;
        return instance;
    }
 
    int32_t NotifyUsbFuseMount(int fuseFd, const std::string &volumeId, const std::string &fsUuid);
    int32_t NotifyUsbFuseUmount(const std::string &volumeId);
    bool IsUsbFuseByType(const std::string &fsType);
    
private:
    VolumeManagerServiceExt();
    ~VolumeManagerServiceExt();
    VolumeManagerServiceExt(const VolumeManagerServiceExt &) = delete;
    VolumeManagerServiceExt &operator=(const VolumeManagerServiceExt &) = delete;
    VolumeManagerServiceExt(const VolumeManagerServiceExt &&) = delete;
    VolumeManagerServiceExt &operator=(const VolumeManagerServiceExt &&) = delete;
 
    void Init();
    void UnInit();
    void *handler_ = nullptr;
};
} // StorageManager
} // OHOS
 
#endif // OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_EXT_H