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
#include <map>
#include <string>
#include <sys/types.h>

#include "partition_params.h"
#include "partition_table_info.h"
#include "format_params.h"

namespace OHOS {
namespace StorageDaemon {
class DiskInfo {
public:
    enum DiskType {
        SD_CARD = 1,
        USB_FLASH = 2,
        CD_DVD_BD = 3,
        MTP_PTP = 4,
        UNKNOWN_DISK_TYPE = 255,
    };

    enum class Table {
        UNKNOWN,
        MBR,
        GPT,
    };
    enum DiskState {
        MOUNTED,
        REMOVED,
    };

    enum MediaType {
        SSD = 0,
        HDD = 1,
        UNKNOWN_MEDIA_TYPE = 2,
    };

    DiskInfo(std::string &diskName, std::string &sysPath_, std::string &devPath_, dev_t device, int diskType);
    virtual ~DiskInfo();
    int Create();
    int Destroy();
    void ReadMetadata();
    int ReadPartition(const std::string &ejectStatus = "");
    int ReadPartitionCD(const std::string &ejectStatus);
    int ReadPartitionUSB();
    int CreateVolume(dev_t dev, uint32_t partitionNum);
    int Partition();
    int32_t GetPartitionTable(OHOS::StorageManager::PartitionTableInfo &partitionTableInfo);
int32_t CreatePartition(const OHOS::StorageManager::PartitionParams &partitionParams);
    bool IsOptionsValid(const OHOS::StorageManager::PartitionParams &partitionParams);
    int32_t ExecAsyncCreatePartition(const OHOS::StorageManager::PartitionParams &partitionParams);
    int32_t ExecAsyncDeletePartition(uint32_t partitionNum);
    int32_t ExecAsyncFormatPartition(uint32_t partitionNum, const OHOS::StorageManager::FormatParams &formatParams);
    std::vector<std::string> GetFormatCMD(const std::string &fsType, const std::string &devPath,
                                          const std::string &volName);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_INFO_H