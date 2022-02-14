/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_FSCRYPT_UAPI_H
#define STORAGE_DAEMON_CRYPTO_FSCRYPT_UAPI_H

#include <linux/fs.h>

// adapt to old kernel uapi defines
#define FSCRYPT_KEY_DESCRIPTOR_SIZE FS_KEY_DESCRIPTOR_SIZE
#define FSCRYPT_POLICY_FLAGS_PAD_4 FS_POLICY_FLAGS_PAD_4
#define FSCRYPT_POLICY_FLAGS_PAD_8 FS_POLICY_FLAGS_PAD_8
#define FSCRYPT_POLICY_FLAGS_PAD_16 FS_POLICY_FLAGS_PAD_16
#define FSCRYPT_POLICY_FLAGS_PAD_32 FS_POLICY_FLAGS_PAD_32
#define FSCRYPT_POLICY_FLAGS_PAD_MASK FS_POLICY_FLAGS_PAD_MASK
#define FSCRYPT_POLICY_FLAG_DIRECT_KEY FS_POLICY_FLAG_DIRECT_KEY

#define FSCRYPT_MODE_AES_256_XTS FS_ENCRYPTION_MODE_AES_256_XTS
#define FSCRYPT_MODE_AES_256_CTS FS_ENCRYPTION_MODE_AES_256_CTS
#define FSCRYPT_MODE_AES_128_CBC FS_ENCRYPTION_MODE_AES_128_CBC
#define FSCRYPT_MODE_AES_128_CTS FS_ENCRYPTION_MODE_AES_128_CTS
#define FSCRYPT_MODE_ADIANTUM FS_ENCRYPTION_MODE_ADIANTUM

#define FSCRYPT_KEY_DESC_PREFIX FS_KEY_DESC_PREFIX
#define FSCRYPT_KEY_DESC_PREFIX_SIZE FS_KEY_DESC_PREFIX_SIZE
#define FSCRYPT_MAX_KEY_SIZE FS_MAX_KEY_SIZE

#define FS_IOC_GET_ENCRYPTION_POLICY_EX _IOWR('f', 22, __u8[9])
#define FS_IOC_ADD_ENCRYPTION_KEY _IOWR('f', 23, struct fscrypt_add_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY _IOWR('f', 24, struct fscrypt_remove_key_arg)
#define FS_IOC_REMOVE_ENCRYPTION_KEY_ALL_USERS _IOWR('f', 25, struct fscrypt_remove_key_arg)
#define FS_IOC_GET_ENCRYPTION_KEY_STATUS _IOWR('f', 26, struct fscrypt_get_key_status_arg)

#define FSCRYPT_POLICY_V1 0
#define fscrypt_policy_v1 fscrypt_policy

#define FSCRYPT_POLICY_V2 2
#define FSCRYPT_KEY_IDENTIFIER_SIZE 16
struct fscrypt_policy_v2 {
    __u8 version;
    __u8 contents_encryption_mode;
    __u8 filenames_encryption_mode;
    __u8 flags;
    __u8 __reserved[4];
    __u8 master_key_identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
};

struct fscrypt_get_policy_ex_arg {
    __u64 policy_size;
    union {
        __u8 version;
        struct fscrypt_policy_v1 v1;
        struct fscrypt_policy_v2 v2;
    } policy;
};

struct fscrypt_key_specifier {
#define FSCRYPT_KEY_SPEC_TYPE_DESCRIPTOR 1
#define FSCRYPT_KEY_SPEC_TYPE_IDENTIFIER 2
    __u32 type;
    __u32 __reserved;
    union {
        __u8 __reserved[32];
        __u8 descriptor[FSCRYPT_KEY_DESCRIPTOR_SIZE];
        __u8 identifier[FSCRYPT_KEY_IDENTIFIER_SIZE];
    } u;
};

struct fscrypt_add_key_arg {
    struct fscrypt_key_specifier key_spec;
    __u32 raw_size;
    __u32 __reserved[9];
    __u8 raw[];
};

struct fscrypt_remove_key_arg {
    struct fscrypt_key_specifier key_spec;
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_FILES_BUSY 0x00000001
#define FSCRYPT_KEY_REMOVAL_STATUS_FLAG_OTHER_USERS 0x00000002
    __u32 removal_status_flags;
    __u32 __reserved[5];
};

struct fscrypt_get_key_status_arg {
    struct fscrypt_key_specifier key_spec;
    __u32 __reserved[6];
#define FSCRYPT_KEY_STATUS_ABSENT 1
#define FSCRYPT_KEY_STATUS_PRESENT 2
#define FSCRYPT_KEY_STATUS_INCOMPLETELY_REMOVED 3
    __u32 status;
#define FSCRYPT_KEY_STATUS_FLAG_ADDED_BY_SELF 0x00000001
    __u32 status_flags;
    __u32 user_count;
    __u32 __out_reserved[13];
};

#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_UAPI_H