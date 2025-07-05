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
 
#include "volume/volume_manager_service_ext.h"
 
#include <dlfcn.h>
#include "disk.h"
#include "disk/disk_manager_service.h"
#include "safe_map.h"
#include "securec.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include <sys/xattr.h>
#include "utils/storage_radar.h"
#include "utils/storage_utils.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"
#include "volume/notification.h"
 
using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageManager {
 
namespace {
typedef int32_t (*FuncMount)(int, const std::string&, const std::string&);
typedef int32_t (*FuncUMount)(const std::string&);
}
 
VolumeManagerServiceExt::VolumeManagerServiceExt()
{
    Init();
    LOGI("Instance created.");
}
 
VolumeManagerServiceExt::~VolumeManagerServiceExt()
{
    UnInit();
    LOGI("Instance destroyed.");
}
 
void VolumeManagerServiceExt::Init()
{
    handler_ = dlopen("/system/lib64/libspace_ability_fuse_ext.z.so", RTLD_LAZY);
    if (handler_ == nullptr) {
        LOGE("Policy not exist, just start service.");
        return;
    }
}
 
void VolumeManagerServiceExt::UnInit()
{
    LOGI("UnInit start");
    if (handler_) {
        dlclose(handler_);
        handler_ = nullptr;
    }
}
 
int32_t VolumeManagerServiceExt::NotifyUsbFuseMount(int fuseFd, std::string volumeid, std::string fsUuid)
{
    FuncMount funcMount = (FuncMount)dlsym(handler_, "NotifyExternalVolumeFuseMount");
    if (funcMount == nullptr) {
        LOGE("Failed to get function pointer for NotifyExternalVolumeFuseMount");
        return E_MOUNT_CLOUD_FUSE;
    }
    if (funcMount(fuseFd, volumeid, fsUuid) != E_OK) {
        LOGE("NotifyUsbFuseMount fail");
        return E_MOUNT_CLOUD_FUSE;
    }
    return E_OK;
}
 
int32_t VolumeManagerServiceExt::NotifyUsbFuseUMount(std::string volumeid)
{
    LOGI("NotifyUsbFuseUMount in");
    FuncUMount funcUMount = (FuncUMount)dlsym(handler_, "NotifyExternalVolumeFuseUmount");
    if (funcUMount(volumeid) != E_OK) {
        LOGE("NotifyUsbFuseUMount fail");
        return E_MOUNT_CLOUD_FUSE;
    }
    return E_OK;
}
} // StorageManager
} // OHOS