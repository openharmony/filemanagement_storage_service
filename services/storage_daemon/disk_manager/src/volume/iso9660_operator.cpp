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

#include "disk_manager/volume/iso9660_operator.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "disk_manager/disk/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/string_utils.h"

#include <cerrno>
#include <sys/mount.h>
#include <map>
#include <sstream>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr int UID_FILE_MANAGER = 1006;
constexpr const char* MNT_EXTERNAL_FILE_CONTEXT = "context=u:object_r:mnt_external_file:s0";
constexpr const char* IO_CHAR_SET = "utf8";
constexpr const char* MID_PATH = "/data/local/burn_tmp/midFile.iso";
constexpr const char* BURN_TMP_DIR = "/data/local/burn_tmp";
constexpr const char* VERIFY_MOUNT_PATH = "/mnt/data/burn_verify_mount";
constexpr int32_t E_VERIFY_BURN_DATA_FAILED = 13600030;
constexpr mode_t DEFAULT_DIR_PERMISSIONS = 0755;

int32_t IsoOperator::DoMount(const std::string& devPath,
                             const std::string& mountPath,
                             unsigned long mountFlags,
                             const std::string& mountData)
{
    LOGI("IsoOperator::DoMount devPath=%{public}s, mountPath=%{public}s",
        devPath.c_str(), mountPath.c_str());

    int32_t cdStatus = 0;
    int32_t ret = DiskUtils::QueryCDStatus(devPath, cdStatus);
    uint32_t statusMask = static_cast<uint32_t>(cdStatus);
    if (ret == E_OK && (statusMask & 0x01) != 0 && (statusMask & 0x02) != 0) {
        LOGI("IsoOperator::DoMount empty disc, skip mount");
        return E_OK;
    }

    std::string options = "rw,uid=1006,gid=1006,dmask=0006,fmask=0007";
    std::string mountIsoData = StringPrintf("ro,uid=%d,gid=%d,%s,iocharset=%s",
        UID_FILE_MANAGER, UID_FILE_MANAGER, MNT_EXTERNAL_FILE_CONTEXT, IO_CHAR_SET);
    std::vector<std::string> cmd = {
        "mount",
        "-t",
        "iso9660",
        "-o",
        mountIsoData,
        devPath,
        mountPath,
    };

    std::vector<std::string> output;
    ret = ForkExec(cmd, &output);

    for (auto& str : output) {
        LOGI("IsoOperator::DoMount output: %{public}s", str.c_str());
    }

    if (ret != E_OK) {
        LOGE("IsoOperator::DoMount failed, ret=%{public}d, errno=%{public}d", ret, errno);
        return E_ISO9660_MOUNT;
    }

    LOGI("IsoOperator::DoMount success");
    return E_OK;
}

int32_t IsoOperator::ReadMetadata(const std::string& devPath,
                                  std::string& uuid,
                                  std::string& type,
                                  std::string& label)
{
    LOGI("IsoOperator::ReadMetadata devPath=%{public}s", devPath.c_str());

    if (devPath.empty() || devPath.length() >= PATH_MAX) {
        LOGE("IsoOperator::ReadMetadata invalid devPath");
        return E_PARAMS_INVALID;
    }
    char realPath[PATH_MAX] = {0};
    if (realpath(devPath.c_str(), realPath) == nullptr) {
        LOGE("IsoOperator::ReadMetadata realpath failed, errno=%{public}d", errno);
        return E_PARAMS_INVALID;
    }
    if (std::string(realPath).find("/dev/block/") != 0) {
        LOGE("IsoOperator::ReadMetadata invalid devPath prefix");
        return E_PARAMS_INVALID;
    }

    uuid = GetBlkidData(realPath, "UUID");
    std::string offset = devPath + "/" + type;
    if (uuid.empty()) {
        uuid = GenerateRandomUuid(realPath, offset);
    }

    label = GetBlkidData(realPath, "LABEL");
    if (label.empty()) {
        label = GetCDType(realPath);
    }

    LOGI("IsoOperator::ReadMetadata success - uuid=%{public}s, type=%{public}s, label=%{public}s",
         GetAnonyString(uuid).c_str(), type.c_str(), GetAnonyString(label).c_str());
    return E_OK;
}

