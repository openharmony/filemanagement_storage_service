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
#include <map>
#include <string>
#include <vector>

#include "disk/disk_info.h"

namespace OHOS {
namespace StorageDaemon {
class IDiskUtilMoc {
public:
    virtual ~IDiskUtilMoc() = default;
public:
    virtual int GetMaxVolume(dev_t device) = 0;
    virtual int IsBlankCD(const std::string &diskPath, bool &isBlankCD) = 0;
    virtual int IsExistCD(const std::string &diskPath, bool &isExistCD) = 0;
    virtual bool IsAcceptableUuid(const std::string &uuid) = 0;
    virtual std::string GetBlkidData(const std::string &devPath, const std::string &type) = 0;
    virtual std::string GetBlkidDataByCmd(std::vector<std::string> &cmd) = 0;
    virtual std::string GetAnonyString(const std::string &value) = 0;
    virtual std::string GetCDType(const std::string &diskPath) = 0;
    virtual std::string GetOpticalDriveType(const std::string &diskPath) = 0;
    virtual int GetOpticalDriveMaxWriteSpeed(const std::string &diskPath, int32_t &maxWriteSpeed) = 0;
    virtual int GetCDStatus(const char *device, int &status) = 0;
    virtual int GetCdTotalCapacity(int fd, int64_t &cdTotalCapacity) = 0;
    virtual int GetCdUsedCapacity(int fd, int64_t &cdUsedCapacity) = 0;
    virtual int GetDvdTotalCapacity(int fd, int64_t &dvdTotalCapacity) = 0;
    virtual int GetDvdUsedCapacity(int fd, int64_t &dvdUsedCapacity) = 0;
    virtual int GetDvdPlusRwTotalCapacity(int fd, int64_t &dvdTotalCapacity) = 0;
    virtual int GetBdTotalCapacity(int fd, int64_t &bdTotalCapacity) = 0;
    virtual int GetDvdConfiguration(int fd, int &dvdMedia) = 0;
    virtual std::string GetScsiBusNum(const std::string &sysPath) = 0;
    virtual std::string GetOddDriverType(const std::string &sysPath) = 0;
    virtual int32_t QueryCDStatus(const std::string &devPath, int32_t &status) = 0;
    virtual int32_t CleanTempDirectory() = 0;
    virtual int32_t EjectCD(const std::string &devPath) = 0;
    virtual std::vector<std::string> SplitString(const std::string &str, char delimiter) = 0;
    virtual std::string GetRelativePath(const std::string &fullPath, const std::string &baseDir) = 0;
    virtual std::vector<std::string> MergeOutputLines(const std::vector<std::string> &output) = 0;
    virtual std::string ParseDirectoryPath(const std::string &line) = 0;
    virtual bool IsFileEntry(const std::string &line, char &entryType) = 0;
    virtual std::string ParseFileName(const std::string &trimmedLine) = 0;
    virtual int32_t GenerateChecksums(const std::string &dirPath, const std::string &checksumFilePath) = 0;
    virtual std::map<std::string, std::string> ParseChecksumFile(
        const std::string &checksumContent, const std::string &basePath) = 0;
    virtual int32_t CompareChecksums(
        const std::map<std::string, std::string> &sourceMap,
        const std::map<std::string, std::string> &discMap) = 0;
    virtual bool IsCDBlank(const std::string &diskPath) = 0;
    virtual int32_t GetIncBurnAddr(const std::string &devPath, std::string &incBurnAddr) = 0;
    virtual std::string GetOpticalDriveNode(const std::string &devPath) = 0;
    virtual int64_t GetDiscCapacity(int cmdFd, const std::string &discType) = 0;
    virtual void AdjustBlankDiscCapacity(const std::string &devPath, const std::string &discType,
                                         int64_t &totalSize, int64_t &usedSize) = 0;
    virtual int64_t GetUsedSizeFromSysfs(const std::string &devPath) = 0;
    virtual int32_t GetCapacity(const std::string &devPath, int64_t &totalSize, int64_t &freeSize) = 0;
    virtual std::string GenerateRandomUuid(const std::string &diskPath, const std::string &ns) = 0;
    virtual int32_t RefreshCDRomMediaNode(const std::string &devPath) = 0;
public:
    static inline std::shared_ptr<IDiskUtilMoc> diskUtilMoc = nullptr;
};
 
class DiskUtilMoc : public IDiskUtilMoc {
public:
    MOCK_METHOD1(GetMaxVolume, int(dev_t device));
    MOCK_METHOD2(IsBlankCD, int(const std::string &diskPath, bool &isBlankCD));
    MOCK_METHOD2(IsExistCD, int(const std::string &diskPath, bool &isExistCD));
    MOCK_METHOD1(IsAcceptableUuid, bool(const std::string &uuid));
    MOCK_METHOD2(GetBlkidData, std::string(const std::string &devPath, const std::string &type));
    MOCK_METHOD1(GetBlkidDataByCmd, std::string(std::vector<std::string> &cmd));
    MOCK_METHOD1(GetAnonyString, std::string(const std::string &value));
    MOCK_METHOD1(GetCDType, std::string(const std::string &diskPath));
    MOCK_METHOD1(GetOpticalDriveType, std::string(const std::string &diskPath));
    MOCK_METHOD2(GetOpticalDriveMaxWriteSpeed, int(const std::string &diskPath, int32_t &maxWriteSpeed));
    MOCK_METHOD2(GetCDStatus, int(const char *device, int &status));
    MOCK_METHOD2(GetCdTotalCapacity, int(int fd, int64_t &cdTotalCapacity));
    MOCK_METHOD2(GetCdUsedCapacity, int(int fd, int64_t &cdUsedCapacity));
    MOCK_METHOD2(GetDvdTotalCapacity, int(int fd, int64_t &dvdTotalCapacity));
    MOCK_METHOD2(GetDvdUsedCapacity, int(int fd, int64_t &dvdUsedCapacity));
    MOCK_METHOD2(GetDvdPlusRwTotalCapacity, int(int fd, int64_t &dvdTotalCapacity));
    MOCK_METHOD2(GetBdTotalCapacity, int(int fd, int64_t &bdTotalCapacity));
    MOCK_METHOD2(GetDvdConfiguration, int(int fd, int &dvdMedia));
    MOCK_METHOD1(GetScsiBusNum, std::string(const std::string &sysPath));
    MOCK_METHOD1(GetOddDriverType, std::string(const std::string &sysPath));
    MOCK_METHOD2(QueryCDStatus, int32_t(const std::string &devPath, int32_t &status));
    MOCK_METHOD0(CleanTempDirectory, int32_t());
    MOCK_METHOD1(EjectCD, int32_t(const std::string &devPath));
    MOCK_METHOD2(SplitString, std::vector<std::string>(const std::string &str, char delimiter));
    MOCK_METHOD2(GetRelativePath, std::string(const std::string &fullPath, const std::string &baseDir));
    MOCK_METHOD1(MergeOutputLines, std::vector<std::string>(const std::vector<std::string> &output));
    MOCK_METHOD1(ParseDirectoryPath, std::string(const std::string &line));
    MOCK_METHOD2(IsFileEntry, bool(const std::string &line, char &entryType));
    MOCK_METHOD1(ParseFileName, std::string(const std::string &trimmedLine));
    MOCK_METHOD2(GenerateChecksums, int32_t(const std::string &dirPath, const std::string &checksumFilePath));
    MOCK_METHOD2(ParseChecksumFile, std::map<std::string, std::string>(
        const std::string &checksumContent, const std::string &basePath));
    MOCK_METHOD2(CompareChecksums, int32_t(
        const std::map<std::string, std::string> &sourceMap,
        const std::map<std::string, std::string> &discMap));
    MOCK_METHOD1(IsCDBlank, bool(const std::string &diskPath));
    MOCK_METHOD2(GetIncBurnAddr, int32_t(const std::string &devPath, std::string &incBurnAddr));
    MOCK_METHOD1(GetOpticalDriveNode, std::string(const std::string &devPath));
    MOCK_METHOD2(GetDiscCapacity, int64_t(int cmdFd, const std::string &discType));
    MOCK_METHOD4(AdjustBlankDiscCapacity, void(const std::string &devPath, const std::string &discType,
                                               int64_t &totalSize, int64_t &usedSize));
    MOCK_METHOD1(GetUsedSizeFromSysfs, int64_t(const std::string &devPath));
    MOCK_METHOD3(GetCapacity, int32_t(const std::string &devPath, int64_t &totalSize, int64_t &freeSize));
    MOCK_METHOD2(GenerateRandomUuid, std::string(const std::string &diskPath, const std::string &ns));
    MOCK_METHOD1(RefreshCDRomMediaNode, int32_t(const std::string &devPath));
};
}
}
#endif