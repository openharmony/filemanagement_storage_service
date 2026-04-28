/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_EXTERNAL_DISK_UTILS_H
#define OHOS_STORAGE_DAEMON_EXTERNAL_DISK_UTILS_H

#include <string>
#include <vector>

namespace OHOS {
namespace StorageDaemon {

/**
 * @brief Disk-level utility operations for external storage management
 *
 * Provides static methods for block device node management and disk partition
 * operations. These operations require root privileges.
 */
class DiskUtils {
public:
    DiskUtils() = delete;

    static int32_t CreateBlockDeviceNode(const std::string& devPath,
                                         uint32_t mode,
                                         int32_t major,
                                         int32_t minor);
    static int32_t DestroyBlockDeviceNode(const std::string& devPath);
    static int32_t ReadPartitionTable(const std::string& devPath,
                                      std::string& output,
                                      int32_t& maxVolume);
    static int32_t Partition(const std::string& diskPath,
                             int32_t partitionType,
                             uint32_t partitionFlags);
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_EXTERNAL_DISK_UTILS_H
