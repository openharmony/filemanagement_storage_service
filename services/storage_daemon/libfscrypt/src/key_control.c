/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "key_control.h"

#include <bits/fcntl.h>
#include <ctype.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/keyctl.h>

#include "fscrypt_log.h"
#include "fscrypt_sysparam.h"
#include "init_utils.h"
#include "securec.h"

key_serial_t KeyCtrlGetKeyringId(key_serial_t id, int create)
{
    return syscall(__NR_keyctl, KEYCTL_GET_KEYRING_ID, id, create);
}

key_serial_t KeyCtrlAddKey(const char *type, const char *description,
    const key_serial_t ringId)
{
    return syscall(__NR_add_key, type, description, NULL, 0, ringId);
}

key_serial_t KeyCtrlAddKeyEx(const char *type, const char *description,
    struct fscrypt_key *fsKey, const key_serial_t ringId)
{
    return syscall(__NR_add_key, type, description,
        (void *)(fsKey), sizeof(struct fscrypt_key),
        ringId);
}

long KeyCtrlSearch(key_serial_t ringId, const char *type, const char *description,
    key_serial_t destRingId)
{
    return syscall(__NR_keyctl, KEYCTL_SEARCH, ringId, type,
        description, destRingId);
}

long KeyCtrlUnlink(key_serial_t key, key_serial_t keyring)
{
    return syscall(__NR_keyctl, KEYCTL_UNLINK, key, keyring);
}

static bool FsIoctl(const char *mnt, unsigned long cmd, void *arg)
{
    int fd = open(mnt, O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        FSCRYPT_LOGE("open %s failed, errno:%d", mnt, errno);
        return false;
    }
    if (ioctl(fd, cmd, arg) != 0) {
        FSCRYPT_LOGE("ioctl to %s failed, errno:%d", mnt, errno);
        (void)close(fd);
        return false;
    }
    (void)close(fd);
    FSCRYPT_LOGI("success");
    return true;
}

#ifdef SUPPORT_FSCRYPT_V2
bool KeyCtrlInstallKey(const char *mnt, struct fscrypt_add_key_arg *arg)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(mnt, FS_IOC_ADD_ENCRYPTION_KEY, (void *)(arg));
}

bool KeyCtrlRemoveKey(const char *mnt, struct fscrypt_remove_key_arg *arg)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(mnt, FS_IOC_REMOVE_ENCRYPTION_KEY, (void *)arg);
}

bool KeyCtrlGetKeyStatus(const char *mnt, struct fscrypt_get_key_status_arg *arg)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(mnt, FS_IOC_GET_ENCRYPTION_KEY_STATUS, (void *)(arg));
}

bool KeyCtrlGetPolicyEx(const char *path, struct fscrypt_get_policy_ex_arg *policy)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(path, FS_IOC_GET_ENCRYPTION_POLICY_EX, (void *)(policy));
}
#endif

bool KeyCtrlSetPolicy(const char *path, union FscryptPolicy *policy)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(path, FS_IOC_SET_ENCRYPTION_POLICY, (void *)(policy));
}

bool KeyCtrlGetPolicy(const char *path, struct fscrypt_policy *policy)
{
    FSCRYPT_LOGI("enter");
    return FsIoctl(path, FS_IOC_GET_ENCRYPTION_POLICY, (void *)(policy));
}

static uint8_t CheckKernelFscrypt(const char *mnt)
{
    int fd = open(mnt, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) {
        FSCRYPT_LOGE("open policy file failed, errno: %d", errno);
        return FSCRYPT_INVALID;
    }

#ifdef SUPPORT_FSCRYPT_V2
    errno = 0;
    (void)ioctl(fd, FS_IOC_ADD_ENCRYPTION_KEY, NULL);
    (void)close(fd);
    if (errno == EOPNOTSUPP) {
        FSCRYPT_LOGE("Kernel doesn't support fscrypt v1 or v2.");
        return FSCRYPT_INVALID;
    } else if (errno == ENOTTY) {
        FSCRYPT_LOGE("Kernel doesn't support fscrypt v2, pls use v1.");
        return FSCRYPT_V1;
    } else if (errno == EFAULT) {
        FSCRYPT_LOGI("Kernel is support fscrypt v2.");
        return FSCRYPT_V2;
    }
    FSCRYPT_LOGE("Unexpected errno: %d", errno);
    return FSCRYPT_INVALID;
#else
    (void)close(fd);
    return FSCRYPT_V1;
#endif
}

uint8_t KeyCtrlGetFscryptVersion(const char *mnt)
{
    uint8_t version = CheckKernelFscrypt(mnt);
    return version;
}

bool KeyCtrlHasFscryptSyspara(void)
{
    FSCRYPT_LOGI("enter");
    char tmp[POLICY_BUF_SIZE] = { 0 };
    uint32_t len = POLICY_BUF_SIZE;
    int ret = GetFscryptParameter(FSCRYPT_POLICY_KEY, "", tmp, &len);
    if (ret != 0) {
        FSCRYPT_LOGE("fscrypt config parameter not set, not enable fscrypt");
        return false;
    }

    return true;
}

uint8_t KeyCtrlLoadVersion(const char *keyPath)
{
    if (!keyPath) {
        FSCRYPT_LOGE("key path is null");
        return FSCRYPT_INVALID;
    }
    char pathLen = strlen(keyPath) + strlen(PATH_FSCRYPT_VER) + 1;
    char *path = (char *)malloc(pathLen);
    if (!path) {
        FSCRYPT_LOGE("no memory for full key path");
        return FSCRYPT_INVALID;
    }
    path[0] = '\0';
    if (strncat_s(path, pathLen - strlen(PATH_FSCRYPT_VER), keyPath, strlen(keyPath)) != EOK) {
        free(path);
        FSCRYPT_LOGE("KEY path strcat error");
        return FSCRYPT_INVALID;
    }
    if (strncat_s(path, pathLen, PATH_FSCRYPT_VER, strlen(PATH_FSCRYPT_VER)) != EOK) {
        free(path);
        FSCRYPT_LOGE("version path strcat error");
        return FSCRYPT_INVALID;
    }

    char *buf = ReadFileToBuf(path);
    free(path);
    if (!buf) {
        FSCRYPT_LOGE("read fscrypt version file failed");
        return FSCRYPT_INVALID;
    }
    if (isdigit(*buf)) {
        int ver = atoi(buf);
        if (ver == FSCRYPT_V1 || ver == FSCRYPT_V2) {
            free(buf);
            FSCRYPT_LOGI("version %d loaded", ver);
            return ver;
        }
    }
    free(buf);

    FSCRYPT_LOGE("bad version content");
    return FSCRYPT_INVALID;
}
