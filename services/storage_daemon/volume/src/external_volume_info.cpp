/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <sys/mount.h>
#include <csignal>
#include <algorithm>
#include <sys/wait.h>
#include <cstring>
#include "securec.h"
#include "storage_service_log.h"
#include "storage_service_errno.h"
#include "utils/string_utils.h"
#include "volume/process.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {
std::string ExternalVolumeInfo::GetBlkidData(const std::string type, int32_t size)
{
    char s[size];
    int32_t len = 0;

    memset_s(s, size, 0, size);

    char *const argv1[] = {
        (char *)"blkid",
        (char *)"-s",
        (char *)(type.c_str()),
        (char *)"-o",
        (char *)"value",
        (char *)(devPath_.c_str()),
        (char *)NULL,
    };
    int32_t err = RunPopen("blkid", argv1, s, size, false);
    int32_t ret = GetExitErr(err);
    if (ret) {
        return "";
    }

    len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
    return string(s);
}

int32_t ExternalVolumeInfo::ReadMetadata()
{
    fsUuid_ = GetBlkidData("UUID", fsUuidLen_);
    fsType_ = GetBlkidData("TYPE", fsTypeLen_);
    fsLabel_ = GetBlkidData("LABEL", fsLabelLen_);

    if (fsUuid_.empty() || fsType_.empty()) {
        LOGE("External volume ReadMetadata error.");
        return E_ERR;
    }
    LOGI("ReadMetadata, fsUuid=%{public}s, fsType=%{public}d, fsLabel=%{public}s.",
         GetFsUuid().c_str(), GetFsType(), GetFsLabel().c_str());
    return E_OK;
}

int32_t ExternalVolumeInfo::GetFsType()
{
    for (uint32_t i = 0; i < supportMountType_.size(); i++) {
        if (supportMountType_[i] == fsType_) {
            return i;
        }
    }
    return -1;
}

std::string ExternalVolumeInfo::GetFsUuid()
{
    return fsUuid_;
}

std::string ExternalVolumeInfo::GetFsLabel()
{
    return fsLabel_;
}

int32_t ExternalVolumeInfo::DoCreate(dev_t dev)
{
    int32_t ret = 0;
    string id = VolumeInfo::GetVolumeId();

    device_ = dev;
    devPath_ = StringPrintf(devPathDir_.c_str(), (id).c_str());

    ret = mknod(devPath_.c_str(), S_IFBLK, dev);
    if (ret) {
        LOGE("External volume DoCreate error.");
        return E_ERR;
    }

    ret = ReadMetadata();
    if (ret) {
        LOGE("External volume ReadMetadata failed.");
        remove(devPath_.c_str());
        return E_ERR;
    }

    return E_OK;
}

