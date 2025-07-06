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
#include <singleton.h>
#include "volume_external.h"

namespace OHOS {
namespace StorageManager {
 
class FuseExtInterface {
public:
    virtual ~FuseExtInterface() = default;
    virtual bool NotifyExternalVolumeFuseMount(int fd, std::string volumeid, std::string fsUuid) = 0;
    virtual bool NotifyExternalVolumeFuseUMount(std::string volumeid) = 0;
};
 
class VolumeManagerServiceExt {
public:
    static VolumeManagerServiceExt *GetInstance(void)
    {
        static VolumeManagerServiceExt instance;
        return &instance;
    }
 
    int32_t NotifyUsbFuseMount(int fuseFd, std::string volumeid, std::string fsUuid);
    int32_t NotifyUsbFuseUMount(std::string volumeid);
 
private:
    VolumeManagerServiceExt();
    ~VolumeManagerServiceExt();
 
    void Init();
    void UnInit();
    void *handler_ = nullptr;
    std::mutex mutex_;
};
} // StorageManager
} // OHOS
 
#endif // OHOS_STORAGE_MANAGER_VOLUME_MANAGER_SERVICE_EXT_H