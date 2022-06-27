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

#ifndef OHOS_STORAGE_MANAGER_DISK_MANAGER_SERVICE_H
#define OHOS_STORAGE_MANAGER_DISK_MANAGER_SERVICE_H

#include <unordered_map>
#include <singleton.h>
#include <nocopyable.h>
#include "disk.h"
#include "utils/storage_rl_map.h"

namespace OHOS {
namespace StorageManager {
class DiskManagerService final : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(DiskManagerService);
public:
    std::shared_ptr<Disk> GetDiskById(std::string diskId);
    int32_t Partition(std::string diskId, int32_t type);
    void OnDiskCreated(Disk disk);
    void OnDiskDestroyed(std::string diskId);
    std::vector<Disk> GetAllDisks();
    int32_t GetDiskById(std::string diskId, Disk &disk);
private:
    StorageRlMap<std::string, std::shared_ptr<Disk>> diskMap_;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_DISK_MANAGER_SERVICE_H