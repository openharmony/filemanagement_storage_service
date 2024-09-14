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

#define FSCRYPT_POLICY_V1 0
#define fscrypt_policy_v1 fscrypt_policy

#define FSCRYPT_POLICY_V2 2

#endif // STORAGE_DAEMON_CRYPTO_FSCRYPT_UAPI_H