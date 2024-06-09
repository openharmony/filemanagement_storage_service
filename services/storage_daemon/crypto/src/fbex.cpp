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

#include "fbex.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <securec.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

#include "file_ex.h"
#include "storage_service_log.h"
#include "openssl_crypto.h"

namespace {
constexpr const char *FBEX_UFS_INLINE_SUPPORT_PREFIX = "/sys/devices/platform/";
constexpr const char *FBEX_UFS_INLINE_SUPPORT_END = "/ufs_inline_stat";
constexpr const char *FBEX_NVME_INLINE_SUPPORT_PATH = "/proc/nvme_crypto";
constexpr const char *FBEX_UFS_INLINE_BASE_ADDR = "/proc/bootdevice/name";
constexpr const char *FBEX_INLINE_CRYPTO_V3 = "3\n";

constexpr const char *FBEX_CMD_PATH = "/dev/fbex_cmd";
constexpr const char *FBEX_UECE_PATH = "/dev/fbex_uece";

const uint8_t FBEX_IOC_MAGIC = 'f';
const uint8_t FBEX_ADD_IV = 0x1;
const uint8_t FBEX_DEL_IV = 0x2;
const uint8_t FBEX_LOCK_SCREEN = 0x3;
const uint8_t FBEX_UNLOCK_SCREEN = 0x4;
const uint8_t FBEX_USER_LOGOUT = 0x8;
const uint8_t FBEX_STATUS_REPORT = 0xC;
const uint8_t FBEX_ADD_EL5 = 21;
const uint8_t FBEX_READ_EL5 = 22;
const uint8_t FBEX_WRITE_EL5 = 23;
const uint8_t FBEX_DEL_EL5 = 24;
const uint8_t FBEX_GENERATE_APP_KEY = 25;
const uint8_t FBEX_CHANGE_PINCODE = 26;

struct FbeOptStr {
    uint32_t user = 0;
    uint32_t type = 0;
    uint32_t len = 0;
    uint8_t iv[OHOS::StorageDaemon::FBEX_IV_SIZE] = {0};
    uint8_t flag = 0;
};
using FbeOpts = FbeOptStr;

struct FbeOptStrE {
    uint32_t user = 0;
    uint32_t status = 0;
    uint32_t length = 0;
    uint8_t eBuffer[OHOS::StorageDaemon::FBEX_E_BUFFER_SIZE] = {0};
};
using FbeOptsE = FbeOptStrE;

#define FBEX_IOC_ADD_IV _IOWR(FBEX_IOC_MAGIC, FBEX_ADD_IV, FbeOpts)
#define FBEX_IOC_DEL_IV _IOW(FBEX_IOC_MAGIC, FBEX_DEL_IV, FbeOpts)
#define FBEX_IOC_LOCK_SCREEN _IOW(FBEX_IOC_MAGIC, FBEX_LOCK_SCREEN, FbeOpts)
#define FBEX_IOC_UNLOCK_SCREEN _IOWR(FBEX_IOC_MAGIC, FBEX_UNLOCK_SCREEN, FbeOpts)
#define FBEX_IOC_USER_LOGOUT _IOW(FBEX_IOC_MAGIC, FBEX_USER_LOGOUT, FbeOpts)
#define FBEX_IOC_STATUS_REPORT _IOW(FBEX_IOC_MAGIC, FBEX_STATUS_REPORT, FbeOpts)
#define HISI_FBEX_READ_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_READ_EL5, FbeOptsE)
#define HISI_FBEX_WRITE_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_WRITE_EL5, FbeOptsE)
#define HISI_FBEX_ADD_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_ADD_EL5, FbeOptsE)
#define HISI_FBEX_DEL_USER_PINCODE _IOWR(FBEX_IOC_MAGIC, FBEX_DEL_EL5, FbeOptsE)
#define HISI_FBEX_ADD_APPKEY2 _IOWR(FBEX_IOC_MAGIC, FBEX_GENERATE_APP_KEY, FbeOptsE)
#define HISI_FBEX_CHANGE_PINCODE _IOWR(FBEX_IOC_MAGIC, FBEX_CHANGE_PINCODE, FbeOptsE)

} // namespace

namespace OHOS {
namespace StorageDaemon {
bool FBEX::IsFBEXSupported()
{
    std::string baseAddr;
    if (!OHOS::LoadStringFromFile(FBEX_UFS_INLINE_BASE_ADDR, baseAddr)) {
        LOGE("Read baseAddr failed, errno: %{public}d", errno);
        return false;
    }

    std::string path = FBEX_UFS_INLINE_SUPPORT_PREFIX + baseAddr + FBEX_UFS_INLINE_SUPPORT_END;
    std::string rpath(PATH_MAX + 1, '\0');
    std::string nvmePath = FBEX_NVME_INLINE_SUPPORT_PATH;

    if ((path.length() > PATH_MAX) || (realpath(path.c_str(), rpath.data()) == nullptr)) {
        LOGE("realpath of %{public}s failed, errno: %{public}d", path.c_str(), errno);
        return access(nvmePath.c_str(), F_OK) == 0;
    }
    if (rpath.rfind(FBEX_UFS_INLINE_SUPPORT_PREFIX) != 0) {
        LOGE("rpath %{public}s is invalid", rpath.c_str());
        return false;
    }

    std::string versionNum;
    if (!OHOS::LoadStringFromFile(rpath, versionNum)) {
        LOGE("read ufs_inline_stat failed, errno: %{public}d", errno);
        return false;
    }
    return versionNum.compare(FBEX_INLINE_CRYPTO_V3) == 0;
}

static inline bool CheckIvValid(const uint8_t *iv, uint32_t size)
{
    return (iv != nullptr) && (size == FBEX_IV_SIZE);
}

static inline bool CheckReadBuffValid(const uint8_t *eBuffer, uint32_t size, uint32_t status)
{
    if (status == UNLOCK_STATUS) {
        return (eBuffer != nullptr) && (size == (GCM_NONCE_BYTES + AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES));
    }
    return (eBuffer != nullptr) && (size == AES_256_HASH_RANDOM_SIZE);
}

static inline bool CheckWriteBuffValid(const uint8_t *eBuffer, uint32_t size, uint32_t status)
{
    if (status == UNLOCK_STATUS) {
        return (eBuffer != nullptr) && (size == AES_256_HASH_RANDOM_SIZE);
    }
    return (eBuffer != nullptr) && (size == (GCM_NONCE_BYTES + AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES));
}

int FBEX::InstallEL5KeyToKernel(uint32_t userId, uint8_t flag)
{
    LOGI("InstallEL5KeyToKernel enter, userId: %{public}d, flag: %{public}u", userId, flag);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOptsE ops{.user = userId};
    uint32_t fbeRet = ioctl(fd, HISI_FBEX_ADD_CLASS_E, &ops);
    int ret = 0;
    LOGE("InstallEL5KeyToKernel, ret: 0x%{public}x, errno: %{public}d", fbeRet, errno);
    if (fbeRet != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", fbeRet, errno);
        ret = -errno;
    }
    close(fd);
    LOGI("InstallEL5KeyToKernel success");
    return ret;
}

int FBEX::InstallKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, uint8_t flag)
{
    LOGD("enter, userId: %{public}d, type: %{public}u, flag: %{public}u", userId, type, flag);
    if (!CheckIvValid(iv, size)) {
        LOGE("install key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOpts ops{.user = userId, .type = type, .len = size, .flag = flag};
    (void)memcpy_s(ops.iv, sizeof(ops.iv), iv, size);
    int ret = ioctl(fd, FBEX_IOC_ADD_IV, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        close(fd);
        return ret;
    }
    close(fd);

    (void)memcpy_s(iv, size, ops.iv, sizeof(ops.iv));
    LOGD("success");
    return ret;
}

int FBEX::UninstallOrLockUserKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, bool destroy)
{
    LOGD("enter, userId: %{public}d, type: %{public}u, flag: %{public}d", userId, type, destroy);
    if (!CheckIvValid(iv, size)) {
        LOGE("uninstall key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOpts ops{.user = userId, .type = type, .len = size};
    (void)memcpy_s(ops.iv, sizeof(ops.iv), iv, size);
    int ret = ioctl(fd, destroy ? FBEX_IOC_DEL_IV : FBEX_IOC_USER_LOGOUT, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
    }
    close(fd);
    LOGD("success");
    return ret;
}

int FBEX::UninstallOrLockUserKeyForEL5ToKernel(uint32_t userId, bool destroy)
{
    LOGI("enter, userId: %{public}d, flag: %{public}d", userId, destroy);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{.user = userId};
    uint32_t fbeRet = ioctl(fd, destroy ? HISI_FBEX_DEL_USER_PINCODE : FBEX_IOC_USER_LOGOUT, &ops);
    int ret = 0;
    if (fbeRet != 0) {
        LOGE("ioctl fbex_cmd failed, fbeRet: 0x%{public}x, errno: %{public}d", fbeRet, errno);
        ret = -errno;
    }
    close(fd);
    LOGI("success");
    return ret;
}

int FBEX::ChangePinCodeClassE(uint32_t userId)
{
    LOGI("enter, userId: %{public}d", userId);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{.user = userId};
    int ret = ioctl(fd, FBEX_CHANGE_PINCODE, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        ret = -errno;
    }
    close(fd);
    LOGI("change pincode classE finish.");
    return ret;
}

// for el3 & el4
int FBEX::LockScreenToKernel(uint32_t userId)
{
    LOGD("enter, userId: %{public}d", userId);

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOpts ops;
    (void)memset_s(&ops, sizeof(FbeOpts), 0, sizeof(FbeOpts));
    ops.user = userId;
    int ret = ioctl(fd, FBEX_IOC_LOCK_SCREEN, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
    }
    close(fd);
    LOGD("success");
    return ret;
}

int FBEX::GenerateAppkey(uint32_t userId, uint32_t appUid, std::unique_ptr<uint8_t[]> &appKey, uint32_t size)
{
    LOGI("GenerateAppkey enter");
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            appKey.reset(nullptr);
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{.user = userId, .status = appUid, .length = size};
    uint32_t fbeRet = ioctl(fd, HISI_FBEX_ADD_APPKEY2, &ops);
    if (fbeRet != 0) {
        LOGE("ioctl fbex_cmd failed, fbeRet: 0x%{public}x, errno: %{public}d", fbeRet, errno);
        close(fd);
        return -errno;
    }
    (void)memcpy_s(appKey.get(), size, ops.eBuffer, sizeof(ops.eBuffer));
    close(fd);
    LOGI("success");
    return 0;
}

int FBEX::UnlockScreenToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size)
{
    LOGD("enter, userId: %{public}d, type: %{public}u", userId, type);
    if (!CheckIvValid(iv, size)) {
        LOGE("install key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOpts ops{.user = userId, .type = type, .len = size};
    (void)memcpy_s(ops.iv, sizeof(ops.iv), iv, size);
    int ret = ioctl(fd, FBEX_IOC_UNLOCK_SCREEN, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        close(fd);
        return ret;
    }
    close(fd);

    (void)memcpy_s(iv, size, ops.iv, sizeof(ops.iv));
    LOGD("success");
    return ret;
}

int FBEX::ReadESecretToKernel(uint32_t userId, uint32_t status, uint8_t *eBuffer, uint32_t length, bool &isFbeSupport)
{
    LOGI("enter, userId: %{public}d, status: %{public}u", userId, status);
    if (!CheckReadBuffValid(eBuffer, length, status)) {
        LOGE("read e secret param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            isFbeSupport = false;
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    uint32_t bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    FbeOptsE ops{.user = userId, .status = status, .length = bufferSize};
    (void)memcpy_s(ops.eBuffer, sizeof(ops.eBuffer), eBuffer, length);
    uint32_t ret = ioctl(fd, HISI_FBEX_READ_CLASS_E, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        close(fd);
        return -errno;
    }
    close(fd);
    if (status == UNLOCK_STATUS) {
        bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    } else {
        bufferSize = AES_256_HASH_RANDOM_SIZE;
    }
    (void)memcpy_s(eBuffer, length, ops.eBuffer, bufferSize);
    LOGI("success");
    return 0;
}

int FBEX::WriteESecretToKernel(uint32_t userId, uint32_t status, uint8_t *eBuffer, uint32_t length)
{
    LOGI("enter, userId: %{public}d, status: %{public}u", userId, status);
    if (!CheckWriteBuffValid(eBuffer, length, status)) {
        LOGE("write e secret param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("fbex_uece does not exist, fbe not support this command!");
            return 0;
        }
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    uint32_t bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    FbeOptsE ops{.user = userId, .status = status, .length = bufferSize};
    (void)memcpy_s(ops.eBuffer, sizeof(ops.eBuffer), eBuffer, length);
    uint32_t ret = ioctl(fd, HISI_FBEX_WRITE_CLASS_E, &ops);
    if (ret != 0) {
        LOGE("ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        close(fd);
        return -errno;
    }
    close(fd);
    LOGI("success");
    return 0;
}

bool FBEX::IsMspReady()
{
    std::string status;
    (void)OHOS::LoadStringFromFile(FBEX_CMD_PATH, status);
    return status == "true";
}

int FBEX::GetStatus()
{
    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOpts ops;
    int ret = ioctl(fd, FBEX_IOC_STATUS_REPORT, &ops);
    close(fd);
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS
