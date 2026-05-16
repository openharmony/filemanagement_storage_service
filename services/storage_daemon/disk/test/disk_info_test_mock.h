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

#ifndef DISK_INFO_TEST_MOCK_H
#define DISK_INFO_TEST_MOCK_H

#include <gmock/gmock.h>

#include "disk/disk_info.h"
#include "partition_params.h"
#include "partition_table_info.h"

namespace OHOS {
namespace StorageManager {
class FormatParams;
}

namespace StorageDaemon {
class DiskInfoTestMock : public DiskInfo {
public:
    DiskInfoTestMock(std::string &diskName, std::string &sysPath, std::string &devPath, dev_t device, int flags)
        :DiskInfo(diskName, sysPath, devPath, device, flags) {};
    ~DiskInfoTestMock() {};
    MOCK_METHOD0(Create, int(void));
    MOCK_METHOD0(Destroy, int(void));
    MOCK_METHOD0(ReadPartition, int(void));
    MOCK_METHOD1(CreateVolume, int(dev_t));
    MOCK_METHOD0(Partition, int(void));
    MOCK_METHOD0(GetDevice, dev_t(void));
    MOCK_METHOD0(GetDiskId, std::string(void));
    MOCK_METHOD0(GetDevPath, std::string(void));
    MOCK_METHOD0(GetTotalSize, uint64_t(void));
    MOCK_METHOD0(GetSysPath, std::string(void));
    MOCK_METHOD0(GetDevVendor, std::string(void));
    MOCK_METHOD0(GetDiskType, int(void));

    // New methods for PR #2812
    MOCK_METHOD1(CreatePartition, int32_t(const OHOS::StorageManager::PartitionParams&));
    MOCK_METHOD1(DeletePartition, int32_t(uint32_t));
    MOCK_METHOD2(FormatPartition, int32_t(uint32_t, const OHOS::StorageManager::FormatParams&));
    MOCK_METHOD1(GetPartitionTable, int32_t(OHOS::StorageManager::PartitionTableInfo&));
};
}
}
#endif