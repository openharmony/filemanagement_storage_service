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
#include "fscrypt_ctrl.h"

#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/fscrypt.h>
#else
#include "fscrypt_uapi.h"
#endif
#include <stdbool.h>
#include <string.h>

#include "parameter.h"
#include "fscrypt_log.h"
#include "init_utils.h"
#include "securec.h"

#define ARRAY_LEN(array) (sizeof((array)) / sizeof((array)[0]))
#define POLICY_BUF_SIZE (100)

typedef struct FscrtpyItem_ {
    char *key;
    unsigned int value;
}FscrtpyItem;

typedef struct EncryptPolicy_ {
    int version;
    int fileName;
    int content;
    int flags;
    bool hwWrappedKey;
}EncryptPolicy;

union FscryptPolicy {
    fscrypt_policy_v1 v1;
    fscrypt_policy_v2 v2;
};

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

static const char *FSCRYPT_POLICY_KEY = "fscrypt.policy.config";

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

//static const char *DATA_EL0_DIR = "/data/service/el0";
//static const char *STORAGE_DAEMON_DIR = "/data/service/el0/storage_daemon";
static const char *DEVICE_EL1_DIR = "/data/service/el0/storage_daemon/sd";
static const char *PATH_KEYID = "/key_id";
static const char *PATH_KEYDESC = "/key_desc";

static bool IsSupportedPolicyKey(const char *key, const FscrtpyItem *items, size_t len)
{
    for (size_t i = 0 ; i < len; i++) {
        if (strncmp(key, items[i].key, strlen(items[i].key)) == 0) {
            return true;
        }
    }
    return false;
}

static bool IsSupportedPolicy(const char *policy, enum FscryptOptins number)
{
    if ((number >= FSCRYPT_OPTIONS_MAX) ||(!policy)) {
        return false;
    }

    switch (number) {
        case FSCRYPT_VERSION_NUM:
            return IsSupportedPolicyKey(policy, ALL_VERSION, ARRAY_LEN(ALL_VERSION));
        case FSCRYPT_FILENAME_MODE:
            return IsSupportedPolicyKey(policy, FILENAME_MODES, ARRAY_LEN(FILENAME_MODES));
        case FSCRYPT_CONTENT_MODE:
            return IsSupportedPolicyKey(policy, CONTENTS_MODES, ARRAY_LEN(CONTENTS_MODES));
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
        FSCRYPT_LOGE("No memory for policy option");
        return -ENOMEM;
    }
    int optNums = 0;
    char **options = SplitStringExt(tmp, ":", &optNums, FSCRYPT_OPTIONS_MAX);
    free(tmp);
    if (!options) {  
        return -ENOMEM;
    }
    if (optNums != FSCRYPT_OPTIONS_MAX) {
        FSCRYPT_LOGE("Mount fscrypt option setting error, not enabled crypto!");
        FreeStringVector(options, optNums);
        return -EFAULT;
    }

    // check supported policy
    for (enum FscryptOptins i = FSCRYPT_VERSION_NUM; i < FSCRYPT_OPTIONS_MAX; i++) {
        char *temp = options[i];
        if (!IsSupportedPolicy(temp, i)) {
            FSCRYPT_LOGE("Not supported policy, %s", temp);
            FreeStringVector(options, optNums);
            return -ENOTSUP;            
        }
    }
    FreeStringVector(options, optNums);

    int ret = SetParameter(FSCRYPT_POLICY_KEY, policy);
    if (ret < 0) {
        FSCRYPT_LOGE("Set fscrypt system parameter failed %d", ret);
        return ret;
    }
    g_fscryptEnabled = true;

    return 0;
}

