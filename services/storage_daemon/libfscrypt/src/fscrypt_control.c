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
#include "fscrypt_control.h"

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "fscrypt_log.h"
#include "fscrypt_sysparam.h"
#include "init_utils.h"
#include "key_control.h"
#include "securec.h"
#include <sys/ioctl.h>

#ifdef SUPPORT_RECOVERY_KEY_SERVICE
#include "fscrypt_enable.h"
#endif

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
#define SAFE_FREE_PTR(ptr) do { \
    if ((ptr) != NULL) { \
        free(ptr); \
        (ptr) = NULL; \
    } \
} while (0)

typedef struct FscrtpyItem_ {
    char *key;
    uint8_t value;
}FscrtpyItem;

typedef struct EncryptPolicy_ {
    uint8_t version;
    uint8_t fileName;
    uint8_t content;
    uint8_t flags;
    bool hwWrappedKey;
}EncryptPolicy;

static EncryptPolicy g_fscryptPolicy = {
    FSCRYPT_V2,
    FSCRYPT_MODE_AES_256_CTS,
    FSCRYPT_MODE_AES_256_XTS,
    FSCRYPT_POLICY_FLAGS_PAD_32,
    false
};

enum FscryptOptins {
    FSCRYPT_VERSION_NUM = 0,
    FSCRYPT_FILENAME_MODE,
    FSCRYPT_CONTENT_MODE,
    FSCRYPT_OPTIONS_MAX,
};

static const FscrtpyItem ALL_VERSION[] = {
    {"1", FSCRYPT_V1},
    {"2", FSCRYPT_V2},
};

static const FscrtpyItem FILENAME_MODES[] = {
    {"aes-256-cts", FSCRYPT_MODE_AES_256_CTS},
};

static const FscrtpyItem CONTENTS_MODES[] = {
    {"aes-256-xts", FSCRYPT_MODE_AES_256_XTS},
};

static bool g_fscryptEnabled = false;
static bool g_fscryptInited = false;

static const char *GLOBAL_FSCRYPT_DIR[] = {
    "/data/app/el1/bundle/public",
    "/data/service/el1/public",
    "/data/chipset/el1/public",
};

static const char *DEVICE_EL1_DIR = "/data/service/el0/storage_daemon/sd";
static const char *PATH_KEYDESC = "/key_desc";
#ifdef SUPPORT_FSCRYPT_V2
static const char *PATH_KEYID = "/key_id";
#endif

static bool IsSupportedPolicyKey(const char *key,
                                 const FscrtpyItem *items,
                                 size_t len)
{
    for (size_t i = 0 ; i < len; i++) {
        if (strncmp(key, items[i].key, strlen(items[i].key)) == 0) {
            return true;
        }
    }
    return false;
}

static bool IsSupportedPolicy(const char *policy,
                              enum FscryptOptins number)
{
    if ((number >= FSCRYPT_OPTIONS_MAX) || (!policy)) {
        return false;
    }

    switch (number) {
        case FSCRYPT_VERSION_NUM:
            return IsSupportedPolicyKey(policy, ALL_VERSION,
                ARRAY_LEN(ALL_VERSION));
        case FSCRYPT_FILENAME_MODE:
            return IsSupportedPolicyKey(policy, FILENAME_MODES,
                ARRAY_LEN(FILENAME_MODES));
        case FSCRYPT_CONTENT_MODE:
            return IsSupportedPolicyKey(policy, CONTENTS_MODES,
                ARRAY_LEN(CONTENTS_MODES));
        default:
            return false;
    }
    return false;
}

int FscryptSetSysparam(const char *policy)
{
    if (!policy) {
        return -EINVAL;
    }

    char *tmp = strdup(policy);
    if (!tmp) {
        LOGE("No memory for policy option");
        return -ENOMEM;
    }
    int optNums = 0;
    char **options = SplitStringExt(tmp, ":", &optNums, FSCRYPT_OPTIONS_MAX);
    free(tmp);
    if (!options) {
        return -ENOMEM;
    }
    if (optNums != FSCRYPT_OPTIONS_MAX) {
        LOGE("Mount fscrypt option setting error, not enabled crypto!");
        FreeStringVector(options, optNums);
        return -EINVAL;
    }

    // check supported policy
    for (enum FscryptOptins i = FSCRYPT_VERSION_NUM; i < FSCRYPT_OPTIONS_MAX; i++) {
        char *temp = options[i];
        if (!IsSupportedPolicy(temp, i)) {
            LOGE("Not supported policy, %{public}s", temp);
            FreeStringVector(options, optNums);
            return -ENOTSUP;
        }
    }
    FreeStringVector(options, optNums);

    int ret = SetFscryptParameter(FSCRYPT_POLICY_KEY, policy);
    if (ret < 0) {
        LOGE("Set fscrypt system parameter failed %{public}d", ret);
        return ret;
    }
    g_fscryptEnabled = true;
    g_fscryptInited = false; // need to re-init

    return 0;
}

