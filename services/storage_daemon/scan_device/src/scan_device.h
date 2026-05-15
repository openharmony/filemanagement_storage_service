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

using json = nlohmann::json;

enum class MediaType { SSD = 0, HDD = 1, UNKNOWN = 2 };

struct BlockInfo {
    uint64_t sizeBytes;
    std::string vendor;
    std::string model;
    std::string interfaceType;
    uint32_t rpm;
    std::string state;
    MediaType mediaType;
    bool removable;
    std::string serialNumber;
    std::string pciePath;
    std::string location;
    std::string diskId;
    uint64_t usedBytes;
    uint64_t availableBytes;
    std::string devicePath;
    std::string port;

    json ToJson() const;
    static BlockInfo FromJson(const json& j);
    static std::string SerializeVector(const std::vector<BlockInfo>& infos);
    static std::vector<BlockInfo> DeserializeVector(const std::string& jsonStr);
};

class ScanDevice {
public:
    explicit ScanDevice(const std::string &sysBlockPath = "/sys/block", const std::string &devBlockPath = "/dev/block");
    ~ScanDevice() = default;

    std::vector<BlockInfo> GetDataDisks();
    std::vector<BlockInfo> GetExternalDisks();

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
    std::string GetDiskState(const std::string &deviceName);
    MediaType GetMediaType(const std::string &deviceName, const bool isNvmeDevice);
    std::string GetSataSerialNumber(int fd);
    std::string GetNvmeSerialNumber(int fd);
    std::string GetSerialNumber(const std::string &deviceName, const bool isNvmeDevice);
    std::string GetPciePath(const std::string &deviceName);
    std::string GetDiskId(const std::string &deviceName);
    uint64_t GetUsedBytes(const std::string &deviceName);
    uint64_t GetAvailableBytes(const std::string &deviceName);
    std::string GetDevicePath(const std::string &deviceName);
    std::string GetPort(const std::string &pciePath, const bool isNvmeDevice);
    bool IsValidNvmeDevice(const std::string &deviceName);
    bool ParseStringToUlongLong(const std::string &str, unsigned long long &result);

    std::string sysBlockPath;
    std::string devBlockPath;
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_SCAN_DEVICE_H