int32_t ExternalVolumeInfo::DoDestroy()
{
    int err = remove(devPath_.c_str());
    if (err) {
        LOGE("External volume DoDestroy error.");
        return E_ERR;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::DoMount(const std::string mountPath, uint32_t mountFlags)
{
    int ret = mount(devPath_.c_str(), mountPath.c_str(), fsType_.c_str(), MS_MGC_VAL, "fmask=0000,dmask=0000");
    if (ret) {
        LOGE("External volume DoMount error.");
        return E_MOUNT;
    }

    return E_OK;
}

int32_t ExternalVolumeInfo::DoUMount(const std::string mountPath, bool force)
{
    if (force) {
        LOGI("External volume start force to unmount.");
        Process ps(mountPath);
        ps.UpdatePidByPath();
        ps.KillProcess(SIGKILL);
    }

    int ret = umount(mountPath.c_str());
    if (!force && ret) {
        LOGE("External volume DoUmount error.");
        return E_UMOUNT;
    }
    remove(mountPath.c_str());
    return E_OK;
}

int32_t ExternalVolumeInfo::DoCheck()
{
    int32_t ret = ExternalVolumeInfo::ReadMetadata();
    if (ret) {
        LOGE("External volume uuid=%{public}s DoCheck failed.", GetFsUuid().c_str());
        return E_ERR;
    }

    // check fstype
    if (GetFsType() == -1) {
        LOGE("External Volume type not support.");
        return E_NOT_SUPPORT;
    }
    return E_OK;
}

int32_t ExternalVolumeInfo::GetExitErr(int32_t status)
{
    if (status < 0) {
        LOGE("failed to get status.");
        return E_ERR;
    }

    if (WIFEXITED(status)) {
        int32_t ret = WEXITSTATUS(status);
        if (ret) {
            LOGE("normal termination.");
            return E_ERR;
        } else
            return E_OK;
    }

    if (WIFSIGNALED(status)) {
        LOGE("abnormal termination.");
        return E_ERR;
    }

    if (WIFSTOPPED(status)) {
        LOGE("process stopped.");
        return E_ERR;
    }

    LOGE("unknow error.");
    return E_ERR;
}

FILE *ExternalVolumeInfo::HandlePidForPopen(pid_t pid, int32_t *pfd, int32_t outsize, bool rw)
{
    FILE *fp = NULL;

    if (outsize == 0)
        return NULL;

    // child
    if (pid == 0) {
        if (rw) { // write
            close(pfd[1]);
            if (pfd[0] != STDIN_FILENO) {
                dup2(pfd[0], STDIN_FILENO);
                close(pfd[0]);
            }
        } else { // read
            close(pfd[0]);
            if (pfd[1] != STDOUT_FILENO) {
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[1]);
            }
        }
        return NULL;
    }

    // parent
    if (rw) { // write
        close(pfd[0]);
        if ((fp = fdopen(pfd[1], "w")) == NULL) {
            return NULL;
        }
    } else { // read
        close(pfd[1]);
        if ((fp = fdopen(pfd[0], "r")) == NULL) {
            return NULL;
        }
    }
    return fp;
}

int32_t ExternalVolumeInfo::RunPopen(const char *cmd, char *const argv[], char *out, int32_t outsize, bool rw)
{
    pid_t pid;
    int32_t status = 0;
    int32_t pfd[2];
    int32_t childFailExit = 127;

    if (pipe(pfd) < 0) {
        LOGE("create pipe failed.");
        return -1;
    }

    if ((pid = fork()) < 0) {
        LOGE("create fork failed.");
        status = -1;
    } else if (pid == 0) {
        // child
        HandlePidForPopen(pid, pfd, outsize, rw);

        execvp(cmd, argv);
        _exit(childFailExit);
    }

    // parent
    FILE *fp = HandlePidForPopen(pid, pfd, outsize, rw);
    if (fp) {
        fgets(out, outsize, fp);
        (void)fclose(fp);
    } else if (outsize) {
        return -1;
    }

    while (waitpid(pid, &status, 0) < 0) {
        if (errno == ECHILD) {
            break;
        }
        if (errno != EINTR) {
            LOGE("waitpid is interrupt failed.");
            status = -1;
            break;
        }
    }
    return status;
}

int32_t ExternalVolumeInfo::DoFormat(std::string type)
{
    int32_t err = 0;
    std::map<std::string, std::string>::iterator iter = supportFormatType_.find(type);
    if (iter == supportFormatType_.end()) {
        LOGE("External volume format not support.");
        return E_NOT_SUPPORT;
    }

    if (type == "ext2" || type == "ext3" || type == "ext4") {
        char *const argv[] = {
            (char *)iter->second.c_str(),
            (char *)"-F",
            (char *)"-t",
            (char *)(type.c_str()),
            (char *)(devPath_.c_str()),
            (char *)NULL,
        };
        err = RunPopen(iter->second.c_str(), argv, NULL, 0, true);
    } else {
        char *const argv[] = {
            (char *)iter->second.c_str(),
            (char *)(devPath_.c_str()),
            (char *)NULL,
        };
        err = RunPopen(iter->second.c_str(), argv, NULL, 0, true);
    }
    int32_t ret = GetExitErr(err);

    ReadMetadata();
    return ret;
}
} // StorageDaemon
} // OHOS
