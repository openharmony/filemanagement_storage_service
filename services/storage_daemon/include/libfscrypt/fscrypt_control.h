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
#ifndef FSCRYPT_CONTROL_H
#define FSCRYPT_CONTROL_H

#include "key_control.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define SDP_VERSIOIN 0
#define SDP_FLAGS 0
#define SDP_CONTENTS_ENCRYPTION_MODE 1
#define SDP_FILENAMES_ENCRYPTION_MODE 4
#define FS_KEY_DESC_SIZE 8
#define F2FS_IOCTL_MAGIC 0xf5
#define F2FS_IOC_SET_SDP_ENCRYPTION_POLICY _IOW(F2FS_IOCTL_MAGIC, 80, struct FscryptSdpPolicy)

#pragma pack(push, 1)
struct FscryptSdpPolicy {
    uint8_t version;
    uint8_t sdpclass;
    uint8_t contentsEncryptionMode;
    uint8_t filenamesEncryptionMode;
    uint8_t flags;
    uint8_t masterKeyDescriptor[FS_KEY_DESC_SIZE];
};
#pragma pack(pop)

int FscryptSetSysparam(const char *policy);
int SetGlobalEl1DirPolicy(const char *dir);
int LoadAndSetPolicy(const char *keyDir, const char *dir);
int LoadAndSetEceAndSecePolicy(const char *keyDir, const char *dir, int type);
int InitFscryptPolicy(void);
uint8_t GetFscryptVersionFromPolicy(void);

#ifdef __cplusplus
}
#endif

#endif