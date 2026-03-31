/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_UTILS_DISK_MOCK_H
#define STORAGE_DAEMON_UTILS_DISK_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "disk/disk_info.h"

namespace OHOS {
namespace StorageDaemon {
class IDiskUtilMoc {
public:
    virtual ~IDiskUtilMoc() = default;
public:
    virtual int CreateDiskNode(const std::string &path, dev_t dev) = 0;
    virtual int DestroyDiskNode(const std::string &path) = 0;
    virtual int GetDevSize(const std::string &path, uint64_t *size) = 0;
    virtual int GetMaxVolume(dev_t device) = 0;
    virtual int IsBlankCD(const std::string &diskPath, bool &isBlankCD) = 0;
    virtual int IsExistCD(const std::string &diskPath, bool &isExistCD) = 0;
    virtual int Eject(const std::string &devPath) = 0;
    virtual int32_t ReadMetadata(const std::string &path, std::string &uuid, std::string &type,
                                 std::string &label) = 0;
    virtual std::string GetBlkidData(const std::string &devPath, const std::string &type) = 0;
    virtual std::string GetBlkidDataByCmd(std::vector<std::string> &cmd) = 0;
    virtual std::string GetAnonyString(const std::string &value) = 0;
    virtual int32_t ReadVolumeUuid(const std::string &devPath, std::string &uuid) = 0;
    virtual std::string GetCDType(const std::string &diskPath) = 0;
    virtual int GetCDStatus(const char *device, int &status) = 0;
    virtual int GetCdTotalCapacity(int fd, int64_t &cdTotalCapacity) = 0;
    virtual int GetCdUsedCapacity(int fd, int64_t &cdUsedCapacity) = 0;
    virtual int GetDvdTotalCapacity(int fd, int64_t &dvdTotalCapacity) = 0;
    virtual int GetDvdUsedCapacity(int fd, int64_t &dvdUsedCapacity) = 0;
    virtual int GetDvdConfiguration(int fd, int &dvdMedia) = 0;
public:
    static inline std::shared_ptr<IDiskUtilMoc> diskUtilMoc = nullptr;
};
 
class DiskUtilMoc : public IDiskUtilMoc {
public:
    MOCK_METHOD2(CreateDiskNode, int(const std::string &path, dev_t dev));
    MOCK_METHOD1(DestroyDiskNode, int(const std::string &path));
    MOCK_METHOD2(GetDevSize, int(const std::string &path, uint64_t *size));
    MOCK_METHOD1(GetMaxVolume, int(dev_t device));
    MOCK_METHOD2(IsBlankCD, int(const std::string &diskPath, bool &isBlankCD));
    MOCK_METHOD2(IsExistCD, int(const std::string &diskPath, bool &isExistCD));
    MOCK_METHOD1(Eject, int(const std::string &devPath));
    MOCK_METHOD4(ReadMetadata, int32_t(const std::string &path, std::string &uuid, std::string &type,
                                       std::string &label));
    MOCK_METHOD2(GetBlkidData, std::string(const std::string &devPath, const std::string &type));
    MOCK_METHOD1(GetBlkidDataByCmd, std::string(std::vector<std::string> &cmd));
    MOCK_METHOD1(GetAnonyString, std::string(const std::string &value));
    MOCK_METHOD2(ReadVolumeUuid, int32_t(const std::string &devPath, std::string &uuid));
    MOCK_METHOD1(GetCDType, std::string(const std::string &diskPath));
    MOCK_METHOD2(GetCDStatus, int(const char *device, int &status));
    MOCK_METHOD2(GetCdTotalCapacity, int(int fd, int64_t &cdTotalCapacity));
    MOCK_METHOD2(GetCdUsedCapacity, int(int fd, int64_t &cdUsedCapacity));
    MOCK_METHOD2(GetDvdTotalCapacity, int(int fd, int64_t &dvdTotalCapacity));
    MOCK_METHOD2(GetDvdUsedCapacity, int(int fd, int64_t &dvdUsedCapacity));
    MOCK_METHOD2(GetDvdConfiguration, int(int fd, int &dvdMedia));
};
}
}
#endif