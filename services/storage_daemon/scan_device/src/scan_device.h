/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace StorageDaemon {

enum class MediaType { SSD = 0, HDD = 1, UNKNOWN = 2 };

struct BlockInfo {
    uint64_t sizeBytes;        // 磁盘容量
    std::string vendor;        // 厂家
    std::string model;         // 设备型号
    std::string interfaceType; // 接口类型
    uint32_t rpm;              // 机械硬盘转速
    std::string state;         // 磁盘状态
    MediaType mediaType;       // 介质类型（SSD/HDD/UNKNOWN）
    bool removable;            // 是否可插拔（true=外盘，false=内盘）
    std::string serialNumber;  // 序列号
    std::string pciePath;      // PCIe路径
    std::string location;      // 位置
    std::string diskId;        // 磁盘ID
    uint64_t usedBytes;        // 已使用容量
    uint64_t availableBytes;   // 可用容量
    std::string devicePath;    // 设备路径
    std::string port;          // 物理接口
};

class ScanDevice {
public:
    explicit ScanDevice(const std::string &sysBlockPath = "/sys/block", const std::string &devBlockPath = "/dev/block");
    ~ScanDevice() = default;

    /**
     * 获取数据盘设备
     * @return 数据盘信息列表
     */
    std::vector<BlockInfo> GetDataDisks();

    /**
     * 获取外盘设备空实现
     * @return 外盘信息列表
     */
    std::vector<BlockInfo> GetExternalDisks();

private:
    /**
     * 获取设备信息
     * @param deviceName 设备名称
     * @param blockInfo 输出的设备信息
     * @return 成功返回0，失败返回-1
     */
    int GetBlockInfo(const std::string &deviceName, const bool isNvmeDevice, BlockInfo &blockInfo);

    /**
     * 读取sysfs节点内容
     * @param path 节点路径
     * @param content 输出内容
     * @return 成功返回true，失败返回false
     */
    bool ReadSysfsNode(const std::string &path, std::string &content);

    /**
     * 读取removable节点内容
     * @param deviceName 设备名称
     * @param isRemovable 是否可以移除
     * @return 读取成功返回true，失败返回false
     */
    bool ReadRemovableNode(const std::string &deviceName, bool &isRemovable);

    /**
     * 判断是否是数据盘
     * @param deviceName 设备名称
     * @param isNeedCheckUfs 是否需要检查ufs
     * @param isRemovable 是否可以移除
     * @return 数据盘返回true，其他返回false
     */
    bool IsDataDisk(const std::string &deviceName, const bool isNeedCheckUfs, const bool isRemovable);

    /**
     * 获取磁盘容量
     * @param deviceName 设备名称
     * @return 容量（字节）
     */
    uint64_t GetDiskSize(const std::string &deviceName);

    /**
     * 获取厂家信息
     * @param deviceName 设备名称
     * @return 厂家名称
     */
    std::string GetVendor(const std::string &deviceName);

    /**
     * 获取设备型号
     * @param deviceName 设备名称
     * @return 设备型号
     */
    std::string GetModel(const std::string &deviceName);

    /**
     * 获取接口类型
     * @param deviceName 设备名称
     * @return 接口类型
     */
    std::string GetInterfaceType(const std::string &deviceName);

    /**
     * 获取机械硬盘转速
     * @param deviceName 设备名称
     * @return 转速（RPM）
     */
    uint32_t GetDiskRpm(const std::string &deviceName, const bool isNvmeDevice);

    /**
     * 获取磁盘状态
     * @param deviceName 设备名称
     * @return 磁盘状态
     */
    std::string GetDiskState(const std::string &deviceName);

    /**
     * 获取介质类型
     * @param deviceName 设备名称
     * @return 介质类型
     */
    MediaType GetMediaType(const std::string &deviceName, const bool isNvmeDevice);

    /**
     * 获取SATA类型设备序列号
     * @param fd 设备文件
     * @return 序列号
     */
    std::string GetSataSerialNumber(int fd);

    /**
     * 获取Nvme类型设备序列号
     * @param fd 设备文件
     * @return 序列号
     */
    std::string GetNvmeSerialNumber(int fd);

    /**
     * 获取序列号
     * @param deviceName 设备名称
     * @return 序列号
     */
    std::string GetSerialNumber(const std::string &deviceName, const bool isNvmeDevice);

    /**
     * 获取PCIe路径
     * @param deviceName 设备名称
     * @return PCIe路径
     */
    std::string GetPciePath(const std::string &deviceName);

    /**
     * 获取磁盘ID
     * @param deviceName 设备名称
     * @return 磁盘ID
     */
    std::string GetDiskId(const std::string &deviceName);

    /**
     * 获取已使用容量
     * @param deviceName 设备名称
     * @return 已使用容量（字节）
     */
    uint64_t GetUsedBytes(const std::string &deviceName);

    /**
     * 获取可用容量
     * @param deviceName 设备名称
     * @return 可用容量（字节）
     */
    uint64_t GetAvailableBytes(const std::string &deviceName);

    /**
     * 获取设备路径
     * @param deviceName 设备名称
     * @return 设备路径
     */
    std::string GetDevicePath(const std::string &deviceName);

    /**
     * 获取物理接口
     * @param pciePath PCIE路径
     * @return 物理接口
     */
    std::string GetPort(const std::string &pciePath, const bool isNvmeDevice);

    bool IsValidNvmeDevice(const std::string &deviceName);

    bool ParseStringToUlongLong(const std::string &str, unsigned long long &result);

    std::string sysBlockPath;
    std::string devBlockPath;
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_SCAN_DEVICE_H