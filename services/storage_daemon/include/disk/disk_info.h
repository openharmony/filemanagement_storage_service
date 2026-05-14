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

#include "partition_options.h"
#include "partition_table_info.h"
#include "format_options.h"

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
    int32_t CreatePartition(const OHOS::StorageManager::PartitionOptions &partitionOption);
    int32_t DeletePartition(uint32_t partitionNum);
    int32_t FormatPartition(uint32_t partitionNum, const OHOS::StorageManager::FormatOptions &options);
    dev_t GetDevice() const;
    std::string GetDiskId() const;
    std::string GetDevPath() const;
    uint64_t GetTotalSize() const;
    std::string GetSysPath() const;
    std::string GetDevVendor() const;
    int32_t GetDiskType() const;
    int32_t GetMediaType() const;
    std::string GetDiskName() const;
    bool GetRemovable() const;
    std::string GetExtraInfo() const;
    int EjectDisk();

private:
    std::string diskId_;
    std::string diskName_;
    uint64_t totalSize_ {};
    /* device vendor infomation */
    std::string vendor_;
    std::string sysPath_;
    int status;
    bool isUserdata;
    std::string eventPath_;
    std::string devPath_;
    dev_t device_ {};
    std::list<std::string> volumeId_;
    std::vector<std::string> sgdiskLines_;
    std::map<uint32_t, std::string> vendorMap_;
    MediaType mediaType_ = MediaType::UNKNOWN_MEDIA_TYPE;
    DiskType diskType_;
    uint64_t totalSector_;
    uint64_t lastUsableSector_;
    uint32_t sectorSize_;
    uint32_t alignSector_;
    bool removable_ = true;
    std::string extraInfo_;
    int32_t ReadDiskLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata);
    bool CreateMBRVolume(int32_t type, dev_t dev, uint32_t partitionNum);
    int32_t CreateUnknownTabVol();
    dev_t ProcessPartition(std::vector<std::string>::iterator &it, int32_t maxVols, bool isUserdata);
    int32_t GetMaxMinor(int32_t major);
    void CreateTableVolume(std::vector<std::string>::iterator &it, const std::vector<std::string>::iterator &end,
                           Table table, bool &foundPart, dev_t partitionDev);
    void UmountLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata);
    void ProcessPartitionChanges(const std::vector<std::string>& lines, int maxVolumes, bool isUserdata);
    bool ParseAndValidateManfid(const std::string& str, uint32_t& manfid);
    void FilterOutput(std::vector<std::string> &lines, std::vector<std::string> &output);
    bool ParsePartitionInfo(const std::string &context, OHOS::StorageManager::PartitionInfo &info);
    bool SetTotalSector(std::vector<std::string> &content);
    bool SetSectorSize(std::vector<std::string> &content);
    bool SetAlignSector(std::vector<std::string> &content);
    void SetPartitions(std::vector<std::string> &content, OHOS::StorageManager::PartitionTableInfo &partitionTableInfo);
    void SetTableType(std::vector<std::string> &content, OHOS::StorageManager::PartitionTableInfo &partitionTableInfo);
    bool IsPartitionNumExists(uint32_t partitionNum);
    int32_t ExecAsyncGetPartitionTable(std::vector<std::string> &output);
    bool SetUsableSector(std::vector<std::string> &content);
    bool IsOptionsValid(const OHOS::StorageManager::PartitionOptions &partitionOption);
    int32_t ExecAsyncCreatePartition(const OHOS::StorageManager::PartitionOptions &partitionOption);
    int32_t ExecAsyncDeletePartition(uint32_t partitionNum);
    int32_t ExecAsyncFormatPartition(uint32_t partitionNum, const OHOS::StorageManager::FormatOptions &options);
    std::vector<std::string> GetFormatCMD(const std::string &fsType, const std::string &devPath,
                                          const std::string &volName);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_INFO_H