#ifdef USER_CRYPTO_MANAGER
static void PraseOnePloicyValue(uint8_t *value, const char *key,
                                const FscrtpyItem *table, size_t numbers)
{
    for (size_t i = 0; i < numbers; i++) {
        size_t len = strlen(table[i].key);
        if (strncmp(key, table[i].key, len) == 0 &&
            strlen(key) == len) {
            *value = table[i].value;
            return;
        }
    }
    LOGE("Have not found value for the key!");
}
#endif

int InitFscryptPolicy(void)
{
#ifdef USER_CRYPTO_MANAGER
    if (g_fscryptInited) {
        LOGI("Have been init");
        return 0;
    }
    char policy[POLICY_BUF_SIZE];
    uint32_t len = POLICY_BUF_SIZE - 1;
    int ret = GetFscryptParameter(FSCRYPT_POLICY_KEY, "", policy, &len);
    if (ret != 0) {
        LOGI("Get fscrypt policy failed");
        return -ENOTSUP;
    }
    int count = 0;
    char **option = SplitStringExt(policy, ":", &count, FSCRYPT_OPTIONS_MAX);
    if (!option) {
        LOGE("Fscrypt setting error");
        return -ENOTSUP;
    }
    if (count != FSCRYPT_OPTIONS_MAX) {
        LOGE("Fscrypt policy count error");
        FreeStringVector(option, count);
        return -ENOTSUP;
    }

    PraseOnePloicyValue(&g_fscryptPolicy.version, option[FSCRYPT_VERSION_NUM],
        ALL_VERSION, ARRAY_LEN(ALL_VERSION));
    PraseOnePloicyValue(&g_fscryptPolicy.fileName, option[FSCRYPT_FILENAME_MODE],
        FILENAME_MODES, ARRAY_LEN(FILENAME_MODES));
    PraseOnePloicyValue(&g_fscryptPolicy.content, option[FSCRYPT_CONTENT_MODE],
        CONTENTS_MODES, ARRAY_LEN(CONTENTS_MODES));

    FreeStringVector(option, count);
    g_fscryptInited = true;
    LOGI("Fscrypt policy init success");
#endif

    return 0;
}

/*
 * Splic full path, Caller need to free *buf after using
 * if return success.
 *
 * @path: base key path
 * @name: fscrypt file, so as /key_id
 * @buf: splic result if return 0
 */
static int SpliceKeyPath(const char *path, size_t pathLen,
                         const char *name, size_t nameLen,
                         char **buf)
{
    LOGI("key path %{public}s, name %{public}s", path, name);
    *buf = NULL;
    size_t bufMax = pathLen + nameLen + 1;
    char *tmpBuf = (char *)malloc(bufMax);
    if (!tmpBuf) {
        LOGE("No memory for fscrypt v1 path buffer");
        return -ENOMEM;
    }
    tmpBuf[0] = '\0';

    int ret = strncat_s(tmpBuf, bufMax, path, pathLen);
    if (ret != 0) {
        free(tmpBuf);
        LOGE("splic previous path error");
        return ret;
    }
    ret = strncat_s(tmpBuf, bufMax, name, nameLen);
    if (ret != 0) {
        free(tmpBuf);
        LOGE("splic later path error");
        return ret;
    }
    *buf = tmpBuf;

    return 0;
}

