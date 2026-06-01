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

#ifndef OHOS_STORAGE_DAEMON_SCAN_DEVICE_H
#define OHOS_STORAGE_DAEMON_SCAN_DEVICE_H

#include <cstdint>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace OHOS {
namespace StorageDaemon {

struct BlockInfo {
    uint64_t sizeBytes;
    std::string vendor;
    std::string model;
    std::string interfaceType;
    uint32_t rpm;
    bool removable;
    std::string serialNumber;
    std::string diskId;
    std::string devicePath;
    std::string port;

    nlohmann::json ToJson() const;
    static std::string SerializeVector(const std::vector<BlockInfo>& infos);
};

class ScanDevice {
public:
    explicit ScanDevice(const std::string &sysBlockPath = "/sys/block", const std::string &devBlockPath = "/dev/block");
    ~ScanDevice() = default;

    std::vector<BlockInfo> GetDataDisks();
    std::vector<BlockInfo> GetExternalDisks(const std::string &devName, const std::string &diskId);

private:
    int GetBlockInfo(const std::string &deviceName, const bool isNvmeDevice, BlockInfo &blockInfo);
    bool ReadSysfsNode(const std::string &path, std::string &content);
    bool ReadRemovableNode(const std::string &deviceName, bool &isRemovable);
    bool IsDataDisk(const std::string &deviceName, const bool isNeedCheckUfs, const bool isRemovable);
    uint64_t GetDiskSize(const std::string &deviceName);
    std::string GetVendor(const std::string &deviceName);
    std::string GetModel(const std::string &deviceName);
    std::string GetInterfaceType(const std::string &deviceName);
    uint32_t GetDiskRpm(const std::string &deviceName, const bool isNvmeDevice);
    std::string GetSataSerialNumber(int fd);
    std::string GetNvmeSerialNumber(const std::string &deviceName);
    std::string GetSerialNumber(const std::string &deviceName, const bool isNvmeDevice);
    std::string GetPciePath(const std::string &deviceName);
    bool ReadSataDeviceNumber(const std::string &deviceName, std::string &major, std::string &minor);
    bool ReadNvmeDeviceNumber(const std::string &deviceName, std::string &major, std::string &minor);
    std::string GetDiskId(const std::string &deviceName, const bool isNvmeDevice);
    std::string GetDevicePath(const std::string &deviceName);
    std::string GetPort(const std::string &pciePath, const bool isNvmeDevice);
    bool IsValidNvmeDevice(const std::string &deviceName);
    bool ParseStringToUlongLong(const std::string &str, unsigned long long &result);
    bool GetExternalDiskSize(const std::string &path, uint64_t *size);

    std::string sysBlockPath;
    std::string devBlockPath;
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_SCAN_DEVICE_H