int32_t IsoOperator::CreateIsoImage(const std::string &devPath,
                                    const std::string &filePath,
                                    const std::string &mountPath)
{
    LOGI("IsoOperator CreateIsoImage:>>> ENTER <<< devPath=%{public}s, mountPath=%{public}s",
        devPath.c_str(), mountPath.c_str());
    int32_t res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("IsoOperator CreateIsoImage: CleanTempDirectory entry failed, non-critical, res=%{public}d", res);
    }
    std::vector<std::string> output;
    std::vector<std::string> cmd = {"genisoimage", "-V", "ISOIMAGE", "-J", "-r", "-o", filePath, mountPath};

    int32_t err = ForkExec(cmd, &output);
    for (const auto& s : output) {
        LOGI("IsoOperator CreateIsoImage:s=%{public}s", s.c_str());
    }
    if (err != E_OK) {
        LOGE("IsoOperator CreateIsoImage:<<< EXIT FAILED <<< failed for devPath: %{public}s", devPath.c_str());
        return err;
    }
    res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("IsoOperator CreateIsoImage: CleanTempDirectory exit failed, non-critical, res=%{public}d", res);
    }
    LOGI("IsoOperator CreateIsoImage:<<< EXIT SUCCESS <<< devPath=%{public}s", devPath.c_str());
    return E_OK;
}

int32_t IsoOperator::PrepareIsoImage(const std::string &devPath,
                                     const BurnOptions &burnOptions,
                                     bool isDiskEmpty,
                                     const std::string &incBurnAddr)
{
    LOGI("PrepareIsoImage: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    if (IsDir(BURN_TMP_DIR)) {
        LOGI("PrepareIsoImage: burn_tmp directory exists, removing it");
        RmDirRecurse(BURN_TMP_DIR);
    }
    int32_t err = MkDir(BURN_TMP_DIR, DEFAULT_DIR_PERMISSIONS);
    if (err != 0) {
        LOGE("PrepareIsoImage:<<< EXIT FAILED <<< Create burn_tmp directory failed for devPath: %{public}s",
             devPath.c_str());
        return E_ERR;
    }
    int32_t res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("PrepareIsoImage: CleanTempDirectory failed, non-critical, res=%{public}d", res);
    }
    if (burnOptions.isIsoImage) {
        LOGI("PrepareIsoImage: Using existing ISO image, skip generation");
        return E_OK;
    }
    std::vector<std::string> cmd;
    std::vector<std::string> output;
    if (isDiskEmpty) {
        cmd = {"genisoimage", "-V", burnOptions.diskName, "-J", "-r", "-o", MID_PATH, burnOptions.burnPath};
    } else {
        cmd = {"genisoimage", "-V", burnOptions.diskName, "-J", "-r", "-C", incBurnAddr, "-M", devPath, "-o",
                MID_PATH, burnOptions.burnPath};
    }
    err = ForkExec(cmd, &output);
    if (err != E_OK) {
        for (const auto& s : output) {
            LOGI("IsoOperator PrepareIsoImage:s=%{public}s", s.c_str());
        }
        LOGE("PrepareIsoImage:<<< EXIT FAILED <<< genisoimage failed for devPath: %{public}s", devPath.c_str());
        RmDirRecurse(BURN_TMP_DIR);
        return err;
    }
    LOGI("PrepareIsoImage:<<< EXIT SUCCESS <<< ISO image prepared");
    return E_OK;
}

