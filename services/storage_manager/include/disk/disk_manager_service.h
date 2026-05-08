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

#include <map>
#include <mutex>
#include <singleton.h>
#include "disk.h"
#include "partition_options.h"
#include "partition_table_info.h"

namespace OHOS {
namespace StorageManager {
class DiskManagerService final : public NoCopyable {
public:
    static DiskManagerService &GetInstance();
    std::shared_ptr<Disk> GetDiskById(const std::string &diskId);
    int32_t Partition(const std::string &diskId, int32_t type);
    void OnDiskCreated(const Disk &disk);
    void OnDiskDestroyed(const std::string &diskId);
    std::vector<Disk> GetAllDisks();
    int32_t GetDiskById(const std::string &diskId, Disk &disk);
    int32_t GetPartitionTable(const std::string &diskId, PartitionTableInfo &partitionTableInfo);
    int32_t CreatePartition(const std::string &diskId, PartitionOptions &partitionOption);
    int32_t DeletePartition(const std::string &diskId, uint32_t partitionNum);
private:
    DiskManagerService();
    ~DiskManagerService();
    std::map<std::string, std::shared_ptr<Disk>> diskMap_;
    std::mutex diskMapMutex_;
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_DISK_MANAGER_SERVICE_H