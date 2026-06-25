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
#include <vector>

namespace OHOS {
namespace StorageDaemon {
constexpr int UID_FILE_MANAGER = 1006;
constexpr const char* MNT_EXTERNAL_FILE_CONTEXT = "context=u:object_r:mnt_external_file:s0";
constexpr const char* IO_CHAR_SET = "utf8";
constexpr const char* MID_PATH = "/data/local/burn_tmp/midFile.iso";
constexpr const char* BURN_TMP_DIR = "/data/local/burn_tmp";

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
    int32_t err = MkDir(BURN_TMP_DIR, 0755);
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
            cmd = {"wodim", "-v", "dev=" + devPath, "-multi", "-eject", "-data", speedOpt, MID_PATH};
        } else {
            cmd = {"wodim", "-v", "dev=" + devPath, "-tao", "-multi", "-eject", "-data", speedOpt, MID_PATH};
        }
    } else {
        cmd = {"wodim", "-v", "dev=" + devPath, "-multi", "-eject", "-data", speedOpt, burnOptions.burnPath};
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
    int32_t res = DiskUtils::CleanTempDirectory();
    if (res != E_OK) {
        LOGE("BurnDoCDBurn: CleanTempDirectory failed, non-critical, res=%{public}d", res);
    }
    RmDirRecurse(BURN_TMP_DIR);
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
    LOGI("Burn devPath = %{public}s", devPath.c_str());
    int32_t err = 0;

    bool isDiskEmpty = false;
    bool isBlankCD = false;
    int blankRet = IsCDBlank(devPath, isBlankCD);
    if (blankRet == E_OK && isBlankCD) {
        isDiskEmpty = true;
    }
    std::string diskType = GetCDType(devPath);
    std::string incBurnAddr;
    if (diskType.find("CD") != std::string::npos) {
        if (!isDiskEmpty) {
            err = GetIncBurnAddr("dev=" + devPath, incBurnAddr);
            if (err != E_OK) {
                LOGE("Burn:<<< EXIT FAILED <<< devPath=%{public}s", devPath.c_str());
                return err;
            }
        }
        err = DoCDBurn(devPath, burnOptions, isDiskEmpty, incBurnAddr);
    } else {
        err = DoDVDBurn(devPath, burnOptions, isDiskEmpty);
    }
    if (err != E_OK) {
        LOGE("Burn:<<< EXIT FAILED <<< devPath:=%{public}s", devPath.c_str());
        return err;
    }
    LOGI("Burn:<<< EXIT SUCCESS <<< devPath=%{public}s", devPath.c_str());
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS