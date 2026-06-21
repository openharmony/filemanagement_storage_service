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
#include <map>

namespace OHOS {
namespace StorageDaemon {

/**
 * @brief Burn options structure for optical disc burning operations
 */
struct BurnOptions {
    std::string diskName;        // 光盘卷标名称
    std::string burnPath;        // 刻录源路径（文件或目录）
    std::string fsType;          // 文件系统类型（ISO9660, UDF等）
    uint32_t burnSpeed;          // 刻录速度
    bool isIsoImage;             // 是否为ISO镜像文件
    bool isIncBurnSupport;       // 是否支持增量刻录
    
    BurnOptions() : burnSpeed(0), isIsoImage(false), isIncBurnSupport(false) {}
};

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
                             const std::string& partitionType);
    static int32_t GetPartitionTableInfo(const std::string &devPath, std::string &execRet);
    static int32_t CreatePartition(const std::string &devPath, int32_t partitionNum, int64_t startSector,
                                   int64_t endSector, const std::string &typeCode);
    static int32_t DeletePartitionInfo(const std::string &devPath, const std::string &diskId, int32_t partitionNum);
    static int32_t FormatPartition(const std::string &devPath, const std::string &fsType,
                                   const std::string &volumeName, bool quickFormat);
    static std::vector<std::string> GetFormatCMD(const std::string &fsType, const std::string &devPath,
                                                 const std::string &volName);
    static std::string DiskPathToVolPath(const std::string& diskPath);
    static int32_t PartitionHmfs(const std::string& diskPath);
    static int32_t QueryCDStatus(const std::string &devPath, int32_t &status);
    static int32_t EjectCD(const std::string &devPath);
    static int32_t Erase(const std::string &devPath);
    static int32_t Eject(const std::string &devName);
    static int32_t GetVolumeOpProcess(const std::string &volId, int32_t &progressPct);
    static int32_t VerifyBurnData(const std::string &devPath, int32_t verifyType);
    static int32_t GetCapacity(const std::string& devPath, int64_t &totalSize, int64_t &freeSize);
    static int32_t GetDiscCapacity(int cmdFd, const std::string& discType,
                                   int64_t &totalSize, int64_t &usedSize);
    static void AdjustBlankDiscCapacity(const std::string& devPath, const std::string& discType,
                                        int64_t &totalSize, int64_t &usedSize);
    static int32_t ExecAsyncDamagePartition(const std::string &devPath, int32_t partitionNum);
    static int32_t CleanTempDirectory();
};

int ExecuteScsiCmd(int fd, uint8_t *cdb, int cdbLen, uint8_t *dxferp, int dxferLen);
int ReadCDDiscInfo(const std::string &diskPath, int32_t cmdIndex, uint8_t *buf, int len);
int GetCDDiskStatus(const char *device, int &status);
int IsCDExist(const std::string &diskPath, bool &isCDExist);
int IsCDBlank(const std::string &diskPath, bool &isCDBlank);
int32_t ParseBurnOptions(const std::string &burnOptions, BurnOptions &parsedOptions);
int32_t ValidateBurnOptions(const BurnOptions &options);
std::string GetLastNumberSimple(const std::vector<std::string>& lines);
int32_t GetIncBurnAddr(const std::string &devPath, std::string &incBurnAddr);

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_EXTERNAL_DISK_UTILS_H