static int ReadKeyFile(const char *path, char *buf, size_t len)
{
    if (!path || !buf) {
        LOGE("path or buf is null");
        return -EINVAL;
    }
    struct stat st = {0};
    if (stat(path, &st) != 0) {
        LOGE("stat file failed");
        return -EFAULT;
    }
    if ((size_t)st.st_size != len) {
        LOGE("target file size is not equal to buf len");
        return -EINVAL;
    }
    char *realPath = realpath(path, NULL);
    if (realPath == NULL) {
        LOGE("realpath failed");
        return -ENOENT;
    }

    int fd = open(realPath, O_RDONLY);
    free(realPath);
    if (fd < 0) {
        LOGE("key file read open failed");
        return -EFAULT;
    }
    if (read(fd, buf, len) != (ssize_t)len) {
        LOGE("bad file content");
        (void)close(fd);
        return -EBADF;
    }
    (void)close(fd);

    return 0;
}

static int SetPolicyLegacy(const char *keyDescPath,
                           const char *toEncrypt,
                           union FscryptPolicy *arg)
{
    char keyDesc[FSCRYPT_KEY_DESCRIPTOR_SIZE] = {0};
    int ret = ReadKeyFile(keyDescPath, keyDesc, FSCRYPT_KEY_DESCRIPTOR_SIZE);
    if (ret != 0) {
        return ret;
    }
    arg->v1.version = FSCRYPT_POLICY_V1;
    ret = memcpy_s(arg->v1.master_key_descriptor,
        FSCRYPT_KEY_DESCRIPTOR_SIZE, keyDesc, FSCRYPT_KEY_DESCRIPTOR_SIZE);
    if (ret != 0) {
        LOGE("memcpy_s copy failed");
        return ret;
    }
    ret = KeyCtrlSetPolicy(toEncrypt, arg);
    if (ret != 0) {
        LOGE("Set Policy v1 failed");
        return ret;
    }
    return 0;
}

#ifdef SUPPORT_FSCRYPT_V2
static int SetPolicyV2(const char *keyIdPath,
                       const char *toEncrypt,
                       union FscryptPolicy *arg)
{
    char keyId[FSCRYPT_KEY_IDENTIFIER_SIZE] = {0};
    int ret = ReadKeyFile(keyIdPath, keyId, FSCRYPT_KEY_IDENTIFIER_SIZE);
    if (ret != 0) {
        return ret;
    }
    arg->v2.version = FSCRYPT_POLICY_V2;
    ret = memcpy_s(arg->v2.master_key_identifier,
        FSCRYPT_KEY_IDENTIFIER_SIZE, keyId, FSCRYPT_KEY_IDENTIFIER_SIZE);
    if (ret != 0) {
        LOGE("memcpy_s copy failed");
        return ret;
    }
    ret = KeyCtrlSetPolicy(toEncrypt, arg);
    if (ret != 0) {
        LOGE("Set Policy v2 failed");
        return ret;
    }
    return 0;
}
#endif

int LoadAndSetPolicy(const char *keyDir, const char *dir)
{
    if (!keyDir || !dir) {
        LOGE("set policy parameters is null");
        return -EINVAL;
    }
    int ret = InitFscryptPolicy();
    if (ret != 0) {
        LOGE("Get fscrypt policy error %{public}d", ret);
        return ret;
    }

    union FscryptPolicy arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.v1.filenames_encryption_mode = g_fscryptPolicy.fileName;
    arg.v1.contents_encryption_mode = g_fscryptPolicy.content;
    arg.v1.flags = g_fscryptPolicy.flags;

    char *pathBuf = NULL;
    ret = -ENOTSUP;

    uint8_t fscryptVer = KeyCtrlLoadVersion(keyDir);
    if (fscryptVer == FSCRYPT_V1) {
        ret = SpliceKeyPath(keyDir, strlen(keyDir), PATH_KEYDESC,
            strlen(PATH_KEYDESC), &pathBuf);
        if (ret != 0) {
            LOGE("path splice error");
            SAFE_FREE_PTR(pathBuf);
            return ret;
        }
        ret = SetPolicyLegacy(pathBuf, dir, &arg);
        if (ret != 0) {
            LOGE("SetPolicyLegacy fail, ret: %{public}d", ret);
        }
#ifdef SUPPORT_FSCRYPT_V2
    } else if (fscryptVer == FSCRYPT_V2) {
        ret = SpliceKeyPath(keyDir, strlen(keyDir), PATH_KEYID,
            strlen(PATH_KEYID), &pathBuf);
        if (ret != 0) {
            LOGE("path splice error");
            SAFE_FREE_PTR(pathBuf);
            return ret;
        }
        ret = SetPolicyV2(pathBuf, dir, &arg);
        if (ret != 0) {
            LOGE("SetPolicyV2 fail, ret: %{public}d", ret);
        }
#endif
    }
    SAFE_FREE_PTR(pathBuf);
    return ret;
}