int32_t IsoOperator::DoCDBurn(const std::string &devPath,
                              const BurnOptions &burnOptions,
                              bool isDiskEmpty,
                              const std::string &incBurnAddr)
{
    LOGI("BurnDoCDBurn: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    int32_t err = PrepareIsoImage(devPath, burnOptions, isDiskEmpty, incBurnAddr);
    if (err != E_OK) {
        LOGE("BurnDoCDBurn:<<< EXIT FAILED <<< PrepareIsoImage failed for devPath: %{public}s", devPath.c_str());
        return err;
    }
    std::string speedOpt = "-speed=" + std::to_string(burnOptions.burnSpeed);
    std::vector<std::string> cmd;
    std::vector<std::string> output;
    if (!burnOptions.isIsoImage) {
        if (isDiskEmpty) {
            cmd = {"wodim", "-v", "dev=" + devPath, "-multi", "-data", speedOpt, MID_PATH};
        } else {
            cmd = {"wodim", "-v", "dev=" + devPath, "-tao", "-multi", "-data", speedOpt, MID_PATH};
        }
    } else {
        cmd = {"wodim", "-v", "dev=" + devPath, "-multi", "-data", speedOpt, burnOptions.burnPath};
    }
    err = ForkExec(cmd, &output);
    for (const auto& s : output) {
        LOGI("IsoOperator DoCDBurn:s=%{public}s", s.c_str());
    }
    if (err != E_OK) {
        LOGE("BurnDoCDBurn:<<< EXIT FAILED <<< wodim failed for devPath: %{public}s", devPath.c_str());
        RmDirRecurse(BURN_TMP_DIR);
        return err;
    }
    LOGI("BurnDoCDBurn:<<< EXIT SUCCESS <<< devPath=%{public}s", devPath.c_str());
    return E_OK;
}

int32_t IsoOperator::DoDVDBurn(const std::string &devPath, const BurnOptions &burnOptions, bool isDiskEmpty)
{
    LOGI("BurnDoDVDBurn: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    int32_t res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("BurnDoDVDBurn: CleanTempDirectory entry failed, non-critical, res=%{public}d", res);
    }
    int32_t err = 0;
    std::string speedOpt = "-speed=" + std::to_string(burnOptions.burnSpeed);
    std::vector<std::string> cmd;
    std::vector<std::string> output;
    if (!burnOptions.isIsoImage) {
        if (isDiskEmpty) {
            cmd = {"growisofs", speedOpt, "-Z", devPath,
                   "-J", "-r", "-V", burnOptions.diskName, burnOptions.burnPath};
        } else {
            cmd = {"growisofs", speedOpt, "-M", devPath,
                   "-J", "-r", "-V", burnOptions.diskName, burnOptions.burnPath};
        }
    } else {
        std::string isoBurnPath = devPath + "=" + burnOptions.burnPath;
        cmd = {"growisofs", speedOpt, "-Z", isoBurnPath};
    }
    err = ForkExec(cmd, &output);
    for (const auto& s : output) {
        LOGI("IsoOperator DoDVDBurn:s=%{public}s", s.c_str());
    }
    if (err != E_OK) {
        LOGE("BurnDoDVDBurn:<<< EXIT FAILED <<< failed for devPath: %{public}s", devPath.c_str());
        return err;
    }
    res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("BurnDoDVDBurn: CleanTempDirectory exit failed, non-critical, res=%{public}d", res);
    }
    LOGI("BurnDoDVDBurn:<<< EXIT SUCCESS <<< devPath=%{public}s", devPath.c_str());
    return E_OK;
}

