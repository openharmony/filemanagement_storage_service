/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "fbex.h"
#include "file_ex.h"
#include "openssl_crypto.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

using namespace OHOS::StorageService;
namespace {
constexpr const char *FBEX_UFS_INLINE_SUPPORT_PREFIX = "/sys/devices/platform/";
constexpr const char *FBEX_UFS_INLINE_SUPPORT_END = "/ufs_inline_stat";
constexpr const char *FBEX_NVME_INLINE_SUPPORT_PATH = "/sys/block/nvme_crypto";
constexpr const char *FBEX_EMMC_INLINE_SUPPORT_PATH = "/sys/block/emmc_crypto";
constexpr const char *FBEX_UFS_INLINE_BASE_ADDR = "/proc/bootdevice/name";
constexpr const char *FBEX_INLINE_CRYPTO_V3 = "3\n";

constexpr const char *FBEX_CMD_PATH = "/dev/fbex_cmd";
constexpr const char *FBEX_UECE_PATH = "/dev/fbex_uece";

constexpr uint32_t FBEX_E_BUFFER_SIZE = 64;
constexpr uint32_t AUTH_TOKEN_MAX_SIZE = 1024;
constexpr uint32_t UNLOCK_STATUS = 0x2;

constexpr uint8_t FBEX_IOC_MAGIC = 'f';
constexpr uint8_t FBEX_ADD_IV = 0x1;
constexpr uint8_t FBEX_DEL_IV = 0x2;
constexpr uint8_t FBEX_LOCK_SCREEN = 0x3;
constexpr uint8_t FBEX_UNLOCK_SCREEN = 0x4;
constexpr uint8_t FBEX_USER_LOGOUT = 0x8;
constexpr uint8_t FBEX_STATUS_REPORT = 0xC;
constexpr uint8_t FBEX_ADD_DOUBLE_DE_IV = 20;
constexpr uint8_t FBEX_ADD_EL5 = 21;
constexpr uint8_t FBEX_READ_EL5 = 22;
constexpr uint8_t FBEX_WRITE_EL5 = 23;
constexpr uint8_t FBEX_DEL_EL5_PINCODE = 24;
constexpr uint8_t FBEX_GENERATE_APP_KEY = 25;
constexpr uint8_t FBEX_CHANGE_PINCODE = 26;
constexpr uint8_t FBEX_LOCK_EL5 = 27;
constexpr uint8_t FBEX_UECE_BACKUP = 30;

constexpr uint32_t FILE_ENCRY_ERROR_UECE_ALREADY_CREATED = 0xFBE30031;
constexpr uint32_t FILE_ENCRY_ERROR_NOT_FOUND_UECE = 0xFBE30033;
constexpr uint32_t FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG = 0xFBE30034;

struct FbeOptStr {
    uint32_t user = 0;
    uint32_t type = 0;
    uint32_t len = 0;
    uint8_t iv[OHOS::StorageDaemon::FBEX_IV_SIZE] = {0};
    uint8_t flag = 0;
};
using FbeOpts = FbeOptStr;

struct FbeOptStrV1 {
    uint32_t user = 0;
    uint32_t type = 0;
    uint32_t len = 0;
    uint8_t iv[OHOS::StorageDaemon::FBEX_IV_SIZE] = {0};
    uint8_t flag = 0;
    uint32_t authTokenSize = 0;
    uint8_t authToken[AUTH_TOKEN_MAX_SIZE] = {0};
};
using FbeOptsV1 = FbeOptStrV1;

struct FbeOptStrE {
    uint32_t userIdDouble = 0;
    uint32_t userIdSingle = 0;
    uint32_t status = 0;
    uint32_t length = 0;
    uint8_t eBuffer[FBEX_E_BUFFER_SIZE] = {0};
};
using FbeOptsE = FbeOptStrE;

struct FbeOptStrEV1 {
    uint32_t userIdDouble = 0;
    uint32_t userIdSingle = 0;
    uint32_t status = 0;
    uint32_t length = 0;
    uint8_t eBuffer[FBEX_E_BUFFER_SIZE] = {0};
    uint32_t authTokenSize = 0;
    uint8_t authToken[AUTH_TOKEN_MAX_SIZE] = {0};
};
using FbeOptsEV1 = FbeOptStrEV1;

#define FBEX_IOC_ADD_IV _IOWR(FBEX_IOC_MAGIC, FBEX_ADD_IV, FbeOpts)
#define FBEX_IOC_ADD_DOUBLE_DE_IV _IOWR(FBEX_IOC_MAGIC, FBEX_ADD_DOUBLE_DE_IV, FbeOpts)
#define FBEX_IOC_DEL_IV _IOW(FBEX_IOC_MAGIC, FBEX_DEL_IV, FbeOpts)
#define FBEX_IOC_LOCK_SCREEN _IOW(FBEX_IOC_MAGIC, FBEX_LOCK_SCREEN, FbeOpts)
#define FBEX_IOC_UNLOCK_SCREEN _IOWR(FBEX_IOC_MAGIC, FBEX_UNLOCK_SCREEN, FbeOpts)
#define FBEX_IOC_USER_LOGOUT _IOW(FBEX_IOC_MAGIC, FBEX_USER_LOGOUT, FbeOpts)
#define FBEX_IOC_STATUS_REPORT _IOW(FBEX_IOC_MAGIC, FBEX_STATUS_REPORT, FbeOpts)
#define FBEX_READ_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_READ_EL5, FbeOptsE)
#define FBEX_WRITE_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_WRITE_EL5, FbeOptsE)
#define FBEX_ADD_CLASS_E _IOWR(FBEX_IOC_MAGIC, FBEX_ADD_EL5, FbeOptsE)
#define FBEX_DEL_USER_PINCODE _IOWR(FBEX_IOC_MAGIC, FBEX_DEL_EL5_PINCODE, FbeOptsE)
#define FBEX_ADD_APPKEY2 _IOWR(FBEX_IOC_MAGIC, FBEX_GENERATE_APP_KEY, FbeOptsE)
#define FBEX_CHANGE_PINCODE _IOWR(FBEX_IOC_MAGIC, FBEX_CHANGE_PINCODE, FbeOptsE)
#define FBEX_LOCK_UECE _IOWR(FBEX_IOC_MAGIC, FBEX_LOCK_EL5, FbeOptsE)
#define FBEX_UPDATE_UECE_KEY_CONTEXT _IOWR(FBEX_IOC_MAGIC, FBEX_UECE_BACKUP, FbeOptsE)

} // namespace