static int ActSetFileXattrActSetFileXattr(const char *path, char *keyDesc, int storageType)
{
    struct FscryptSdpPolicy PolicySDP = {0, 0, 0, 0, 0, {0, 0, 0, 0, 0, 0, 0, 0}};
    PolicySDP.version = SDP_VERSIOIN;
    PolicySDP.sdpclass = storageType;
    PolicySDP.contentsEncryptionMode = SDP_CONTENTS_ENCRYPTION_MODE;
    PolicySDP.filenamesEncryptionMode = SDP_FILENAMES_ENCRYPTION_MODE;
    PolicySDP.flags = SDP_FLAGS;

    int ret = memcpy_s((char *)PolicySDP.masterKeyDescriptor, FS_KEY_DESC_SIZE, (char *)keyDesc,
                       FSCRYPT_KEY_DESCRIPTOR_SIZE);
    if (ret != 0) {
        LOGE("memcpy_s copy failed");
        return -errno;
    }
    int fd = open((char *)path, O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        LOGE("install File or Directory open failed: %{public}d", errno);
        return -errno;
    }
    ret = ioctl(fd, F2FS_IOC_SET_SDP_ENCRYPTION_POLICY, &PolicySDP);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}X, errno: %{public}d", ret, errno);
        close(fd);
        return ret;
    }
    close(fd);
    return ret;
}

int LoadAndSetEceAndSecePolicy(const char *keyDir, const char *dir, int type)
{
    int el3Key = 3; // el3
    int el4Key = 4; // el4
    if (!keyDir || !dir) {
        LOGE("set policy parameters is null");
        return -EINVAL;
    }
    char *pathBuf = NULL;
    int ret = SpliceKeyPath(keyDir, strlen(keyDir), PATH_KEYDESC, strlen(PATH_KEYDESC), &pathBuf);
    if (ret != 0) {
        LOGE("path splice error");
        return ret;
    }

    uint8_t fscryptVer = KeyCtrlLoadVersion(keyDir);
    if (fscryptVer == FSCRYPT_V1) {
        if (type == el3Key || type == el4Key) {
            if (type == el3Key) {
                type = FSCRYPT_SDP_SECE_CLASS;
            } else {
                type = FSCRYPT_SDP_ECE_CLASS;
            }
            char keyDesc[FSCRYPT_KEY_DESCRIPTOR_SIZE] = {0};
            ret = ReadKeyFile(pathBuf, keyDesc, FSCRYPT_KEY_DESCRIPTOR_SIZE);
            if (ret != 0) {
                SAFE_FREE_PTR(pathBuf);
                return ret;
            }
            ret = ActSetFileXattrActSetFileXattr(dir, keyDesc, type);
            if (ret != 0) {
                SAFE_FREE_PTR(pathBuf);
                LOGE("ActSetFileXattr failed");
                return ret;
            }
        }
#ifdef SUPPORT_FSCRYPT_V2
    } else if (fscryptVer == FSCRYPT_V2) {
        SAFE_FREE_PTR(pathBuf);
        return 0;
#endif
    }
    SAFE_FREE_PTR(pathBuf);
    return ret;
}

int SetGlobalEl1DirPolicy(const char *dir)
{
    if (!g_fscryptEnabled) {
        LOGI("Fscrypt have not enabled");
        return 0;
    }
#ifdef SUPPORT_RECOVERY_KEY_SERVICE
    if (!IsFsCryptEnableByOemInfo()) {
        LOGI("Fscrypt have not enabled by oeminfo");
        return 0;
    }
#endif
    for (size_t i = 0; i < ARRAY_LEN(GLOBAL_FSCRYPT_DIR); i++) {
        size_t tmpLen = strlen(GLOBAL_FSCRYPT_DIR[i]);
        if ((strncmp(dir, GLOBAL_FSCRYPT_DIR[i], tmpLen) == 0) && (strlen(dir) == tmpLen)) {
            return LoadAndSetPolicy(DEVICE_EL1_DIR, dir);
        }
    }
    return 0;
}

uint8_t GetFscryptVersionFromPolicy(void)
{
    return g_fscryptPolicy.version;
}
