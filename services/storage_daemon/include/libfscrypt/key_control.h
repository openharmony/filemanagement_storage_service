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
#ifndef KEY_CONTROL_H
#define KEY_CONTROL_H

#include <linux/keyctl.h>
#include <linux/version.h>
#if ((defined LINUX_VERSION_CODE ) && LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/fscrypt.h>
#define SUPPORT_FSCRYPT_V2
#else
#include "fscrypt_uapi.h"
#endif

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POLICY_BUF_SIZE (100)

enum {
    FSCRYPT_INVALID = 0,
    FSCRYPT_V1 = 1,
    FSCRYPT_V2 = 2,
};

union FscryptPolicy {
    struct fscrypt_policy_v1 v1;
#ifdef SUPPORT_FSCRYPT_V2
    struct fscrypt_policy_v2 v2;
#endif
};

typedef unsigned char uint8_t;
typedef int key_serial_t;

static const char *FSCRYPT_POLICY_KEY = "fscrypt.policy.config";
static const char *PATH_FSCRYPT_VER = "/fscrypt_version";

key_serial_t KeyCtrlGetKeyringId(key_serial_t id, int create);
key_serial_t KeyCtrlAddKey(const char *type, const char *description,
    const key_serial_t ringId);
key_serial_t KeyCtrlAddKeyEx(const char *type, const char *description,
    struct fscrypt_key *fsKey, const key_serial_t ringId);
long KeyCtrlSearch(key_serial_t ringId, const char *type, const char *description,
    key_serial_t destRingId);
long KeyCtrlUnlink(key_serial_t key, key_serial_t keyring);

#ifdef SUPPORT_FSCRYPT_V2
bool KeyCtrlInstallKey(const char *mnt, struct fscrypt_add_key_arg *arg);
bool KeyCtrlRemoveKey(const char *mnt, struct fscrypt_remove_key_arg *arg);
bool KeyCtrlGetKeyStatus(const char *mnt, struct fscrypt_get_key_status_arg *arg);
bool KeyCtrlGetPolicyEx(const char *path, struct fscrypt_get_policy_ex_arg *policy);
#endif

bool KeyCtrlSetPolicy(const char *path, union FscryptPolicy *policy);
bool KeyCtrlGetPolicy(const char *path, struct fscrypt_policy *policy);

uint8_t KeyCtrlGetFscryptVersion(const char *mnt);
uint8_t KeyCtrlLoadVersion(const char *keyPath);

bool KeyCtrlHasFscryptSyspara(void);

#ifdef __cplusplus
}
#endif

#endif