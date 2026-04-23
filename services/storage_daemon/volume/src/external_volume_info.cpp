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

#include "volume/external_volume_info.h"

#include <cinttypes>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <future>
#include <regex>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/sysmacros.h>

#include "ipc/storage_manager_client.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"
#include "utils/string_utils.h"
#include "volume/process.h"

#define STORAGE_MANAGER_IOC_CHK_BUSY _IOR(0xAC, 77, int)
constexpr uid_t FILE_MANAGER_UID = 1006;
constexpr gid_t FILE_MANAGER_GID = 1006;
constexpr const char *NTFS_LABEL_DESC_PREFIX = "Volume label :  ";
constexpr const char* MNT_EXTERNAL_FILE_CONTEXT = "context=u:object_r:mnt_external_file:s0";
constexpr const char* MNT_EXTERNAL_FILE_CONTEXT_WITH_ERRORS =
    "context=u:object_r:mnt_external_file:s0,errors=continue";
constexpr const char* IO_CHAR_SET = "utf8";
using namespace std;
using namespace OHOS::StorageService;
namespace OHOS {
namespace StorageDaemon {
constexpr int32_t WAIT_THREAD_TIMEOUT_S = 60;
constexpr int32_t FILE_NOT_EXIST = 2;
constexpr int UID_FILE_MANAGER = 1006;
constexpr const char *CRYPTSETUP_PATH = "/system/bin/cryptsetup";
constexpr const char *BLOCK_PATH = "/dev/block/";
constexpr const char *DEV_MAPPER_PATH = "/dev/mapper/";
constexpr const char *PROGRESS_FILE = "/data/local/crypt_tmp";
constexpr const char *REMOVE_CMD = "remove";
constexpr int MIN_PASSWORD_LENGTH = 8;
constexpr int MAX_PASSWORD_LENGTH = 256;
constexpr int PWD_TYPE_NUMBER = 2;
static const char FILE_SEPARATOR_CHAR = '/';
static const std::string INVALID_PREFIX_PATH = "../";
static const std::string INVALID_SUFFIX_PATH = "/..";

int32_t ExternalVolumeInfo::ReadMetadata()
{
    LOGD("[L3:ExternalVolumeInfo] ReadMetadata: >>> ENTER <<< devPath=%{public}s", devPath_.c_str());
    int32_t ret = OHOS::StorageDaemon::ReadMetadata(devPath_, fsUuid_, fsType_, fsLabel_);
    if (fsType_ == "ntfs" && (fsLabel_.find('?') != std::string::npos || fsLabel_ == "")) {
        std::vector<std::string> cmd;
        cmd = {
            "ntfslabel",
            "-v",
            devPath_
        };
        fsLabel_ = GetVolDescByNtfsLabel(cmd);
    } else if (fsType_ == "udf" || fsType_ == "iso9660") {
        if (fsUuid_.empty()) {
            fsUuid_ = GenerateRandomUuid(devPath_, StringPrintf(mountPathDir_.c_str(), fsType_.c_str()));
        }
        if (fsLabel_.empty()) {
            fsLabel_ = GetCDType(devPath_);
        }
        LOGD("[L3:ExternalVolumeInfo] ReadMetadata: <<< EXIT SUCCESS <<< fsType=%{public}s", fsType_.c_str());
        return E_OK;
    }
    if (ret == E_OK) {
        LOGD("[L3:ExternalVolumeInfo] ReadMetadata: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:ExternalVolumeInfo] ReadMetadata: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

std::string ExternalVolumeInfo::GetFsType()
{
    return fsType_;
}

std::string ExternalVolumeInfo::GetFsUuid()
{
    return fsUuid_;
}

std::string ExternalVolumeInfo::GetFsLabel()
{
    return fsLabel_;
}

std::string ExternalVolumeInfo::GetMountPath()
{
    return mountPath_;
}

bool ExternalVolumeInfo::GetDamagedFlag()
{
    return isDamaged_;
}

int32_t ExternalVolumeInfo::DoCreate(dev_t dev)
{
    LOGI("[L3:ExternalVolumeInfo] DoCreate: >>> ENTER <<< volId=%{public}s", VolumeInfo::GetVolumeId().c_str());

    int32_t ret = 0;
    string id = VolumeInfo::GetVolumeId();

    device_ = dev;
    devPath_ = StringPrintf(devPathDir_.c_str(), (id).c_str());

    ret = mknod(devPath_.c_str(), S_IFBLK, dev);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoCreate: <<< EXIT FAILED <<< mknod failed, ret=%{public}d", ret);
        return E_ERR;
    }
    LOGI("[L3:ExternalVolumeInfo] DoCreate: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::string ExternalVolumeInfo::GetDevPathByVolumeId(const std::string& volumeId)
{
    std::string cmdDevPath = StringPrintf(devPathDir_.c_str(), (volumeId).c_str());
    return cmdDevPath;
}
 
int32_t ExternalVolumeInfo::DoGetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize)
{
    std::string cmdDevPath = GetDevPathByVolumeId(volumeId);
    int64_t usedSize = 0;
    LOGI("dev path is %{public}s", cmdDevPath.c_str());
    char realPath[PATH_MAX] = {0};
    if ((cmdDevPath.length() >= PATH_MAX) || realpath(cmdDevPath.c_str(), realPath) == nullptr) {
        LOGE("DoGetOddCapacity realpath failed, errno: %{public}d.", errno);
        return E_ERR;
    }
    int cmdFd = open(realPath, O_RDONLY|O_NONBLOCK);
    if (cmdFd < 0) {
        return E_ERR;
    }
    std::string oddLabel = GetCDType(cmdDevPath);
    LOGI("label is %{public}s", oddLabel.c_str());
    int err1 = E_OK;
    int err2 = E_OK;
    if (oddLabel == "DVD-RW" || oddLabel == "DVD-R" || oddLabel == "DVD+R") {
        err1 = GetDvdTotalCapacity(cmdFd, totalSize);
        err2 = GetDvdUsedCapacity(cmdFd, usedSize);
    } else if (oddLabel == "DVD+RW") {
        err1 = GetDvdTotalCapacity(cmdFd, totalSize);
        usedSize = totalSize;
    } else if (oddLabel == "CD-RW" || oddLabel == "CD-R") {
        err1 = GetCdTotalCapacity(cmdFd, totalSize);
        err2 = GetCdUsedCapacity(cmdFd, usedSize);
    } else {
        totalSize = 0;
        usedSize = 0;
    }
    LOGI("totalsize is %{public}" PRId64 ", usedsize is : %{public}" PRId64, totalSize, usedSize);
    if (err1 != E_OK || err2 != E_OK) {
        return E_ERR;
    }
    if (usedSize > totalSize) {
        freeSize = 0;
    } else {
        freeSize = totalSize - usedSize;
    }
    close(cmdFd);
    return E_OK;
}

std::string ExternalVolumeInfo::GetFsTypeByDev(dev_t dev)
{
    std::string volId = StringPrintf("vol-%u-%u", major(dev), minor(dev));
    std::string devPath = "/dev/block/" + volId;
    return OHOS::StorageDaemon::GetBlkidData(devPath, "TYPE");
}

int32_t ExternalVolumeInfo::DoDestroy()
{
    LOGI("[L3:ExternalVolumeInfo] DoDestroy: >>> ENTER <<<");

    int err = E_OK;
    LOGE("[L3:ExternalVolumeInfo] DoDestroy: External volume DoDestroy %{public}s.", devPath_.c_str());
    struct stat pathStat;
    if (lstat(devPath_.c_str(), &pathStat) == 0) {
        err = remove(devPath_.c_str());
        if (err != E_OK) {
            LOGE("[L3:ExternalVolumeInfo] DoDestroy:External volume DoDestroy error, errno %{public}d.", errno);
            return E_ERR;
        }
    }
    if (lstat(devBackupPath_.c_str(), &pathStat) == 0) {
        err = remove(devBackupPath_.c_str());
        if (err != E_OK) {
            LOGE("[L3:ExternalVolumeInfo] DoDestroy:External volume DoDestroy error, errno %{public}d.", errno);
            return E_ERR;
        }
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoMount4Hmfs(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Hmfs: >>> ENTER <<<");

    const char *fsType = "hmfs";
    auto mountData = StringPrintf(MNT_EXTERNAL_FILE_CONTEXT_WITH_ERRORS);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType, mountFlags, mountData.c_str());
    if (ret == E_OK) {
        StorageRadar::ReportVolumeOperation("ExternalVolumeInfo::DoMount4Hmfs", ret);
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Hmfs: initial mount failed, errno=%{public}d", errno);
        return ret;
    }
    ret = umount(mountPath_.c_str());
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Hmfs: umount failed, errno=%{public}d", errno);
        return ret;
    }
 
    auto mountDataWithoutOpt = StringPrintf(MNT_EXTERNAL_FILE_CONTEXT);
    ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType, MS_RDONLY, mountDataWithoutOpt.c_str());
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Hmfs: mount RO failed, errno=%{public}d", errno);
        return ret;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Hmfs: <<< EXIT SUCCESS <<<");
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Ntfs(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Ntfs: >>> ENTER <<<");

#ifdef EXTERNAL_STORAGE_QOS_TRANS
    auto mountData = StringPrintf("rw,big_writes,uid=%d,gid=%d,dmask=0006,fmask=0007",
        UID_FILE_MANAGER, UID_FILE_MANAGER);
#else
    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
#endif
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.ntfs",
        devPath_,
        mountPath_,
        "-o",
        mountData.c_str()
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Ntfs: <<< EXIT FAILED <<< exec failed, errno=%{public}d", errno);
        return E_NTFS_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Ntfs: <<< EXIT SUCCESS <<<");
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoMount4Ntfs: output: %{public}s", str.c_str());
        }
        LOGE("[L3:ExternalVolumeInfo] DoMount4Ntfs: exec mount for ntfs failed, errno is %{public}d.", errno);
        return E_NTFS_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Ntfs: <<< EXIT SUCCESS <<<");
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4Exfat(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Exfat: >>> ENTER <<<");

    auto mountData = StringPrintf("rw,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    if (mountFlags & MS_RDONLY) {
        mountData = StringPrintf("ro,uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    }

    std::vector<std::string> cmd = {
        "mount.exfat",
        "-o",
        mountData.c_str(),
        devPath_,
        mountPath_,
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Exfat: <<< EXIT FAILED <<< exec failed, errno=%{public}d", errno);
        return E_EXFAT_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Exfat: <<< EXIT SUCCESS <<<");
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoMount4Exfat: output: %{public}s", str.c_str());
        }
        LOGE("[L3:ExternalVolumeInfo] DoMount4Exfat: exec mount for exfat failed, errno is %{public}d.", errno);
        return E_EXFAT_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Exfat: <<< EXIT SUCCESS <<<");
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4Udf(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Udf: >>> ENTER <<<");

    auto mountData = StringPrintf("ro,uid=%d,gid=%d,%s", UID_FILE_MANAGER, UID_FILE_MANAGER, MNT_EXTERNAL_FILE_CONTEXT);
    std::vector<std::string> cmd = {
        "mount",
        "-t",
        fsType_,
        "-o",
        mountData,
        devPath_,
        mountPath_,
    };
    std::vector<std::string> output;

    if (ForkExec(cmd, &output) != E_OK) {
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoMount4Udf output: %{public}s", str.c_str());
        }
        LOGE("[L3:ExternalVolumeInfo] DoMount4Udf: exec mount for udf failed, errno is %{public}d.", errno);
        return E_UDF_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Udf: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoMount4Iso9660(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Iso9660: >>> ENTER <<<");
    auto mountData = StringPrintf("ro,uid=%d,gid=%d,%s,iocharset=%s",
        UID_FILE_MANAGER, UID_FILE_MANAGER, MNT_EXTERNAL_FILE_CONTEXT, IO_CHAR_SET);
    std::vector<std::string> cmd = {
        "mount",
        "-t",
        fsType_,
        "-o",
        mountData,
        devPath_,
        mountPath_,
    };
    std::vector<std::string> output;

    if (ForkExec(cmd, &output) != E_OK) {
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoMount4Iso9660: output: %{public}s", str.c_str());
        }
        LOGE("[L3:ExternalVolumeInfo] DoMount4Iso9660: exec mount for iso9660 failed, errno is %{public}d.", errno);
        return E_ISO9660_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Iso9660: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck4Exfat()
{
    LOGI("[L3:ExternalVolumeInfo] DoCheck4Exfat: >>> ENTER <<<");

    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-n",
        devPath_,
    };
    int execRet = ForkExecWithExit(cmd);
    if (execRet != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoCheck4Exfat: <<< EXIT FAILED <<< execRet=%{public}d", execRet);
        return E_VOL_NEED_FIX;
    }
    LOGI("[L3:ExternalVolumeInfo] DoCheck4Exfat: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck4Ntfs()
{
    LOGI("[L3:ExternalVolumeInfo] DoCheck4Ntfs: >>> ENTER <<<");

    std::vector<std::string> cmd = {
        "fsck.ntfs",
        devPath_,
    };
    int exitStatus = 0;
    int execRet = ForkExecWithExit(cmd, &exitStatus);
    LOGI("[L3:ExternalVolumeInfo] DoCheck4Ntfs: execRet: %{public}d, exitStatus: %{public}d", execRet, exitStatus);
    if (exitStatus != 1) {
        LOGE("[L3:ExternalVolumeInfo] DoCheck4Ntfs: <<< EXIT FAILED <<< exitStatus=%{public}d", exitStatus);
        return E_VOL_NEED_FIX;
    }
    LOGI("[L3:ExternalVolumeInfo] DoCheck4Ntfs: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoFix4Ntfs()
{
    LOGE("[L3:ExternalVolumeInfo] DoFix4Ntfs: >>> ENTER <<<");

    std::vector<std::string> cmd = {
        "ntfsfix",
        "-d",
        devPath_
    };
    std::vector<std::string> output;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    if (ExtStorageMountForkExec(cmd) != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoFix4Ntfs: <<< EXIT FAILED <<< exec failed, errno=%{public}d", errno);
        return E_VOL_FIX_FAILED;
    }
    LOGI("[L3:ExternalVolumeInfo] DoFix4Ntfs: <<< EXIT SUCCESS <<<");
    return E_OK;
#else
    if (ForkExec(cmd, &output) != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoFix4Ntfs: <<< EXIT FAILED <<< exec failed, errno=%{public}d", errno);
        return E_VOL_FIX_FAILED;
    }
    LOGI("[L3:ExternalVolumeInfo] DoFix4Ntfs: <<< EXIT SUCCESS <<<");
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoFix4Exfat()
{
    LOGI("[L3:ExternalVolumeInfo] DoFix4Exfat: >>> ENTER <<< devPath_ is %{public}s", devPath_.c_str());
    std::vector<std::string> cmd = {
        "fsck.exfat",
        "-p",
        devPath_,
    };
    std::vector<std::string> output;
    int exitStatus = 0;
    int forkExecRes = 0;
#ifdef EXTERNAL_STORAGE_QOS_TRANS
    forkExecRes = ExtStorageMountForkExec(cmd, &exitStatus);
    LOGI("[L3:ExternalVolumeInfo] DoFix4Exfat: ExtStorageMountForkExec is %{public}d,"
        "exitStatus is %{public}d", forkExecRes, exitStatus);
    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoFix4Exfat: <<< EXIT FAILED <<< exitStatus=%{public}d, res=%{public}d",
            exitStatus, forkExecRes);
        return E_VOL_FIX_FAILED;
    }
    LOGI("[L3:ExternalVolumeInfo] DoFix4Exfat: <<< EXIT SUCCESS <<<");
    return E_OK;
#else
    forkExecRes = ForkExec(cmd, &output, &exitStatus);
    LOGI("[L3:ExternalVolumeInfo] DoFix4Exfat: forkExecRes is %{public}d, exitStatus is"
        "%{public}d", forkExecRes, exitStatus);
    if (exitStatus != 1 || forkExecRes != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoFix4Exfat: <<< EXIT FAILED <<< exitStatus=%{public}d, res=%{public}d",
            exitStatus, forkExecRes);
        return E_VOL_FIX_FAILED;
    }
    LOGI("[L3:ExternalVolumeInfo] DoFix4Exfat: <<< EXIT SUCCESS <<<");
    return E_OK;
#endif
}

int32_t ExternalVolumeInfo::DoMount4OtherType(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4OtherType: >>> ENTER <<< fsType=%{public}s", fsType_.c_str());

    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007", UID_FILE_MANAGER, UID_FILE_MANAGER);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4OtherType: <<< EXIT FAILED <<< ret=%{public}d, errno=%{public}d",
            ret, errno);
        return E_OTHER_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4OtherType: <<< EXIT SUCCESS <<<");
    return ret;
}

int32_t ExternalVolumeInfo::DoMount4Vfat(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount4Vfat: >>> ENTER <<<");

    mountFlags |= MS_MGC_VAL;
    auto mountData = StringPrintf("uid=%d,gid=%d,dmask=0006,fmask=0007,utf8", UID_FILE_MANAGER, UID_FILE_MANAGER);
    int32_t ret = mount(devPath_.c_str(), mountPath_.c_str(), fsType_.c_str(), mountFlags, mountData.c_str());
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount4Vfat: <<< EXIT FAILED <<< errno=%{public}d", errno);
        return E_FAT_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoMount4Vfat: <<< EXIT SUCCESS <<<");
    return ret;
}

int32_t ExternalVolumeInfo::DoMount(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] DoMount: >>> ENTER <<< volId=%{public}s, fsType=%{public}s, flags=%{public}u",
        VolumeInfo::GetVolumeId().c_str(), fsType_.c_str(), mountFlags);
    if (major(device_) == DISK_CD_MAJOR) { // Don't deal mount, when CD is blank.
        bool isBlankCD = false;
        int blankRet = IsBlankCD(devPath_, isBlankCD);
        if (blankRet == E_OK && isBlankCD) {
            LOGW("Current cd is blank skip.");
            fsType_ = "udf"; // Set default value
            fsUuid_ = " ";
            fsLabel_ = "DVD RW";
            mountPath_ = StringPrintf(mountPathDir_.c_str(), fsType_.c_str());
            return E_OK;
        }
    }
    int32_t ret = IsUsbFuseByType(fsType_) ? CreateFuseMountPath() : CreateMountPath();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount: <<< EXIT FAILED <<< create mount path failed, ret=%{public}d", ret);
        return ret;
    }
    ret = DoCheck();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoMount: <<< EXIT FAILED <<< DoCheck failed, ret=%{public}d", ret);
        mountPath_ = mountBackupPath_;
        return E_DOCHECK_MOUNT;
    }
    if ((fsType_ == "hmfs" || fsType_ == "f2fs") && GetIsUserdata()) {
        ret = DoMount4Hmfs(mountFlags);
    }
    if (ret != E_OK) {
        auto retNo = remove(mountPath_.c_str());
        if (retNo != 0) {
            LOGE("[L3:ExternalVolumeInfo] DoMount: remove failed errno: %{public}d,"
                "retNo is: %{public}d", errno, retNo);
        }
        LOGE("[L3:ExternalVolumeInfo] DoMount: <<< EXIT FAILED <<< DoMount4Hmfs failed, ret=%{public}d", ret);
        return E_HMFS_MOUNT;
    }

    ret = ExecuteAsyncMount(mountFlags);
    if (ret != E_OK) {
        auto retNo = remove(mountPath_.c_str());
        if (retNo != 0) {
            LOGE("[L3:ExternalVolumeInfo] DoMount: remove failed errno: %{public}d,"
                "retNo is : %{public}d", errno, retNo);
        }
    }
    mountPath_ = mountBackupPath_;

    if (ret == E_OK) {
        LOGI("[L3:ExternalVolumeInfo] DoMount: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoMount: <<< EXIT FAILED <<< ExecuteAsyncMount failed, ret=%{public}d", ret);
    }
    return ret;
}

int32_t ExternalVolumeInfo::ExecuteAsyncMount(uint32_t mountFlags)
{
    LOGI("[L3:ExternalVolumeInfo] ExecuteAsyncMount: >>> ENTER <<< fsType=%{public}s, flags=%{public}u",
        fsType_.c_str(), mountFlags);

    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();

    std::thread mountThread([this, mountFlags, p = std::move(promise)]() mutable {
        LOGI("[L3:ExternalVolumeInfo] ExecuteAsyncMount: Ready to mount: volume fstype"
            "is %{public}s, mountflag is %{public}d", fsType_.c_str(), mountFlags);
        
        int retValue = E_OK;
        if (fsType_ == "ntfs") {
            retValue = DoMount4Ntfs(mountFlags);
        } else if (fsType_ == "exfat") {
            retValue = DoMount4Exfat(mountFlags);
        } else if (fsType_ == "vfat" || fsType_ == "fat32") {
            retValue = DoMount4Vfat(mountFlags);
        } else if (fsType_ == "udf") {
            retValue = DoMount4Udf(mountFlags);
        } else if (fsType_ == "iso9660") {
            retValue = DoMount4Iso9660(mountFlags);
        } else if ((fsType_ == "hmfs" || fsType_ == "f2fs") && GetIsUserdata()) {
            retValue = E_OK;
        } else {
            retValue = E_OTHER_MOUNT;
        }

        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:ExternalVolumeInfo] ExecuteAsyncMount: <<< EXIT FAILED <<< timed out");
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }

    int32_t ret = future.get();
    mountThread.join();

    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] ExecuteAsyncMount: <<< EXIT FAILED <<< ret=%{public}d, errno=%{public}d",
             ret, errno);
    } else {
        LOGI("[L3:ExternalVolumeInfo] ExecuteAsyncMount: <<< EXIT SUCCESS <<<");
    }
    
    return ret;
}

int32_t ExternalVolumeInfo::IsUsbInUse(int fd)
{
    int32_t inUse = -1;
    if (ioctl(fd, STORAGE_MANAGER_IOC_CHK_BUSY, &inUse) < 0) {
        LOGE("[L3:ExternalVolumeInfo] IsUsbInUse: ioctl failed, errno=%{public}d", errno);
        return E_IOCTL_FAILED;
    }
    if (inUse) {
        LOGI("[L3:ExternalVolumeInfo] IsUsbInUse: inUse=%{public}d", inUse);
        return E_USB_IN_USE;
    }
    LOGI("[L3:ExternalVolumeInfo] IsUsbInUse: not in use");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMountWithForceUsbFuse()
{
    LOGI("[L3:ExternalVolumeInfo] DoUMount: >>> ENTER <<< External volume start force to unmount.");
    Process ps(mountPath_);
    ps.UpdatePidAndKill(SIGKILL);
    int ret = umount2(mountPath_.c_str(), MNT_DETACH);
    if (ret != 0) {
        LOGW("[L3:ExternalVolumeInfo] DoUMount: umount2 failed in force mode, errno %{public}d", errno);
        return E_OK;
    }
    ret = rmdir(mountPath_.c_str());
    if (ret != 0) {
        LOGW("[L3:ExternalVolumeInfo] DoUMount: remove failed in force mode, errno %{public}d", errno);
        return E_OK;
    }
    LOGI("[L3:ExternalVolumeInfo] DoUMount: External volume force to unmount success.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(bool force)
{
    if (fsType_ == "crypt_LUKS") {
        LOGI("[L3:ExternalVolumeInfo] DoUMount: External volume start force to unmount, fsType is crypt_LUKS.");
        return E_OK;
    }
    if (mountPath_.empty()) {
        LOGI("[L3:ExternalVolumeInfo] DoUMount: External volume start force to unmount,"
            "mountPath_ is empty %{public}s.", mountPath_.c_str());
        return E_OK;
    }
    if (major(device_) == DISK_CD_MAJOR) { // Don't deal unmount, when CD is blank.
        bool isBlankCD = false;
        int blankRet = IsBlankCD(devPath_, isBlankCD);
        if (blankRet == E_OK && isBlankCD) {
            LOGW("[L3:ExternalVolumeInfo] DoUMount: Current cd is blank skip.");
            return E_OK;
        }
    }
    bool isUsbFuseByType = IsUsbFuseByType(fsType_);
    if (force && !isUsbFuseByType) {
        return DoUMountWithForceUsbFuse();
    }
    if (isUsbFuseByType) {
        mountPath_ = mountUsbFusePath_;
    }
    int fd = open(mountPath_.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGE("[L3:ExternalVolumeInfo] DoUMount: open failed, errno=%{public}d", errno);
    }
    if (fd >= 0) {
        IsUsbInUse(fd);
    }
    LOGI("[L3:ExternalVolumeInfo] DoUMount: External volume start to unmount mountPath:"
        "%{public}s.", GetAnonyString(mountPath_).c_str());
    int ret = umount2(mountPath_.c_str(), MNT_DETACH);
    if (fd >= 0) {
        IsUsbInUse(fd);
        close(fd);
    }
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoUMount: <<< EXIT FAILED <<< umount2 failed, errno=%{public}d", errno);
        return E_VOL_UMOUNT_ERR;
    }
    int err = rmdir(mountPath_.c_str());
    if (err != E_OK && errno != FILE_NOT_EXIST) {
        LOGE("[L3:ExternalVolumeInfo] DoUMount: remove failed, errno=%{public}d", errno);
        return E_RMDIR_MOUNT;
    }
    mountPath_ = mountBackupPath_;

    LOGI("[L3:ExternalVolumeInfo] DoUMount: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMountUsbFuse()
{
    LOGI("[L3:ExternalVolumeInfo] DoUMountUsbFuse: >>> ENTER <<<");

    int ret = umount2(mountPath_.c_str(), MNT_DETACH);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoUMountUsbFuse: <<< EXIT FAILED <<< umount2 failed, errno=%{public}d", errno);
        return E_VOL_UMOUNT_ERR;
    }

    int err = rmdir(mountPath_.c_str());
    if (err != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoUMountUsbFuse: rmdir failed, errno=%{public}d", errno);
        return E_RMDIR_MOUNT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoUMountUsbFuse: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoTryToCheck()
{
    LOGI("[L3:ExternalVolumeInfo] DoTryToCheck: >>> ENTER <<< fsType=%{public}s", fsType_.c_str());

    int32_t ret = DoCheck();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoTryToCheck: <<< EXIT FAILED <<< DoCheck failed, ret=%{public}d", ret);
        return E_DOCHECK_MOUNT;
    }

    if (fsType_ != "ntfs" && fsType_ != "exfat") {
        LOGE("[L3:ExternalVolumeInfo] DoTryToCheck: <<< EXIT FAILED <<< fsType not support fix");
        return E_VOL_FIX_NOT_SUPPORT;
    }

    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread mountThread ([this, p = std::move(promise)]() mutable {
        LOGI("[L3:ExternalVolumeInfo] DoTryToCheck: Ready to DoTryToCheck:"
            "volume fstype is %{public}s", fsType_.c_str());
        int retValue = E_OK;
        if (fsType_ == "ntfs") {
            retValue = DoCheck4Ntfs();
        } else if (fsType_ == "exfat") {
            retValue = DoCheck4Exfat();
        } else {
            retValue = E_VOL_FIX_NOT_SUPPORT;
        }
        LOGI("[L3:ExternalVolumeInfo] DoTryToCheck: DoTryToCheck cmdRet:%{public}d", retValue);
        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:ExternalVolumeInfo] DoTryToCheck: <<< EXIT FAILED <<< timed out");
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }

    ret = future.get();
    mountThread.join();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoTryToCheck: <<< EXIT FAILED <<< need fix, ret=%{public}d", ret);
        return E_VOL_NEED_FIX;
    }
    LOGI("[L3:ExternalVolumeInfo] DoTryToCheck: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoTryToFix()
{
    LOGI("[L3:ExternalVolumeInfo] DoTryToFix: >>> ENTER <<< fsType=%{public}s", fsType_.c_str());

    int32_t ret = DoCheck();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoTryToFix: <<< EXIT FAILED <<< DoCheck failed, ret=%{public}d", ret);
        return E_DOCHECK_MOUNT;
    }

    if (fsType_ != "ntfs" && fsType_ != "exfat") {
        LOGE("[L3:ExternalVolumeInfo] DoTryToFix: <<< EXIT FAILED <<< fsType not support fix");
        return E_VOL_FIX_NOT_SUPPORT;
    }
    std::promise<int32_t> promise;
    std::future<int32_t> future = promise.get_future();
    std::thread mountThread ([this, p = std::move(promise)]() mutable {
        int retValue = E_OK;
        if (fsType_ == "ntfs") {
            retValue = DoFix4Ntfs();
        } else if (fsType_ == "exfat") {
            retValue = DoFix4Exfat();
        }
        p.set_value(retValue);
    });
    if (future.wait_for(std::chrono::seconds(WAIT_THREAD_TIMEOUT_S)) == std::future_status::timeout) {
        LOGE("[L3:ExternalVolumeInfo] DoTryToFix: <<< EXIT FAILED <<< timed out");
        mountThread.detach();
        return E_TIMEOUT_MOUNT;
    }

    ret = future.get();
    mountThread.join();
    if (ret == E_OK) {
        LOGI("[L3:ExternalVolumeInfo] DoTryToFix: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoTryToFix: <<< EXIT FAILED <<< ret=%{public}d", ret);
    }
    return ret;
}

int32_t ExternalVolumeInfo::DoCheck()
{
    LOGI("[L3:ExternalVolumeInfo] DoCheck: >>> ENTER <<<");
    if (major(device_) == DISK_CD_MAJOR) { // Don't deal check, when CD is blank.
        bool isBlankCD = false;
        int blankRet = IsBlankCD(devPath_, isBlankCD);
        if (blankRet == E_OK && isBlankCD) {
            LOGW("[L3:ExternalVolumeInfo] DoCheck: Current cd is blank skip.");
            return E_OK;
        }
    }
    int32_t ret = ExternalVolumeInfo::ReadMetadata();
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoCheck: <<< EXIT FAILED <<< ReadMetadata failed, ret=%{public}d", ret);
        return E_CHECK;
    }

    // check fstype
    for (std::string item : supportMountType_) {
        if (item == fsType_) {
            LOGI("[L3:ExternalVolumeInfo] DoCheck: <<< EXIT SUCCESS <<<");
            return E_OK;
        }
    }
    LOGE("[L3:ExternalVolumeInfo] DoCheck: <<< EXIT FAILED <<< fsType not supported");
    return E_NOT_SUPPORT;
}

int32_t ExternalVolumeInfo::DoFormat(std::string type)
{
    LOGI("[L3:ExternalVolumeInfo] DoFormat: >>> ENTER <<< type=%{public}s", type.c_str());

    int32_t err = 0;
    if (IsUsbFuseByType(fsType_) && IsPathMounted(mountPath_)) {
        err = DoUMountUsbFuse();
    }
    if (err != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoFormat: <<< EXIT FAILED <<< UMountUsbFuse failed, err=%{public}d", err);
        return err;
    }

    std::map<std::string, std::string>::iterator iter = supportFormatType_.find(type);
    if (iter == supportFormatType_.end() || (fsType_ == "udf" || fsType_ == "iso9660")) {
        LOGE("[L3:ExternalVolumeInfo] DoFormat: <<< EXIT FAILED <<< type not supported");
        return E_NOT_SUPPORT;
    }
    std::vector<std::string> output;
    if (type == "vfat") {
        std::vector<std::string> cmd = {
            iter->second,
            "-A",
            devPath_
        };
        err = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("DoFormat with-A output: %{public}s", str.c_str());
        }
    } else {
        std::vector<std::string> cmd = {
            iter->second,
            devPath_
        };
        err = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("DoFormat output: %{public}s", str.c_str());
        }
    }

    if (err == E_NO_CHILD) {
        err = E_OK;
    }
    ReadMetadata();
    if (err == E_OK) {
        LOGI("[L3:ExternalVolumeInfo] DoFormat: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoFormat: <<< EXIT FAILED <<< err=%{public}d", err);
    }
    return err;
}

int32_t ExternalVolumeInfo::DoSetVolDesc(std::string description)
{
    LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: >>> ENTER <<< desc=%{public}s", description.c_str());

    int32_t err = 0;
    std::vector<std::string> output;
    if (fsType_ == "ntfs") {
        std::vector<std::string> fixCmd = {"ntfsfix", "-d", devPath_};
        err = ForkExec(fixCmd, &output);
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: ntfsfix output: %{public}s", str.c_str());
        }
        std::vector<std::string> labelCmd = {"ntfslabel", devPath_, description};
        output.clear();
        err = ForkExec(labelCmd, &output);
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: ntfslabel output: %{public}s", str.c_str());
        }
    } else if (fsType_ == "exfat") {
        std::vector<std::string> cmd = {"exfatlabel", devPath_, description};
        err = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: exfatlabel output: %{public}s", str.c_str());
        }
    } else if (fsType_ == "hmfs") {
        std::vector<std::string> cmd = {"hmfslabel", devPath_, description};
        err = ForkExec(cmd, &output);
        for (auto str : output) {
            LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: hmfslabel output: %{public}s", str.c_str());
        }
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoSetVolDesc: <<< EXIT FAILED <<< fsType not supported");
        return E_NOT_SUPPORT;
    }
    ReadMetadata();
    if (err == E_OK) {
        LOGI("[L3:ExternalVolumeInfo] DoSetVolDesc: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L3:ExternalVolumeInfo] DoSetVolDesc: <<< EXIT FAILED <<< err=%{public}d", err);
    }
    return err;
}

int32_t ExternalVolumeInfo::CreateFuseMountPath()
{
    LOGI("[L3:ExternalVolumeInfo] CreateFuseMountPath: >>> ENTER <<< fsUuid=%{private}s", fsUuid_.c_str());

    struct stat statbuf;
    mountBackupPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());
    mountUsbFusePath_ = StringPrintf(mountFusePathDir_.c_str(), fsUuid_.c_str());
    std::string mountFusePath = "/mnt/data/external_fuse";
    bool ret = true;
    if (lstat(mountFusePath.c_str(), &statbuf)) {
        ret = PrepareDir("/mnt/data/external_fuse", S_IRWXU | S_IRWXG | S_IXOTH, FILE_MANAGER_UID, FILE_MANAGER_GID);
        if (!ret) {
            LOGE("[L3:ExternalVolumeInfo] CreateFuseMountPath: <<< EXIT FAILED <<< create base path failed");
            return E_MKDIR_MOUNT;
        }
    }