int32_t IsoOperator::Burn(const std::string &devPath, const BurnOptions &burnOptions)
{
    LOGI("Burn devPath = %{public}s, isVerifyBurn = %{public}d", devPath.c_str(), burnOptions.isVerifyBurn);
    int32_t err = 0;

    bool isDiskEmpty = false;
    bool isBlankCD = false;
    int blankRet = IsCDBlank(devPath, isBlankCD);
    if (blankRet == E_OK && isBlankCD) {
        isDiskEmpty = true;
    }
    std::string incBurnAddr;
    if (!isDiskEmpty) {
        err = GetIncBurnAddr("dev=" + devPath, incBurnAddr);
        if (err != E_OK) {
            LOGE("Burn:<<< EXIT FAILED <<< devPath=%{public}s", devPath.c_str());
            return err;
        }
    }
    std::string diskType = GetCDType(devPath);
    if (diskType.find("CD") != std::string::npos) {
        err = DoCDBurn(devPath, burnOptions, isDiskEmpty, incBurnAddr);
    } else {
        err = DoDVDBurn(devPath, burnOptions, isDiskEmpty);
    }
    if (err != E_OK) {
        LOGE("Burn:<<< EXIT FAILED <<< devPath:=%{public}s", devPath.c_str());
        return err;
    }

    if (burnOptions.isVerifyBurn) {
        LOGI("Burn: starting verify process for devPath=%{public}s", devPath.c_str());
        err = DoVerifyBurnData(devPath, burnOptions, isDiskEmpty);
        if (err != E_OK) {
            LOGE("Burn: verify failed, err=%{public}d", err);
            DiskUtils::EjectCD(devPath);
            return E_VERIFY_BURN_DATA_FAILED;
        }
    }

    LOGI("Burn: ejecting disc for devPath=%{public}s", devPath.c_str());
    DiskUtils::EjectCD(devPath);
    
    LOGI("Burn:<<< EXIT SUCCESS <<< devPath=%{public}s", devPath.c_str());
    return E_OK;
}

int32_t IsoOperator::PrepareVerifyMountPath()
{
    if (IsDir(VERIFY_MOUNT_PATH)) {
        RmDirRecurse(VERIFY_MOUNT_PATH);
    }
    int32_t err = MkDir(VERIFY_MOUNT_PATH, DEFAULT_DIR_PERMISSIONS);
    if (err != 0) {
        LOGE("PrepareVerifyMountPath: create verify mount path failed");
        return E_ERR;
    }
    return E_OK;
}

int32_t IsoOperator::ExecuteIsoInfoList(const std::string& isoPath,
                                        std::vector<std::string>& mergedLines)
{
    LOGI("ExtractIsoFiles: listing files in source ISO");
    std::vector<std::string> cmd = {"isoinfo", "-R", "-i", isoPath, "-l"};
    std::vector<std::string> rawOutput;
    int32_t err = ForkExec(cmd, &rawOutput);
    if (err != E_OK) {
        LOGE("ExtractIsoFiles: isoinfo list failed");
        return E_ERR;
    }
    std::vector<std::string> output;
    for (const auto& s : rawOutput) {
        std::vector<std::string> lines = DiskUtils::SplitString(s, '\n');
        output.insert(output.end(), lines.begin(), lines.end());
    }
    for (const auto& s : output) {
        LOGI("ExtractIsoFiles isoinfo list output: %{public}s", s.c_str());
    }
    mergedLines = DiskUtils::MergeOutputLines(output);
    for (const auto& s : mergedLines) {
        LOGI("ExtractIsoFiles merged line: %{public}s", s.c_str());
    }
    return E_OK;
}

int32_t IsoOperator::ProcessMergedLine(const std::string& isoPath, const std::string& sourceDir,
                                       const std::string& line, std::string& currentPath)
{
    LOGI("ExtractIsoFiles: processing line: %{public}s", line.c_str());
    if (line.find("Directory listing of") != std::string::npos) {
        currentPath = DiskUtils::ParseDirectoryPath(line);
        LOGI("ExtractIsoFiles: currentPath changed to: %{public}s", currentPath.c_str());
        return E_OK;
    }
    char entryType;
    if (!DiskUtils::IsFileEntry(line, entryType)) {
        return E_OK;
    }
    size_t firstNonSpace = line.find_first_not_of(' ');
    std::string trimmedLine = line.substr(firstNonSpace);
    std::string name = DiskUtils::ParseFileName(trimmedLine);
    if (name.empty()) {
        return E_OK;
    }
    LOGI("ExtractIsoFiles: name=%{public}s, type=%{public}c, path=%{public}s",
        name.c_str(), entryType, currentPath.c_str());
    std::string fullIsoPath = currentPath + "/" + name;
    std::string fullOutputPath = sourceDir + currentPath + "/" + name;
    if (entryType == 'd') {
        LOGI("ExtractIsoFiles: creating directory: %{public}s", fullOutputPath.c_str());
        return MkDir(fullOutputPath, DEFAULT_DIR_PERMISSIONS);
    }
    size_t lastSlash = fullOutputPath.find_last_of('/');
    if (lastSlash != std::string::npos) {
        std::string dirPart = fullOutputPath.substr(0, lastSlash);
        if (!IsDir(dirPart)) {
            LOGI("ExtractIsoFiles: creating intermediate dir: %{public}s", dirPart.c_str());
            if (MkDir(dirPart, DEFAULT_DIR_PERMISSIONS) != E_OK) {
                return E_ERR;
            }
        }
    }
    LOGI("ExtractIsoFiles: extracting file: %{public}s to %{public}s", fullIsoPath.c_str(), fullOutputPath.c_str());
    std::string extractCmd = "isoinfo -R -i \"" + isoPath + "\" -x \"" +
        fullIsoPath + "\" > \"" + fullOutputPath + "\"";
    std::vector<std::string> cmd = {"/bin/sh", "-c", extractCmd};
    std::vector<std::string> extractOutput;
    int32_t extractErr = ForkExec(cmd, &extractOutput);
    for (const auto& s : extractOutput) {
        LOGI("ExtractIsoFiles extract output: %{public}s", s.c_str());
    }
    if (extractErr != E_OK) {
        LOGE("ExtractIsoFiles: extract failed for %{public}s", fullIsoPath.c_str());
        return E_ERR;
    }
    return E_OK;
}

int32_t IsoOperator::ExtractIsoFiles(const std::string& isoPath,
                                     const std::string& sourceDir)
{
    std::vector<std::string> mergedLines;
    int32_t err = ExecuteIsoInfoList(isoPath, mergedLines);
    if (err != E_OK) {
        return err;
    }
    std::string currentPath;
    for (const auto& line : mergedLines) {
        err = ProcessMergedLine(isoPath, sourceDir, line, currentPath);
        if (err != E_OK) {
            return err;
        }
    }
    return E_OK;
}

int32_t IsoOperator::GenerateChecksums(const std::string& dirPath,
                                       const std::string& checksumFilePath)
{
    LOGI("GenerateChecksums: generating MD5 for %{public}s", dirPath.c_str());
    std::vector<std::string> cmd = {"find", dirPath, "-type", "f", "-exec",
                                    "md5sum", "{}", ";"};
    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);
    if (err != E_OK) {
        LOGE("GenerateChecksums: find/md5sum failed for %{public}s",
             dirPath.c_str());
        return E_ERR;
    }

    std::string checksumContent;
    for (const auto& s : output) {
        checksumContent += s + "\n";
    }

    std::string errMsg;
    if (!WriteFileSync(checksumFilePath.c_str(),
                       reinterpret_cast<const uint8_t*>(checksumContent.c_str()),
                       checksumContent.size(), errMsg)) {
        LOGE("GenerateChecksums: write checksum failed: %{public}s",
             errMsg.c_str());
        return E_ERR;
    }

    return E_OK;
}