namespace OHOS {
namespace StorageDaemon {
bool FBEX::IsFBEXSupported()
{
    LOGD("[L7:FBEX] IsFBEXSupported: >>> ENTER <<<");
    std::string baseAddr;
    if (!OHOS::LoadStringFromFile(FBEX_UFS_INLINE_BASE_ADDR, baseAddr)) {
        LOGE("[L7:FBEX] IsFBEXSupported: <<< EXIT FAILED <<< Read baseAddr failed, errno: %{public}d", errno);
        return false;
    }

    std::string path = FBEX_UFS_INLINE_SUPPORT_PREFIX + baseAddr + FBEX_UFS_INLINE_SUPPORT_END;
    std::string nvmePath = FBEX_NVME_INLINE_SUPPORT_PATH;
    std::string emmcPath = FBEX_EMMC_INLINE_SUPPORT_PATH;
    std::string rpath(PATH_MAX + 1, '\0');

    if ((path.length() > PATH_MAX) || (realpath(path.c_str(), rpath.data()) == nullptr)) {
        LOGE("[L7:FBEX] IsFBEXSupported: realpath of %{public}s failed, errno: %{public}d", path.c_str(), errno);
        bool result = (access(nvmePath.c_str(), F_OK) == 0) || (access(emmcPath.c_str(), F_OK) == 0);
        LOGI("[L7:FBEX] IsFBEXSupported: <<< EXIT %s <<<", result ? "SUCCESS" : "FAILED");
        return result;
    }
    if (rpath.rfind(FBEX_UFS_INLINE_SUPPORT_PREFIX) != 0) {
        LOGE("[L7:FBEX] IsFBEXSupported: <<< EXIT FAILED <<< rpath %{public}s is invalid", rpath.c_str());
        return false;
    }

    std::string versionNum;
    if (!OHOS::LoadStringFromFile(rpath, versionNum)) {
        LOGE("[L7:FBEX] IsFBEXSupported: <<< EXIT FAILED <<< read ufs_inline_stat failed, errno: %{public}d", errno);
        return false;
    }
    bool result = versionNum.compare(FBEX_INLINE_CRYPTO_V3) == 0;
    LOGI("[L7:FBEX] IsFBEXSupported: <<< EXIT %s <<<", result ? "SUCCESS" : "FAILED");
    return result;
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

static inline int MemcpyFbeOptsV1(FbeOptsV1 &ops, const KeyBlob &authToken, uint8_t *iv, uint32_t size)
{
    int err = EOK;
    if (!authToken.IsEmpty()) {
        err = memcpy_s(ops.authToken, AUTH_TOKEN_MAX_SIZE, authToken.data.get(), authToken.size);
        LOGI("[L7:FBEX] MemcpyFbeOptsV1: memcpy end for v1, res is %{public}d", err);
    }
    err = memcpy_s(ops.iv, sizeof(ops.iv), iv, size);
    if (err != EOK) {
        LOGE("[L7:FBEX] MemcpyFbeOptsV1: memcpy failed %{public}d", err);
    }
    return err;
}

static inline int MemcpyFbeOptsEV1(FbeOptsEV1 &ops, const KeyBlob &authToken, uint8_t *eBuffer, uint32_t size)
{
    int err = EOK;
    if (!authToken.IsEmpty()) {
        err = memcpy_s(ops.authToken, AUTH_TOKEN_MAX_SIZE, authToken.data.get(), authToken.size);
        LOGI("[L7:FBEX] MemcpyFbeOptsEV1: memcpy end for ev1, res is %{public}d", err);
    }
    err = memcpy_s(ops.eBuffer, sizeof(ops.eBuffer), eBuffer, size);
    if (err != EOK) {
        LOGE("[L7:FBEX] MemcpyFbeOptsEV1: memcpy failed %{public}d", err);
    }
    return err;
}

int FBEX::InstallEL5KeyToKernel(uint32_t userIdSingle, uint32_t userIdDouble, uint8_t flag,
                                bool &isSupport, bool &isNeedEncryptClassE)
{
    LOGI("[L7:FBEX] InstallEL5KeyToKernel: >>> ENTER <<< userId: %{public}d, flag: %{public}u", userIdDouble, flag);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] InstallEL5KeyToKernel: fbex_uece does not exist, fbe not support this command!");
            isSupport = false;
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdDouble);
        StorageRadar::ReportFbexResult("InstallEL5KeyToKernel::open", userIdSingle, errno, "EL5", extraData);
        LOGE("[L7:FBEX] InstallEL5KeyToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOptsE ops{ .userIdDouble = userIdDouble, .userIdSingle = userIdSingle };
    auto fbeRet = ioctl(fd, FBEX_ADD_CLASS_E, &ops);
    if (static_cast<uint32_t>(fbeRet) == FILE_ENCRY_ERROR_UECE_ALREADY_CREATED) {
        LOGE("[L7:FBEX] InstallEL5KeyToKernel: class uece has already create, ret: 0x%{public}x, errno: %{public}d",
             fbeRet, errno);
        isNeedEncryptClassE = false;
        close(fd);
        LOGI("[L7:FBEX] InstallEL5KeyToKernel: <<< EXIT SUCCESS <<<");
        return 0;
    }
    int ret = 0;
    if (fbeRet != 0) {
        LOGE("[L7:FBEX] InstallEL5KeyToKernel: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d",
            fbeRet, errno);
        std::string extraData = "ioctl cmd=FBEX_ADD_CLASS_E, userIdSingle=" + std::to_string(userIdSingle)
            + ", userIdDouble=" + std::to_string(userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("InstallEL5KeyToKernel", userIdSingle, fbeRet, "EL5", extraData);
        ret = -errno;
    }
    close(fd);
    LOGI("[L7:FBEX] InstallEL5KeyToKernel: <<< EXIT %s <<<", ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int FBEX::InstallKeyToKernel(uint32_t userId, uint32_t type, KeyBlob &iv, uint8_t flag, const KeyBlob &authToken)
{
    LOGI("[L7:FBEX] InstallKeyToKernel: >>> ENTER <<< userId: %{public}d, type: %{public}u, flag: %{public}u",
        userId, type, flag);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    if (iv.IsEmpty() || !CheckIvValid(iv.data.get(), iv.size)) {
        LOGE("[L7:FBEX] InstallKeyToKernel: <<< EXIT FAILED <<< install key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        StorageRadar::ReportFbexResult("InstallKeyToKernel::open", userId, errno, std::to_string(type), "");
        LOGE("[L7:FBEX] InstallKeyToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOptsV1 ops{.user = userId, .type = type, .len = iv.size, .flag = flag, .authTokenSize = authToken.size};
    if (MemcpyFbeOptsV1(ops, authToken, iv.data.get(), iv.size) != EOK) {
        close(fd);
        LOGE("[L7:FBEX] InstallKeyToKernel: <<< EXIT FAILED <<<memcpyFbeOptsV1 failed");
        return 0;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("KEY TO KERNEL: FILE OPS",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("SD_DURATION: FBEX: FILE OPS: keyType=%{public}d, delay time = %{public}s", type, delay.c_str());
    startTime = StorageService::StorageRadar::RecordCurrentTime();
    int ret = ioctl(fd, FBEX_IOC_ADD_IV, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] InstallKeyToKernel: <<< EXIT FAILED <<< ioctl fbex_cmd failed, ret: 0x%{public}x, errno:"
            "%{public}d", ret, errno);
        std::string extraData = "ioctl cmd=FBEX_IOC_ADD_IV, errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("InstallKeyToKernel", userId, ret, std::to_string(type), extraData);
        close(fd);
        return ret;
    }
    close(fd);

    auto errops = memcpy_s(iv.data.get(), iv.size, ops.iv, sizeof(ops.iv));
    if (errops != EOK) {
        LOGE("[L7:FBEX] InstallKeyToKernel: memcpy failed %{public}d", errops);
    }
    (void)memset_s(&ops.iv, sizeof(ops.iv), 0, sizeof(ops.iv));
    delay = StorageService::StorageRadar::ReportDuration("FBEX: INSTALL KEY TO KERNEL",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userId);
    LOGI("[L7:FBEX] InstallKeyToKernel: <<< EXIT SUCCESS <<<");
    return ret;
}

int FBEX::InstallDoubleDeKeyToKernel(UserIdToFbeStr &userIdToFbe, KeyBlob &iv, uint8_t flag, const KeyBlob &authToken)
{
    LOGI("[L7:FBEX] InstallDoubleDeKeyToKernel: >>> ENTER <<< id-single: %{public}d, id-double: %{public}d, flag:"
        "%{public}u",
        userIdToFbe.userIds[SINGLE_ID_INDEX], userIdToFbe.userIds[DOUBLE_ID_INDEX], flag);
    if (iv.IsEmpty() || !CheckIvValid(iv.data.get(), iv.size)) {
        LOGE("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT FAILED <<< install key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        LOGE("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d",
            errno);
        return -errno;
    }

    FbeOptsEV1 ops{ .userIdDouble = userIdToFbe.userIds[DOUBLE_ID_INDEX],
                    .userIdSingle = userIdToFbe.userIds[SINGLE_ID_INDEX],
                    .status = flag, .length = iv.size, .authTokenSize = authToken.size };
    // eBuffer -> iv
    if (MemcpyFbeOptsEV1(ops, authToken, iv.data.get(), iv.size) != EOK) {
        LOGE("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT FAILED <<< memcpyFbeOptsEV1 failed");
        close(fd);
        return 0;
    }
    int ret = ioctl(fd, FBEX_IOC_ADD_DOUBLE_DE_IV, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT FAILED <<< ioctl fbex_cmd failed, ret: 0x%{public}x,"
             "errno: %{public}d", ret, errno);
        std::string extraData = "ioctl cmd=FBEX_IOC_ADD_DOUBLE_DE_IV, userIdSingle=" + std::to_string(ops.userIdSingle)
            + ", userIdDouble=" + std::to_string(ops.userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("InstallDoubleDeKeyToKernel", ops.userIdSingle, ret, "EL1", extraData);
        close(fd);
        return ret;
    }
    close(fd);
    auto errops = memcpy_s(iv.data.get(), iv.size, ops.eBuffer, sizeof(ops.eBuffer));
    if (errops != EOK) {
        LOGE("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT FAILED <<< memcpy failed %{public}d", errops);
        return 0;
    }
    LOGI("[L7:FBEX] InstallDoubleDeKeyToKernel: <<< EXIT SUCCESS <<<");
    return ret;
}

int FBEX::UninstallOrLockUserKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, bool destroy)
{
    LOGI("[L7:FBEX] UninstallOrLockUserKeyToKernel: >>> ENTER <<< userId: %{public}d, type: %{public}u, flag:"
        "%{public}d", userId, type, destroy);
    if (!CheckIvValid(iv, size)) {
        LOGE("[L7:FBEX] UninstallOrLockUserKeyToKernel: <<< EXIT FAILED <<< uninstall key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        StorageRadar::ReportFbexResult("UninstallOrLockUserKeyToKernel:open", userId, errno, std::to_string(type), "");
        LOGE("[L7:FBEX] UninstallOrLockUserKeyToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d",
            errno);
        return -errno;
    }

    FbeOpts ops{.user = userId, .type = type, .len = size};
    auto err = memcpy_s(ops.iv, sizeof(ops.iv), iv, size);
    if (err != EOK) {
        LOGE("[L7:FBEX] UninstallOrLockUserKeyToKernel: memcpy failed %{public}d", err);
        close(fd);
        LOGI("[L7:FBEX] UninstallOrLockUserKeyToKernel: <<< EXIT FAILED <<<");
        return 0;
    }
    int ret = ioctl(fd, destroy ? FBEX_IOC_DEL_IV : FBEX_IOC_USER_LOGOUT, &ops);
    if (ret != 0 && static_cast<uint32_t>(ret) != FILE_ENCRY_ERROR_NOT_FOUND_UECE) {
        LOGE("[L7:FBEX] UninstallOrLockUserKeyToKernel: <<< EXIT FAILED <<< ioctl fbex_cmd failed, ret: 0x%{public}x,"
            "errno: %{public}d", ret, errno);
        std::string febxCmd = destroy ? "FBEX_IOC_DEL_IV" : "FBEX_IOC_USER_LOGOUT";
        std::string extraData = "ioctl cmd=" + febxCmd + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("UninstallOrLockUserKeyToKernel", userId, ret, std::to_string(type), extraData);
        close(fd);
        return ret;
    }
    close(fd);
    LOGI("[L7:FBEX] UninstallOrLockUserKeyToKernel: <<< EXIT SUCCESS <<<");
    return 0;
}

int FBEX::DeleteClassEPinCode(uint32_t userIdSingle, uint32_t userIdDouble)
{
    LOGI("[L7:FBEX] DeleteClassEPinCode: >>> ENTER <<< userId: %{public}d", userIdDouble);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] DeleteClassEPinCode: fbex_uece does not exist, fbe not support this command, "
                 "<<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdDouble);
        StorageRadar::ReportFbexResult("DeleteClassEPinCode::open", userIdSingle, errno, "EL5", extraData);
        LOGE("[L7:FBEX] DeleteClassEPinCode: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{ .userIdDouble = userIdDouble, .userIdSingle = userIdSingle };
    auto fbeRet = ioctl(fd, FBEX_DEL_USER_PINCODE, &ops);
    int ret = 0;
    if (fbeRet != 0) {
        LOGE("[L7:FBEX] DeleteClassEPinCode: ioctl fbex_cmd failed, fbeRet: 0x%{public}x, errno: %{public}d",
            fbeRet, errno);
        std::string extraData = "ioctl cmd=FBEX_DEL_USER_PINCODE, userIdSingle=" + std::to_string(userIdSingle)
            + ", userIdDouble=" + std::to_string(userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("DeleteClassEPinCode", userIdSingle, ret, "EL5", extraData);
        ret = -errno;
    }
    close(fd);
    LOGI("[L7:FBEX] DeleteClassEPinCode: <<< EXIT %s <<<", ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int FBEX::ChangePinCodeClassE(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport)
{
    LOGI("[L7:FBEX] ChangePinCodeClassE: >>> ENTER <<< userId: %{public}d", userIdDouble);
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] ChangePinCodeClassE: fbex_uece does not exist, fbe not support this command!");
            isFbeSupport = false;
            LOGI("[L7:FBEX] ChangePinCodeClassE: <<< EXIT SUCCESS <<<");
            return 0;
        }
        LOGE("[L7:FBEX] ChangePinCodeClassE: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{ .userIdDouble = userIdDouble, .userIdSingle = userIdSingle };
    int ret = ioctl(fd, FBEX_CHANGE_PINCODE, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] ChangePinCodeClassE: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        std::string extraData = "ioctl cmd=FBEX_CHANGE_PINCODE, userIdSingle=" + std::to_string(userIdSingle)
            + ", userIdDouble=" + std::to_string(userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("ChangePinCodeClassE", userIdSingle, ret, "EL5", extraData);
        ret = -errno;
    }
    close(fd);
    LOGI("[L7:FBEX] ChangePinCodeClassE: change pincode classE finish, <<< EXIT %s <<<",
         ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int FBEX::UpdateClassEBackUp(uint32_t userIdSingle, uint32_t userIdDouble)
{
    LOGI("[L7:FBEX] UpdateClassEBackUp: >>> ENTER <<< userId: %{public}d", userIdDouble);
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] UpdateClassEBackUp: fbex_uece does not exist, fbe not support this command, "
                "<<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdDouble);
        StorageRadar::ReportFbexResult("UpdateClassEBackUp::open", userIdSingle, errno, "EL5", extraData);
        LOGE("[L7:FBEX] UpdateClassEBackUp: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    auto delay = StorageService::StorageRadar::ReportDuration("UPDATE E BACKUP: FILE OPS",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userIdSingle);
    LOGI("SD_DURATION: FBEX: FILE OPS: user=%{public}d, delay time = %{public}s", userIdSingle, delay.c_str());
    startTime = StorageService::StorageRadar::RecordCurrentTime();
    FbeOptsE ops{ .userIdDouble = userIdDouble, .userIdSingle = userIdSingle };
    int ret = ioctl(fd, FBEX_UPDATE_UECE_KEY_CONTEXT, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] UpdateClassEBackUp: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        ret = -errno;
        std::string extraData = "ioctl cmd=FBEX_UPDATE_UECE_KEY_CONTEXT, userIdSingle=" + std::to_string(userIdSingle)
            + ", userIdDouble=" + std::to_string(userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("FBEX_UPDATE_UECE_KEY_CONTEXT", userIdSingle, ret, "EL5", extraData);
    }
    delay = StorageService::StorageRadar::ReportDuration("FBEX:UPDATE_E_BACKUP",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, userIdSingle);
    LOGI("SD_DURATION: FBEX: CLASS E BACKUP: user=%{public}d, delay time = %{public}s", userIdSingle, delay.c_str());
    close(fd);
    LOGI("[L7:FBEX] UpdateClassEBackUp: update FBE key backup for classE finish, <<< EXIT %s <<<",
        ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

// for el3 & el4
int FBEX::LockScreenToKernel(uint32_t userId)
{
    LOGI("[L7:FBEX] LockScreenToKernel: >>> ENTER <<< userId: %{public}d", userId);

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        StorageRadar::ReportFbexResult("LockScreenToKernel::open", userId, errno, "", "");
        LOGE("[L7:FBEX] LockScreenToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOpts ops;
    (void)memset_s(&ops, sizeof(FbeOpts), 0, sizeof(FbeOpts));
    ops.user = userId;
    int ret = ioctl(fd, FBEX_IOC_LOCK_SCREEN, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] LockScreenToKernel: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
    }
    close(fd);
    LOGI("[L7:FBEX] LockScreenToKernel: <<< EXIT %s <<<", ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int FBEX::GenerateAppkey(UserIdToFbeStr &userIdToFbe, uint32_t hashId, std::unique_ptr<uint8_t[]> &appKey,
                         uint32_t size)
{
    LOGI("[L7:FBEX] GenerateAppkey: >>> ENTER <<<");
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] GenerateAppkey: fbex_uece does not exist, fbe not support this command!");
            appKey.reset(nullptr);
            LOGI("[L7:FBEX] GenerateAppkey: <<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdToFbe.userIds[DOUBLE_ID_INDEX]);
        StorageRadar::ReportFbexResult("GenerateAppkey::open", userIdToFbe.userIds[SINGLE_ID_INDEX],
                                       errno, "EL5", extraData);
        LOGE("[L7:FBEX] GenerateAppkey: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{ .userIdDouble = userIdToFbe.userIds[DOUBLE_ID_INDEX],
                  .userIdSingle = userIdToFbe.userIds[SINGLE_ID_INDEX],
                  .status = hashId, .length = size };
    auto fbeRet = ioctl(fd, FBEX_ADD_APPKEY2, &ops);
    if (fbeRet != 0) {
        LOGE("[L7:FBEX] GenerateAppkey: ioctl fbex_cmd failed, fbeRet: 0x%{public}x, errno: %{public}d", fbeRet, errno);
        close(fd);
        LOGI("[L7:FBEX] GenerateAppkey: <<< EXIT FAILED <<<");
        return -errno;
    }

    auto err = memcpy_s(appKey.get(), size, ops.eBuffer, sizeof(ops.eBuffer));
    if (err != EOK) {
        LOGE("[L7:FBEX] GenerateAppkey: memcpy failed %{public}d", err);
        close(fd);
        LOGI("[L7:FBEX] GenerateAppkey: <<< EXIT FAILED <<<");
        return 0;
    }
    close(fd);
    LOGI("[L7:FBEX] GenerateAppkey: <<< EXIT SUCCESS <<<");
    return 0;
}

// for el5
int FBEX::LockUece(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport)
{
    LOGI("[L7:FBEX] LockUece: >>> ENTER <<< userId: %{public}d", userIdDouble);

    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] LockUece: fbex_uece does not exist, fbe not support this command!");
            isFbeSupport = false;
            LOGI("[L7:FBEX] LockUece: <<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdDouble);
        StorageRadar::ReportFbexResult("GenerateAppkey::open", userIdSingle, errno, "EL5", extraData);
        LOGE("[L7:FBEX] LockUece: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOptsE ops{ .userIdDouble = userIdDouble, .userIdSingle = userIdSingle };
    int ret = ioctl(fd, FBEX_LOCK_UECE, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] LockUece: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
    }
    close(fd);
    LOGD("[L7:FBEX] LockUece: <<< EXIT %s <<<", ret == 0 ? "SUCCESS" : "FAILED");
    return ret;
}

int FBEX::UnlockScreenToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, const KeyBlob &authToken)
{
    LOGI("[L7:FBEX] UnlockScreenToKernel: >>> ENTER <<< userId: %{public}d, type: %{public}u", userId, type);
    if (!CheckIvValid(iv, size)) {
        LOGE("[L7:FBEX] UnlockScreenToKernel: <<< EXIT FAILED <<< install key param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        StorageRadar::ReportFbexResult("UnlockScreenToKernel::open", userId, errno, std::to_string(type), "");
        LOGE("[L7:FBEX] UnlockScreenToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }

    FbeOptsV1 ops{.user = userId, .type = type, .len = size, .authTokenSize = authToken.size};
    if (MemcpyFbeOptsV1(ops, authToken, iv, size) != EOK) {
        close(fd);
        LOGI("[L7:FBEX] UnlockScreenToKernel: <<< EXIT FAILED <<<");
        return 0;
    }

    int ret = ioctl(fd, FBEX_IOC_UNLOCK_SCREEN, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] UnlockScreenToKernel: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        std::string extraData = "ioctl cmd=FBEX_IOC_UNLOCK_SCREEN, errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("UnlockScreenToKernel", userId, ret, std::to_string(type), extraData);
        close(fd);
        LOGI("[L7:FBEX] UnlockScreenToKernel: <<< EXIT FAILED <<<");
        return ret;
    }
    close(fd);

    auto errops = memcpy_s(iv, size, ops.iv, sizeof(ops.iv));
    if (errops != EOK) {
        LOGE("[L7:FBEX] UnlockScreenToKernel: memcpy failed %{public}d", errops);
    }
    (void)memset_s(&ops.iv, sizeof(ops.iv), 0, sizeof(ops.iv));
    LOGI("[L7:FBEX] UnlockScreenToKernel: <<< EXIT SUCCESS <<<");
    return ret;
}

bool FBEX::CheckPreconditions(UserIdToFbeStr &userIdToFbe, uint32_t status, std::unique_ptr<uint8_t[]> &eBuffer,
                              uint32_t length, bool &isFbeSupport)
{
    LOGI("enter, userId: %{public}d, status: %{public}u", userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
    if (!CheckReadBuffValid(eBuffer.get(), length, status)) {
        LOGE("read e secret invalid");
        return false;
    }
    return true;
}

void FBEX::HandleIoctlError(int ret, int errnoVal, const std::string &cmd, uint32_t userIdSingle,
                            uint32_t userIdDouble)
{
    LOGE("[L7:FBEX] HandleIoctlError: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errnoVal);
    std::string extraData = "ioctl cmd=" + cmd + ", userIdSingle=" + std::to_string(userIdSingle)
                            + ", userIdDouble=" + std::to_string(userIdDouble) + ", errno=" + std::to_string(errnoVal);
    StorageRadar::ReportFbexResult("InstallDoubleDeKeyToKernel", userIdSingle, ret, "EL5", extraData);
}

int FBEX::ReadESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status, KeyBlob &eBuffer,
                              const KeyBlob &authToken, bool &isFbeSupport)
{
    LOGI("[L7:FBEX] ReadESecretToKernel: >>> ENTER <<< userId: %{public}d, status: %{public}u",
        userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
    if (eBuffer.IsEmpty() || !CheckPreconditions(userIdToFbe, status, eBuffer.data, eBuffer.size, isFbeSupport)) {
        LOGE("[L7:FBEX] ReadESecretToKernel: <<< EXIT FAILED <<< eBuffer is empty or CheckPreconditions failed");
        return -EINVAL;
    }
    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            isFbeSupport = false;
            LOGI("[L7:FBEX] ReadESecretToKernel: <<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdToFbe.userIds[DOUBLE_ID_INDEX]);
        StorageRadar::ReportFbexResult("ReadESecretToKernel::open", userIdToFbe.userIds[SINGLE_ID_INDEX],
            errno, "EL5", extraData);
        LOGE("[L7:FBEX] ReadESecretToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    uint32_t bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    FbeOptsEV1 ops{ .userIdDouble = userIdToFbe.userIds[DOUBLE_ID_INDEX],
                    .userIdSingle = userIdToFbe.userIds[SINGLE_ID_INDEX],
                    .status = status, .length = bufferSize, .authTokenSize = authToken.size };
    if (MemcpyFbeOptsEV1(ops, authToken, eBuffer.data.get(), eBuffer.size) != EOK) {
        close(fd);
        LOGI("[L7:FBEX] ReadESecretToKernel: <<< EXIT FAILED <<<");
        return 0;
    }
    auto ret = ioctl(fd, FBEX_READ_CLASS_E, &ops);
    if (ret != 0) {
        HandleIoctlError(ret, errno, "FBEX_READ_CLASS_E", ops.userIdSingle, ops.userIdDouble);
        close(fd);
        LOGI("[L7:FBEX] ReadESecretToKernel: <<< EXIT FAILED <<<");
        return (static_cast<uint32_t>(ret) == FILE_ENCRY_ERROR_UECE_AUTH_STATUS_WRONG) ? ret : -errno;
    }
    close(fd);
    if (ops.length == 0) {
        eBuffer.Clear();
        LOGE("[L7:FBEX] ReadESecretToKernel: ops length is 0, skip, <<< EXIT SUCCESS <<<");
        return 0;
    }
    UnlockSendSecret(status, bufferSize, eBuffer.size, eBuffer.data, ops.eBuffer);
    LOGI("[L7:FBEX] ReadESecretToKernel: <<< EXIT SUCCESS <<<");
    return 0;
}

int FBEX::UnlockSendSecret(uint32_t status, uint32_t bufferSize, uint32_t length, std::unique_ptr<uint8_t[]> &eBuffer,
                           uint8_t *opseBuffer)
{
    LOGD("[L7:FBEX] UnlockSendSecret: >>> ENTER <<< status=%{public}u", status);
    if (status == UNLOCK_STATUS) {
        bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    } else {
        bufferSize = AES_256_HASH_RANDOM_SIZE;
    }
    auto errBuffer = memcpy_s(eBuffer.get(), length, opseBuffer, bufferSize);
    if (errBuffer != EOK) {
        LOGE("[L7:FBEX] UnlockSendSecret: <<< EXIT FAILED <<< memcpy failed %{public}d", errBuffer);
        return 0;
    }
    LOGI("[L7:FBEX] UnlockSendSecret: <<< EXIT SUCCESS <<<");
    return 0;
}

int FBEX::WriteESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status, uint8_t *eBuffer, uint32_t length)
{
    LOGI("[L7:FBEX] WriteESecretToKernel: >>> ENTER <<< userId: %{public}d, status: %{public}u",
        userIdToFbe.userIds[DOUBLE_ID_INDEX], status);
    if (!CheckWriteBuffValid(eBuffer, length, status)) {
        LOGE("[L7:FBEX] WriteESecretToKernel: <<< EXIT FAILED <<< write e secret param invalid");
        return -EINVAL;
    }

    int fd = open(FBEX_UECE_PATH, O_RDWR);
    if (fd < 0) {
        if (errno == ENOENT) {
            LOGE("[L7:FBEX] WriteESecretToKernel: fbex_uece does not exist, fbe not support this command, "
                "<<< EXIT SUCCESS <<<");
            return 0;
        }
        std::string extraData = "userIdDouble=" + std::to_string(userIdToFbe.userIds[DOUBLE_ID_INDEX]);
        StorageRadar::ReportFbexResult("WriteESecretToKernel::open", userIdToFbe.userIds[SINGLE_ID_INDEX],
                                       errno, "EL5", extraData);
        LOGE("[L7:FBEX] WriteESecretToKernel: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    uint32_t bufferSize = AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES;
    FbeOptsE ops{ .userIdDouble = userIdToFbe.userIds[DOUBLE_ID_INDEX],
                  .userIdSingle = userIdToFbe.userIds[SINGLE_ID_INDEX],
                  .status = status, .length = bufferSize };
    auto err = memcpy_s(ops.eBuffer, sizeof(ops.eBuffer), eBuffer, length);
    if (err != EOK) {
        LOGE("[L7:FBEX] WriteESecretToKernel: memcpy failed %{public}d", err);
        close(fd);
        LOGI("[L7:FBEX] WriteESecretToKernel: <<< EXIT FAILED <<<");
        return 0;
    }
    auto ret = ioctl(fd, FBEX_WRITE_CLASS_E, &ops);
    if (ret != 0) {
        LOGE("[L7:FBEX] WriteESecretToKernel: ioctl fbex_cmd failed, ret: 0x%{public}x, errno: %{public}d", ret, errno);
        std::string extraData = "ioctl cmd=FBEX_WRITE_CLASS_E, userIdSingle=" + std::to_string(ops.userIdSingle)
            + ", userIdDouble=" + std::to_string(ops.userIdDouble) + ", errno=" + std::to_string(errno);
        StorageRadar::ReportFbexResult("InstallDoubleDeKeyToKernel", ops.userIdSingle, ret, "EL5", extraData);
        close(fd);
        LOGI("[L7:FBEX] WriteESecretToKernel: <<< EXIT FAILED <<<");
        return -errno;
    }
    close(fd);
    LOGI("[L7:FBEX] WriteESecretToKernel: <<< EXIT SUCCESS <<<");
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
    LOGD("[L7:FBEX] GetStatus: >>> ENTER <<<");
    int fd = open(FBEX_CMD_PATH, O_RDWR);
    if (fd < 0) {
        StorageRadar::ReportFbexResult("GetStatus::open", 0, errno, "", "");
        LOGE("[L7:FBEX] GetStatus: <<< EXIT FAILED <<< open fbex_cmd failed, errno: %{public}d", errno);
        return -errno;
    }
    FbeOpts ops;
    int ret = ioctl(fd, FBEX_IOC_STATUS_REPORT, &ops);
    close(fd);
    LOGI("[L7:FBEX] GetStatus: <<< EXIT %s <<<", ret >= 0 ? "SUCCESS" : "FAILED");
    return ret;
}
} // namespace StorageDaemon
} // namespace OHOS