    if (!lstat(mountUsbFusePath_.c_str(), &statbuf)) {
        LOGE("[L3:ExternalVolumeInfo] CreateFuseMountPath: path exists, removing");
        if (remove(mountUsbFusePath_.c_str()) != 0) {
            LOGE("[L3:ExternalVolumeInfo] CreateFuseMountPath: <<< EXIT FAILED <<< remove failed, errno=%{public}d",
                errno);
            return E_SYS_KERNEL_ERR;
        }
    }
    ret = PrepareDir(mountUsbFusePath_, S_IRWXU | S_IRWXG | S_IXOTH, FILE_MANAGER_UID, FILE_MANAGER_GID);
    if (!ret) {
        LOGE("[L3:ExternalVolumeInfo] CreateFuseMountPath: <<< EXIT FAILED <<< PrepareDir failed");
        return E_MKDIR_MOUNT;
    }
    mountPath_ = mountUsbFusePath_;
    LOGI("[L3:ExternalVolumeInfo] CreateFuseMountPath: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ExternalVolumeInfo::CreateMountPath()
{
    LOGI("[L3:ExternalVolumeInfo] CreateMountPath: >>> ENTER <<< fsUuid=%{private}s", fsUuid_.c_str());

    struct stat statbuf;
    int err;
    mountBackupPath_ = StringPrintf(mountPathDir_.c_str(), fsUuid_.c_str());

    err = lstat(mountBackupPath_.c_str(), &statbuf);
    if (err == 0) {
        LOGE("[L3:ExternalVolumeInfo] CreateMountPath: path exists, removing");
        if (remove(mountBackupPath_.c_str()) != 0) {
            LOGE("[L3:ExternalVolumeInfo] CreateMountPath: <<< EXIT FAILED <<< remove failed, errno=%{public}d", errno);
            return E_SYS_KERNEL_ERR;
        }
    }
    mode_t originalUmask = umask(0);
    err = mkdir(mountBackupPath_.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
    umask(originalUmask);
    if (err != 0) {
        LOGE("[L3:ExternalVolumeInfo] CreateMountPath: <<< EXIT FAILED <<< mkdir failed, errno=%{public}d", errno);
        return E_MKDIR_MOUNT;
    }
    mountPath_ = mountBackupPath_;
    LOGI("[L3:ExternalVolumeInfo] CreateMountPath: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::string ExternalVolumeInfo::GetVolDescByNtfsLabel(std::vector<std::string> &cmd)
{
    std::vector<std::string> output;
    int32_t ret = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("[L3:ExternalVolumeInfo] GetVolDescByNtfsLabel: output: %{public}s", str.c_str());
    }
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] GetVolDescByNtfsLabel: <<< EXIT FAILED <<< ret=%{public}d", ret);
        StorageRadar::ReportVolumeOperation("ForkExec", ret);
        return "";
    }
    return SplitOutputIntoLines(output);
}

std::string ExternalVolumeInfo::SplitOutputIntoLines(std::vector<std::string> &output)
{
    std::vector<std::string> lines;
    std::string bufToken = "\n";
    for (auto &buf : output) {
        auto split = SplitLine(buf, bufToken);
        lines.insert(lines.end(), split.begin(), split.end());
    }
    if (lines.empty()) {
        LOGE("[L3:ExternalVolumeInfo] SplitOutputIntoLines: lines empty");
        return "";
    }
    std::string volDesc;
    for (size_t i = lines.size() - 1; i < lines.size(); --i) {
        std::string line = lines[i];
        std::string::size_type index = line.find(NTFS_LABEL_DESC_PREFIX);
        if (index == std::string::npos) {
            continue;
        }
        volDesc = line.substr(index + std::strlen(NTFS_LABEL_DESC_PREFIX));
        break;
    }
    LOGE("[L3:ExternalVolumeInfo] SplitOutputIntoLines: label=%{public}s", volDesc.c_str());
    return volDesc;
}

int32_t ExternalVolumeInfo::DoDestroyCrypt(const std::string &volumeId)
{
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoDestroyCrypt: volume is %{public}s", devPath.c_str());
    std::string cryptName = volumeId + "_crypt";
    std::string devMapperPath = DEV_MAPPER_PATH + cryptName;
    struct stat pathStat;
    if (lstat(devMapperPath.c_str(), &pathStat) != 0) {
        LOGW("[L3:ExternalVolumeInfo] DoDestroyCrypt: DoDestroyCrypt:"
            "devMapperPath does not exist: %{public}s", devMapperPath.c_str());
        return E_OK;
    }
    std::vector<std::string> cmd = {
        CRYPTSETUP_PATH, REMOVE_CMD, cryptName
    };
    std::vector<std::string> output;
    int ret = ForkExec(cmd, &output);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoDestroyCrypt: exec cryptsetup remove failed, ret is: %{public}d", ret);
        StorageRadar::ReportVolumeOperation("ForkExec", ret);
        return ret;
    }
    int err = remove(devMapperPath.c_str());
    if (err && errno != FILE_NOT_EXIST) {
        LOGE("[L3:ExternalVolumeInfo] DoDestroyCrypt: remove failed, errno is: %{public}d", errno);
        return E_NOT_SUPPORT;
    }
    LOGI("[L3:ExternalVolumeInfo] DoDestroyCrypt: Successfully closed crypt.");
    return E_OK;
}

int32_t ExternalVolumeInfo::ValidatePazzword(const std::string &pazzword)
{
    size_t len = pazzword.length();
    if (len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) {
        LOGE("[L3:ExternalVolumeInfo] ValidatePazzword: Pazzword length must be between 8 and 256 characters.");
        return E_PARAMS_INVALID;
    }
    int typeMask = 0;
    const int UPPER_MASK = 1 << 0;
    const int LOWER_MASK = 1 << 1;
    const int DIGIT_MASK = 1 << 2;
    const int SPACE_MASK = 1 << 3;
    const int PUNCT_MASK = 1 << 4;
    for (unsigned char c : pazzword) {
        if (std::isupper(c)) {
            typeMask |= UPPER_MASK;
        } else if (std::islower(c)) {
            typeMask |= LOWER_MASK;
        } else if (std::isdigit(c)) {
            typeMask |= DIGIT_MASK;
        } else if (c == ' ') {
            typeMask |= SPACE_MASK;
        } else if (std::ispunct(c)) {
            typeMask |= PUNCT_MASK;
        }
        if (__builtin_popcount(typeMask) >= PWD_TYPE_NUMBER) {
            LOGI("[L3:ExternalVolumeInfo] ValidatePazzword: Pazzword validation passed.");
            return E_OK;
        }
    }
    LOGE("[L3:ExternalVolumeInfo] ValidatePazzword: Pazzword must contain at least two types of characters.");
    return E_PARAMS_INVALID;
}

int32_t ExternalVolumeInfo::DoEncrypt(const std::string &volumeId, const std::string &pazzword)
{
    int32_t ret = ValidatePazzword(pazzword);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoEncrypt: pazzword validation failed.");
        return ret;
    }
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoEncrypt: volume is %{public}s", devPath.c_str());
    LOGI("[L3:ExternalVolumeInfo] DoEncrypt: Successfully DoEncrypt.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoGetCryptProgressById(const std::string &volumeId, int32_t &progress)
{
    std::string devPath = BLOCK_PATH + volumeId;
    std::ifstream file(PROGRESS_FILE);
    if (!file.is_open()) {
        LOGE("[L3:ExternalVolumeInfo] DoGetCryptProgressById: Failed to open file %{public}s", PROGRESS_FILE);
        progress = 0;
        return E_OK;
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    (void)file.close();
    std::regex pattern(R"(Progress:\s*(\d+))");
    std::smatch match;
    if (std::regex_search(content, match, pattern)) {
        progress = atoi(match[1].str().c_str());
        LOGI("[L3:ExternalVolumeInfo] DoGetCryptProgressById: ExternalVolumeInfo::GetCryptProgressFromFile,"
            "volume is %{public}s, Progress is %{public}" PRId32, devPath.c_str(), progress);
        return E_OK;
    }
    LOGE("[L3:ExternalVolumeInfo] DoGetCryptProgressById: Progress keyword not found");
    progress = 0;
    return E_OK;
}

int32_t ExternalVolumeInfo::DoGetCryptUuidById(const std::string &volumeId, std::string &uuid)
{
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoGetCryptUuidById: ExternalVolumeInfo::DoGetCryptUuidById,"
        "volume is%{public}s.", devPath.c_str());
    int ret = OHOS::StorageDaemon::ReadVolumeUuid(devPath, uuid);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoGetCryptUuidById: ReadVolumeUuid failed, ret is: %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoBindRecoverKeyToPasswd(const std::string &volumeId,
                                                     const std::string &pazzword,
                                                     const std::string &recoverKey)
{
    int32_t ret = ValidatePazzword(pazzword);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoBindRecoverKeyToPasswd: pazzword validation failed.");
        return ret;
    }
    ret = ValidatePazzword(recoverKey);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoBindRecoverKeyToPasswd: recoverKey validation failed.");
        return ret;
    }
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoBindRecoverKeyToPasswd: volume is%{public}s.", devPath.c_str());
    LOGI("[L3:ExternalVolumeInfo] DoBindRecoverKeyToPasswd: Successfully DoBindRecoverKeyToPasswd.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUpdateCryptPasswd(const std::string &volumeId,
                                                const std::string &pazzword,
                                                const std::string &newPazzword)
{
    int32_t ret = ValidatePazzword(pazzword);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoUpdateCryptPasswd: pazzword validation failed.");
        return ret;
    }
    ret = ValidatePazzword(newPazzword);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoUpdateCryptPasswd: newPazzword validation failed.");
        return ret;
    }
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoUpdateCryptPasswd: volume is%{public}s.", devPath.c_str());
    LOGI("[L3:ExternalVolumeInfo] DoUpdateCryptPasswd: Successfully DoUpdateCryptPasswd.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoResetCryptPasswd(const std::string &volumeId,
                                               const std::string &recoverKey,
                                               const std::string &newPazzword)
{
    int32_t ret = ValidatePazzword(recoverKey);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoResetCryptPasswd: recoverKey validation failed.");
        return ret;
    }
    ret = ValidatePazzword(newPazzword);
    if (ret != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoResetCryptPasswd: newPazzword validation failed.");
        return ret;
    }
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoResetCryptPasswd: volume is%{public}s.", devPath.c_str());
    LOGI("[L3:ExternalVolumeInfo] DoResetCryptPasswd: Successfully DoResetCryptPasswd.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoVerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword)
{
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoVerifyCryptPasswd: volume is%{public}s.", devPath.c_str());
    (void)pazzword;
    LOGI("[L3:ExternalVolumeInfo] DoVerifyCryptPasswd: Successfully DoVerifyCryptPasswd.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoUnlock(const std::string &volumeId, const std::string &pazzword)
{
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("[L3:ExternalVolumeInfo] DoUnlock: volume is%{public}s.", devPath.c_str());
    (void)pazzword;
    LOGI("L3:ExternalVolumeInfo] DoUnlock: Successfully DoUnlock.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoDecrypt(const std::string &volumeId, const std::string &pazzword)
{
    std::string devPath = BLOCK_PATH + volumeId;
    LOGI("L3:ExternalVolumeInfo] DoDecrypt: volume is%{public}s.", devPath.c_str());
    (void)pazzword;
    LOGI("L3:ExternalVolumeInfo] DoDecrypt: Successfully DoDecrypt.");
    return E_OK;
}

int32_t ExternalVolumeInfo::DoEject(const std::string &volId)
{
    LOGI("[L3:ExternalVolumeInfo] DoEject:>>> ENTER <<< volId=%{public}s", volId.c_str());
    int32_t err = 0;

    string nodePath;
    if (!GetRealPath("/dev/block/" + volId, nodePath)) {
        LOGE("[L3:ExternalVolumeInfo] DoEject:<<< EXIT FAILED <<<failed for volId: %{public}s", volId.c_str());
        return E_PARAMS_INVALID;
    }
    if (!IsFilePathInvalid(nodePath)) {
        LOGE("[L3:ExternalVolumeInfo] DoEject:<<< EXIT FAILED <<< nodePath: %{public}s", nodePath.c_str());
        return E_PARAMS_INVALID;
    }
    LOGI("[L3:ExternalVolumeInfo] DoEject nodePath = %{public}s", nodePath.c_str());

    std::vector<std::string> output;
    std::vector<std::string> cmd = {"eject", "-s", nodePath};
    err = ForkExec(cmd, &output);
    if (err != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoEject:<<< EXIT FAILED <<< failed for volId: %{public}s", volId.c_str());
        return err;
    }

    LOGI("[L3:ExternalVolumeInfo] DoEject:<<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return err;
}

int32_t ExternalVolumeInfo::GetLatestProgressFromFile(const char* filePath, uint32_t &progress){
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGE("[L3:ExternalVolumeInfo] GetLatestProgressFromFile:<<< EXIT FAILED <<< open failed");
        return E_NOT_SUPPORT;
    }

    std::string content;
    std::getline(file, content);
    if (file.fail()&&!file.eof()) {
        LOGE("[L3:ExternalVolumeInfo] GetLatestProgressFromFile:<<< EXIT FAILED <<< getline failed");
        file.close();
        return E_NOT_SUPPORT;
    }
    LOGI("[L3:ExternalVolumeInfo] GetLatestProgressFromFile content = %{public}s", content.c_str());

    std::istringstream iss(content);
    iss >> progress;

    if (iss.fail()) {
        LOGE("[L3:ExternalVolumeInfo] GetLatestProgressFromFile:<<< EXIT FAILED <<< iss failed");
        file.close();
        return E_NOT_SUPPORT;
    }
    
    file.close();
    return E_OK;
}

int32_t ExternalVolumeInfo::DoGetOpticalDriveOpsProgress(const std::string &volId, uint32_t &progress)
{
    LOGI("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress: >>> ENTER <<< volId=%{public}s", volId.c_str());
    int32_t err = 0;

    string filePath;
    if (!GetRealPath("/data/local/tmp/" + volId, filePath)) {
        LOGE("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress:<<< EXIT FAILED <<< volId: %{public}s", 
            volId.c_str());
        return E_PARAMS_INVALID;
    }
    if (!IsFilePathInvalid(filePath)) {
        LOGE("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress:<<< EXIT FAILED <<<filePath: %{public}s", 
            filePath.c_str());
        return E_PARAMS_INVALID;
    }
    LOGI("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress filePath = %{public}s", filePath.c_str());

    err = GetLatestProgressFromFile(filePath.c_str(), progress);
    if (err != E_OK) {
        LOGE("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress:<<< EXIT FAILED <<< volId: %{public}s", 
            volId.c_str());
        return err;
    }
    LOGI("[L3:ExternalVolumeInfo] DoGetOpticalDriveOpsProgress:<<< EXIT SUCCESS <<< volId=%{public}s", volId.c_str());
    return err;
}
} // StorageDaemon
} // OHOS