std::map<std::string, std::string> IsoOperator::ParseChecksumFile(
    const std::string& checksumContent, const std::string& basePath)
{
    std::map<std::string, std::string> checksumMap;
    std::vector<std::string> lines = DiskUtils::SplitString(checksumContent, '\n');

    for (const auto& line : lines) {
        if (line.empty()) {
            continue;
        }
        size_t pos = line.find("  ");
        if (pos != std::string::npos) {
            std::string md5 = line.substr(0, pos);
            std::string filePath = line.substr(pos + 2);
            std::string relativePath = DiskUtils::GetRelativePath(filePath, basePath);
            checksumMap[relativePath] = md5;
        }
    }

    return checksumMap;
}

void IsoOperator::LogChecksumMap(const std::string& mapName,
                                 const std::map<std::string, std::string>& map)
{
    LOGI("LogChecksumMap: %{public}s contents:", mapName.c_str());
    for (const auto& pair : map) {
        LOGI("LogChecksumMap:   [%{public}s] = [%{public}s]",
             pair.first.c_str(), pair.second.c_str());
    }
}

int32_t IsoOperator::CompareChecksums(
    const std::map<std::string, std::string>& sourceMap,
    const std::map<std::string, std::string>& discMap)
{
    LOGI("CompareChecksums: comparing, sourceFiles=%{public}zu, discFiles=%{public}zu",
         sourceMap.size(), discMap.size());

    for (const auto& pair : sourceMap) {
        const std::string& filename = pair.first;
        const std::string& sourceMd5 = pair.second;
        auto it = discMap.find(filename);
        if (it == discMap.end()) {
            LOGE("CompareChecksums: file not found on disc: %{public}s",
                 filename.c_str());
            return E_VERIFY_BURN_DATA_FAILED;
        }
        if (sourceMd5 != it->second) {
            LOGE("CompareChecksums: MD5 mismatch for file: %{public}s",
                 filename.c_str());
            return E_VERIFY_BURN_DATA_FAILED;
        }
    }

    LOGI("CompareChecksums: checksum comparison passed");
    return E_OK;
}

int32_t IsoOperator::PrepareSourceDirectory(const BurnOptions& burnOptions, std::string& sourceDir)
{
    if (burnOptions.isIsoImage) {
        sourceDir = std::string(BURN_TMP_DIR) + "/source_extract";
        if (IsDir(sourceDir)) {
            RmDirRecurse(sourceDir);
        }
        int32_t err = MkDir(sourceDir, DEFAULT_DIR_PERMISSIONS);
        if (err != E_OK) {
            LOGE("DoVerifyBurnData: create source extract dir failed");
            return E_ERR;
        }
        err = ExtractIsoFiles(burnOptions.burnPath, sourceDir);
        if (err != E_OK) {
            LOGE("DoVerifyBurnData: extract ISO files failed");
            return err;
        }
    } else {
        sourceDir = burnOptions.burnPath;
    }
    return E_OK;
}

int32_t IsoOperator::GenerateAndCompareChecksums(const std::string& sourceDir,
                                                 const std::string& sourceChecksumPath,
                                                 const std::string& discChecksumPath)
{
    int32_t err = DiskUtils::GenerateChecksums(sourceDir, sourceChecksumPath);
    if (err != E_OK) {
        LOGE("DoVerifyBurnData: generate source checksums failed");
        return err;
    }
    err = DiskUtils::GenerateChecksums(VERIFY_MOUNT_PATH, discChecksumPath);
    if (err != E_OK) {
        LOGE("DoVerifyBurnData: generate disc checksums failed");
        return err;
    }
    err = Unmount(VERIFY_MOUNT_PATH, "iso9660", false);
    if (err != E_OK) {
        LOGE("DoVerifyBurnData: unmount failed, err=%{public}d", err);
        return err;
    }
    std::string sourceChecksumContent = ReadFileContent(sourceChecksumPath);
    if (sourceChecksumContent.empty()) {
        LOGE("DoVerifyBurnData: read source checksum file failed");
        return E_ERR;
    }
    std::string discChecksumContent = ReadFileContent(discChecksumPath);
    if (discChecksumContent.empty()) {
        LOGE("DoVerifyBurnData: read disc checksum file failed");
        return E_ERR;
    }
    std::map<std::string, std::string> sourceMap = DiskUtils::ParseChecksumFile(sourceChecksumContent, sourceDir);
    std::map<std::string, std::string> discMap = DiskUtils::ParseChecksumFile(discChecksumContent, VERIFY_MOUNT_PATH);
    LOGI("LogChecksumMap: sourceMap contents:");
    for (const auto& pair : sourceMap) {
        LOGI("LogChecksumMap:   [%{public}s] = [%{public}s]", pair.first.c_str(), pair.second.c_str());
    }
    LOGI("LogChecksumMap: discMap contents:");
    for (const auto& pair : discMap) {
        LOGI("LogChecksumMap:   [%{public}s] = [%{public}s]", pair.first.c_str(), pair.second.c_str());
    }
    return DiskUtils::CompareChecksums(sourceMap, discMap);
}

