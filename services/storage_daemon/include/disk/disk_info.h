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

namespace OHOS {
namespace StorageDaemon {
class DiskInfo {
public:
    enum DeviceFlag {
        SD_FLAG = 1,
        USB_FLAG = 2,
        CD_FLAG = 3,
    };
    enum class Table {
        UNKNOWN,
        MBR,
        GPT,
    };

    DiskInfo(std::string &sysPath_, std::string &devPath_, dev_t device, int flag);
    virtual ~DiskInfo();
    int Create();
    int Destroy();
    void ReadMetadata();
    int ReadPartition(const std::string &ejectStatus = "");
    int ReadPartitionCD(const std::string &ejectStatus);
    int ReadPartitionUSB();
    int CreateVolume(dev_t dev);
    int Partition();
    dev_t GetDevice() const;
    std::string GetId() const;
    std::string GetDevPath() const;
    uint64_t GetDevDSize() const;
    std::string GetSysPath() const;
    std::string GetDevVendor() const;
    int GetDevFlag() const;

private:
    std::string id_;
    uint64_t size_ {};
    /* device vendor infomation */
    std::string vendor_;
    std::string sysPath_;
    int status;
    bool isUserdata;
    std::string eventPath_;
    std::string devPath_;
    dev_t device_ {};
    unsigned int flags_ {};
    std::list<std::string> volumeId_;
    std::vector<std::string> sgdiskLines_;
    std::map<uint32_t, std::string> vendorMap_;
    int32_t ReadDiskLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata);
    bool CreateMBRVolume(int32_t type, dev_t dev);
    int32_t CreateUnknownTabVol();
    dev_t ProcessPartition(std::vector<std::string>::iterator &it, int32_t maxVols, bool isUserdata);
    int32_t GetMaxMinor(int32_t major);
    void CreateTableVolume(std::vector<std::string>::iterator &it, const std::vector<std::string>::iterator &end,
                           Table table, bool &foundPart, dev_t partitionDev);
    void UmountLines(std::vector<std::string> lines, int32_t maxVols, bool isUserdata);
    void ProcessPartitionChanges(const std::vector<std::string>& lines, int maxVolumes, bool isUserdata);
    bool ParseAndValidateManfid(const std::string& str, uint32_t& manfid);
    void FilterOutput(std::vector<std::string> &lines, std::vector<std::string> &output);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_INFO_H