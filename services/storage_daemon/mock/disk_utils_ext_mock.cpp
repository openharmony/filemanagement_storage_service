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

#include "disk_utils_mock.h"

#include "disk_manager/disk/disk_utils.h"
#include "storage_service_errno.h"
#include "utils/disk_utils.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {

int32_t DiskUtils::QueryCDStatus(const std::string &devPath, int32_t &status)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->QueryCDStatus(devPath, status);
}

int32_t DiskUtils::CleanTempDirectory()
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->CleanTempDirectory();
}

int32_t DiskUtils::EjectCD(const std::string &devPath)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->EjectCD(devPath);
}

std::vector<std::string> DiskUtils::SplitString(const std::string &str, char delimiter)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return {};
    }
    return IDiskUtilMoc::diskUtilMoc->SplitString(str, delimiter);
}

std::string DiskUtils::GetRelativePath(const std::string &fullPath, const std::string &baseDir)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return "";
    }
    return IDiskUtilMoc::diskUtilMoc->GetRelativePath(fullPath, baseDir);
}

std::vector<std::string> DiskUtils::MergeOutputLines(const std::vector<std::string> &output)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return {};
    }
    return IDiskUtilMoc::diskUtilMoc->MergeOutputLines(output);
}

std::string DiskUtils::ParseDirectoryPath(const std::string &line)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return "";
    }
    return IDiskUtilMoc::diskUtilMoc->ParseDirectoryPath(line);
}

bool DiskUtils::IsFileEntry(const std::string &line, char &entryType)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return false;
    }
    return IDiskUtilMoc::diskUtilMoc->IsFileEntry(line, entryType);
}

std::string DiskUtils::ParseFileName(const std::string &trimmedLine)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return "";
    }
    return IDiskUtilMoc::diskUtilMoc->ParseFileName(trimmedLine);
}

int32_t DiskUtils::GenerateChecksums(const std::string &dirPath, const std::string &checksumFilePath)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->GenerateChecksums(dirPath, checksumFilePath);
}

std::map<std::string, std::string> DiskUtils::ParseChecksumFile(
    const std::string &checksumContent, const std::string &basePath)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return {};
    }
    return IDiskUtilMoc::diskUtilMoc->ParseChecksumFile(checksumContent, basePath);
}

int32_t DiskUtils::CompareChecksums(
    const std::map<std::string, std::string> &sourceMap,
    const std::map<std::string, std::string> &discMap)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->CompareChecksums(sourceMap, discMap);
}

int IsCDBlank(const std::string &diskPath, bool &isCDBlank)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->IsCDBlank(diskPath, isCDBlank);
}

int32_t GetIncBurnAddr(const std::string &devPath, std::string &incBurnAddr)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return E_ERR;
    }
    return IDiskUtilMoc::diskUtilMoc->GetIncBurnAddr(devPath, incBurnAddr);
}

std::string GenerateRandomUuid(const std::string &diskPath, const std::string &namespaceUuid)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return "";
    }
    return IDiskUtilMoc::diskUtilMoc->GenerateRandomUuid(diskPath, namespaceUuid);
}

} // namespace StorageDaemon
} // namespace OHOS
