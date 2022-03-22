/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_KEYCTL_H
#define STORAGE_DAEMON_CRYPTO_KEYCTL_H

#include <vector>
#include <map>
#include <string>
#include <set>
#include <linux/keyctl.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/fscrypt.h>
#define SUPPORT_FSCRYPT_V2
#else
#include "fscrypt_uapi.h"
#endif

namespace OHOS {
namespace StorageDaemon {
using key_serial_t = int;
constexpr uint32_t CRYPTO_KEY_DESC_SIZE = FSCRYPT_KEY_DESCRIPTOR_SIZE;
static const std::string MNT_DATA = "/data";
static const std::string PATH_LATEST = "/latest";
static const std::string PATH_FSCRYPT_VER = "/fscrypt_version";
static const std::string PATH_SHIELD = "/shield";
static const std::string PATH_SECDISC = "/sec_discard";
static const std::string PATH_ENCRYPTED = "/encrypted";
static const std::string PATH_KEYID = "/key_id";
static const std::string PATH_KEYDESC = "/key_desc";

enum {
    FSCRYPT_INVALID = 0,
    FSCRYPT_V1 = 1,
    FSCRYPT_V2 = 2,
};

union FscryptPolicy {
    fscrypt_policy_v1 v1;
#ifdef SUPPORT_FSCRYPT_V2
    fscrypt_policy_v2 v2;
#endif
};

const std::string DATA_EL0_DIR = std::string() + "/data/service/el0";
const std::string STORAGE_DAEMON_DIR = DATA_EL0_DIR + "/storage_daemon";
const std::string DEVICE_EL1_DIR = STORAGE_DAEMON_DIR + "/sd";

struct EncryptPolicy {
    std::string version;
    std::string fileName;
    std::string content;
    std::string flags;
    bool hwWrappedKey { false };
};

static const EncryptPolicy DEFAULT_POLICY = {
    .version = "2",
    .fileName = "aes-256-cts",
    .content = "aes-256-xts",
    .flags = "padding-32",
    .hwWrappedKey = false,
};

static const auto ALL_VERSION = std::map<std::string, uint8_t> {
    {"1", FSCRYPT_V1},
    {"2", FSCRYPT_V2},
};

static const auto FILENAME_MODES = std::map<std::string, uint8_t> {
    {"aes-256-cts", FSCRYPT_MODE_AES_256_CTS},
};

static const auto CONTENTS_MODES = std::map<std::string, uint8_t> {
    {"aes-256-xts", FSCRYPT_MODE_AES_256_XTS},
};
// To use Adiantum, CONFIG_CRYPTO_ADIANTUM must be enabled.
// Also, fast implementations of ChaCha and NHPoly1305 should be enabled,
// e.g. CONFIG_CRYPTO_CHACHA20_NEON and CONFIG_CRYPTO_NHPOLY1305_NEON for ARM.

static const auto POLICY_FLAGS = std::map<std::string, uint8_t> {
    {"padding-4", FSCRYPT_POLICY_FLAGS_PAD_4},
    {"padding-8", FSCRYPT_POLICY_FLAGS_PAD_8},
    {"padding-16", FSCRYPT_POLICY_FLAGS_PAD_16},
    {"padding-32", FSCRYPT_POLICY_FLAGS_PAD_32},
    // "direct-key" use with adiantum
};

static const auto FSCRYPT_OPTIONS_TABLE = std::vector<std::map<std::string, uint8_t>> {
    ALL_VERSION,
    FILENAME_MODES,
    CONTENTS_MODES,
};

static const std::set<std::string> GLOBAL_FSCRYPT_DIR = {
    "/data/app/el1/bundle/public",
    "/data/service/el1/public",
    "/data/chipset/el1/public",
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_KEYCTL_H