int32_t IsoOperator::DoVerifyBurnData(const std::string &devPath, const BurnOptions &burnOptions,
                                      bool isDiskEmpty)
{
    LOGI("DoVerifyBurnData: >>> ENTER <<< devPath=%{public}s, isIsoImage=%{public}d, isDiskEmpty=%{public}d",
         devPath.c_str(), burnOptions.isIsoImage, isDiskEmpty);
    if (!isDiskEmpty && !burnOptions.isIsoImage) {
        LOGI("DoVerifyBurnData: incremental burn, verifying source files only");
    }
    int32_t err = PrepareVerifyMountPath();
    if (err != E_OK) {
        LOGE("DoVerifyBurnData: prepare mount path failed");
        return err;
    }
    LOGI("DoVerifyBurnData: mounting disc to verify path");
    err = Mount(devPath, VERIFY_MOUNT_PATH, 0, "");
    if (err != E_OK) {
        LOGE("DoVerifyBurnData: mount failed, err=%{public}d", err);
        RmDirRecurse(VERIFY_MOUNT_PATH);
        return err;
    }
    std::string sourceDir;
    err = PrepareSourceDirectory(burnOptions, sourceDir);
    if (err != E_OK) {
        Unmount(VERIFY_MOUNT_PATH, "iso9660", false);
        RmDirRecurse(VERIFY_MOUNT_PATH);
        return err;
    }
    std::string sourceChecksumPath = std::string(BURN_TMP_DIR) + "/source_checksums.txt";
    std::string discChecksumPath = std::string(BURN_TMP_DIR) + "/disc_checksums.txt";
    if (!IsDir(BURN_TMP_DIR)) {
        LOGI("DoVerifyBurnData: burn_tmp not dir, recreating");
        RmDirRecurse(BURN_TMP_DIR);
        err = MkDir(BURN_TMP_DIR, DEFAULT_DIR_PERMISSIONS);
        if (err != E_OK) {
            LOGE("DoVerifyBurnData: create burn_tmp failed, err=%{public}d", err);
            Unmount(VERIFY_MOUNT_PATH, "iso9660", false);
            RmDirRecurse(VERIFY_MOUNT_PATH);
            return E_ERR;
        }
    }
    LOGI("DoVerifyBurnData: BURN_TMP_DIR=%{public}s, sourceChecksumPath=%{public}s",
         BURN_TMP_DIR, sourceChecksumPath.c_str());
    err = GenerateAndCompareChecksums(sourceDir, sourceChecksumPath, discChecksumPath);
    if (err != E_OK) {
        RmDirRecurse(VERIFY_MOUNT_PATH);
        return err;
    }
    RmDirRecurse(VERIFY_MOUNT_PATH);
    LOGI("DoVerifyBurnData: <<< EXIT SUCCESS <<<");
    return E_OK;
}

} // namespace StorageDaemon
} // namespace OHOS