static int InitFscryptPolicy()
{
    char policy[POLICY_BUF_SIZE];
    int ret = GetParameter(FSCRYPT_POLICY_KEY, "", policy, POLICY_BUF_SIZE - 1);
    if (ret <= 0) {
        FSCRYPT_LOGI("Get fscrypt policy failed");
        return -ENOTSUP;
    }
    int count = 0;
    char **option = SplitStringExt(policy, ":", &count, FSCRYPT_OPTIONS_MAX);
    if (!option) {
        FSCRYPT_LOGE("Fscrypt setting error");
        return -ENOTSUP;
    }
    if (count != FSCRYPT_OPTIONS_MAX) {
        FSCRYPT_LOGE("Fscrypt policy count error");
        FreeStringVector(option, count);
        return -ENOTSUP; 
    }

    for (size_t i = 0; i < ARRAY_LEN(ALL_VERSION); i++) {
        if (strncmp(option[FSCRYPT_VERSION_NUM], ALL_VERSION[i].key,
            strlen(ALL_VERSION[i].key)) == 0) {
            g_fscryptPolicy.version = ALL_VERSION[i].value;
            break;
        }
    }
    for (size_t i = 0; i < ARRAY_LEN(FILENAME_MODES); i++) {
        if (strncmp(option[FSCRYPT_FILENAME_MODE], FILENAME_MODES[i].key,
            strlen(FILENAME_MODES[i].key)) == 0) {
            g_fscryptPolicy.fileName = FILENAME_MODES[i].value;
            break;
        }
    }
    for (size_t i = 0; i < ARRAY_LEN(CONTENTS_MODES); i++) {
        if (strncmp(option[FSCRYPT_CONTENT_MODE], CONTENTS_MODES[i].key,
            strlen(CONTENTS_MODES[i].key)) == 0) {
            g_fscryptPolicy.content = CONTENTS_MODES[i].value;
            break;
        }
    }
    FreeStringVector(option, count);
    FSCRYPT_LOGI("Fscrypt policy init success");

    return 0;
}

static int SplicKeyPath(char *buf, size_t bufMax, const char *src, size_t len)
{
    pathBuf[0] = '\0';
    ret = strncat_s(buf, bufMax, DEVICE_EL1_DIR, strlen(DEVICE_EL1_DIR));
    if (ret != 0) {
        FSCRYPT_LOGE("splic previous path error");
        return -EFAULT;
    }
    ret = strncat_s(buf, bufMax, src, len);
    if (ret != 0) {
        FSCRYPT_LOGE("splic later path error");
        return -EFAULT;
    }
    return 0;
}

static int LoadAndSetPolicy(const char *keyDir, const char *dir)
{
    int ret;
    if (!g_fscryptInited) {
        ret = InitFscryptPolicy();
        if (ret != 0) {
            FSCRYPT_LOGE("Get fscrypt policy error %d", ret);
            return ret;
        }
        g_fscryptInited = true;
    }

    FscryptPolicy arg;
    (void)memset_s(&arg, sizeof(arg), 0, sizeof(arg));
    arg.v1.filenames_encryption_mode = g_fscryptPolicy.fileName;
    arg.v1.contents_encryption_mode = g_fscryptPolicy.content;
    arg.v1.flags = g_fscryptPolicy.flags;

    char *pathBuf = NULL;
    int ret = -ENOTSUP;
    if (g_fscryptPolicy.version == FSCRYPT_V1) {
        size_t len = strlen(DEVICE_EL1_DIR) + strlen(PATH_KEYDESC) + 1;
        pathBuf = (char *)malloc(len);
        if (!pathBuf) {
            FSCRYPT_LOGE("No memory for fscrypt v1 path buffer");
            return -ENOMEM;
        }
        ret = SplicKeyPath(pathBuf, len, PATH_KEYDESC, strlen(PATH_KEYDESC));
        if (ret != 0) {
            return ret;
        }
        ret =  SetPolicyLegacy(pathBuf, dir, arg);
    } else if (g_fscryptPolicy.version == FSCRYPT_V2)
        size_t len = strlen(DEVICE_EL1_DIR) + strlen(PATH_KEYID) + 1;
        pathBuf = (char *)malloc(len);
        if (!pathBuf) {
            FSCRYPT_LOGE("No memory for fscrypt v1 path buffer");
            return -ENOMEM;
        }
        ret = SplicKeyPath(pathBuf, len, PATH_KEYID, strlen(PATH_KEYID));
        if (ret != 0) {
            return ret;
        }
        ret = SetPolicyV2(pathBuf, dir, arg);
    }
    if (pathBuf != NULL) {
        free(pathBuf);
    }

    return ret;
}

int SetGlobalEl1DirPolicy(const char *dir)
{
    if (!g_fscryptEnabled) {
        FSCRYPT_LOGI("Fscrypt have not enabled");
        return 0;
    }
    size_t len = strlen(dir);
    for (size_t i = 0; i < ARRAY_LEN(GLOBAL_FSCRYPT_DIR); i++) {
        size_t tmpLen = strlen(GLOBAL_FSCRYPT_DIR[i]);
        if (tmpLen != len) {
            return 0;
        }
        if (strncmp(dir, GLOBAL_FSCRYPT_DIR[i], len) != 0) {
            return 0;
        }
    }

    return LoadAndSetPolicy(DEVICE_EL1_DIR, dir);
}