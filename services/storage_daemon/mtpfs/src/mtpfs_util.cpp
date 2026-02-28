/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mtpfs_util.h"

#include <algorithm>
#include <cctype>
#include <config.h>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <sstream>

#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <climits>
#include <libmtp.h>
#include <libusb.h>

#include "file_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

constexpr const char *DEV_NULL = "/dev/null";
static constexpr char PATH_INVALID_FLAG1[] = "../";
static constexpr char PATH_INVALID_FLAG2[] = "/..";
static constexpr char DECIMAL_MIN_CHAR = '0';
static constexpr char DECIMAL_MAX_CHAR = '9';
static constexpr char LOWER_HEX_MIN_CHAR = 'a';
static constexpr char LOWER_HEX_MAX_CHAR = 'f';
static constexpr char UPPER_HEX_MIN_CHAR = 'A';
static constexpr char UPPER_HEX_MAX_CHAR = 'F';
static constexpr char URL_ESCAPE_CHAR = '%';

static constexpr int32_t HEX_DECIMAL_BASE = 10;
static constexpr int32_t INVALID_HEX_VALUE = -1;

static constexpr size_t URL_ESCAPE_ENCODED_LEN = 3;
static constexpr size_t URL_ESCAPE_HEX_COUNT = 2;
static constexpr int32_t HEX_RADIX_SHIFT_BITS = 4;
static const char FILE_SEPARATOR_CHAR = '/';
constexpr int32_t MAX_INT = 255;
constexpr int32_t SCANF_NUM = 2;
static const uint32_t PATH_INVALID_FLAG_LEN = 3;

bool MtpFsUtil::enabled_ = false;
int MtpFsUtil::stdOut_ = -1;
int MtpFsUtil::stdErr_ = -1;

void MtpFsUtil::On()
{
    if (!enabled_) {
        return;
    }
    if (freopen(DEV_NULL, "w", stdout) == nullptr) {
        LOGE("freopen stdout fail");
    }
    if (freopen(DEV_NULL, "w", stderr) == nullptr) {
        LOGE("freopen stderr fail");
    }
    dup2(stdOut_, fileno(stdout));
    dup2(stdErr_, fileno(stderr));
    close(stdOut_);
    close(stdErr_);
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0) {
        LOGE("setvbuf stdout fail");
    }
    if (setvbuf(stderr, NULL, _IOLBF, 0) != 0) {
        LOGE("setvbuf stderr fail");
    }
    enabled_ = false;
}

void MtpFsUtil::Off()
{
    if (enabled_) {
        return;
    }
    fflush(stdout);
    fflush(stderr);
    stdOut_ = dup(fileno(stdout));
    stdErr_ = dup(fileno(stderr));
    if (freopen(DEV_NULL, "w", stdout) == nullptr) {
        LOGE("freopen stdout fail");
    }
    if (freopen(DEV_NULL, "w", stderr) == nullptr) {
        LOGE("freopen stderr fail");
    }
    enabled_ = true;
}

std::string SmtpfsDirName(const std::string &path)
{
    char *str = strdup(path.c_str());
    if (!str) {
        return "";
    }
    std::string result(dirname(str));
    free(static_cast<void *>(str));
    return result;
}

std::string SmtpfsBaseName(const std::string &path)
{
    std::unique_ptr<char, decltype(&std::free)> str(::strdup(path.c_str()), &std::free);
    if (!str) {
        return "";
    }
    return std::string(::basename(str.get()));
}

std::string SmtpfsRealPath(const std::string &path)
{
    if (path.length() > PATH_MAX) {
        return "";
    }
    char buf[PATH_MAX + 1];
    char *realPath = realpath(path.c_str(), buf);
    return std::string(realPath ? buf : "");
}

std::string SmtpfsGetTmpDir()
{
    const char *cTmp = "/data/local/mtp_tmp";
    DelTemp(cTmp);
    std::string tmpDir = SmtpfsRealPath(cTmp) + "/simple-mtpfs-XXXXXX";
    char *cTempDir = ::strdup(tmpDir.c_str());
    if (cTempDir == nullptr) {
        return "";
    }
    char *cTmpDir = ::mkdtemp(cTempDir);
    if (cTmpDir == nullptr) {
        ::free(cTempDir);
        return "";
    }
    tmpDir.assign(cTmpDir);
    ::free(static_cast<void *>(cTempDir));
    return tmpDir;
}

void DelTemp(const std::string &path)
{
    DIR *dir;
    if (!OHOS::StorageDaemon::IsDir(path)) {
        return;
    }
    if ((dir = opendir(path.c_str())) != nullptr) {
        struct dirent *dirinfo = ::readdir(dir);
        while (dirinfo != nullptr) {
            if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {
                dirinfo = ::readdir(dir);
                continue;
            }
            std::string filePath;
            filePath.append(path).append("/").append(dirinfo->d_name);
            if (OHOS::StorageDaemon::IsTempFolder(filePath, "simple-mtpfs")) {
                OHOS::StorageDaemon::DeleteFile(filePath.c_str());
                rmdir(filePath.c_str());
            }
            dirinfo = ::readdir(dir);
        }
        closedir(dir);
    }
}

bool SmtpfsCreateDir(const std::string &dirName)
{
    return ::mkdir(dirName.c_str(), S_IRWXU) == 0;
}

bool SmtpfsRemoveDir(const std::string &dirName)
{
    DIR *dir = ::opendir(dirName.c_str());
    if (dir == nullptr) {
        return false;
    }
    struct dirent *entry = ::readdir(dir);
    while (entry != nullptr) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            std::string path = dirName + "/" + entry->d_name;
            if (entry->d_type == DT_DIR) {
                ::closedir(dir);
                return SmtpfsRemoveDir(path);
            }
            auto ret = ::unlink(path.c_str());
            if (ret != OHOS::E_OK) {
                LOGE("SmtpfsRemoveDir: unlink error, errno=%{public}d", ret);
            }
        }
        entry = ::readdir(dir);
    }
    ::closedir(dir);
    auto ret = ::remove(dirName.c_str());
    if (ret != OHOS::E_OK) {
        LOGE("SmtpfsRemoveDir: remove error, errno=%{public}d", ret);
    }
    return true;
}

bool SmtpfsUsbDevPath(const std::string &path, uint8_t *bnum, uint8_t *dnum)
{
    unsigned int bus;
    unsigned int dev;
#ifdef USB_DEVPATH
    std::string realPath(SmtpfsRealPath(path));
    if (realPath.empty() || sscanf_s(realPath.c_str(), USB_DEVPATH, &bus, &dev) != SCANF_NUM) {
#endif
        if (sscanf_s(path.c_str(), "%u/%u", &bus, &dev) != SCANF_NUM) {
            return false;
        }
    }
    if (bus > MAX_INT || dev > MAX_INT) {
        return false;
    }

    *bnum = bus;
    *dnum = dev;
    return true;
}

LIBMTP_raw_device_t *smtpfs_raw_device_new_priv(libusb_device *usb_device)
{
    if (!usb_device) {
        return nullptr;
    }

    LIBMTP_raw_device_t *device = static_cast<LIBMTP_raw_device_t *>(malloc(sizeof(LIBMTP_raw_device_t)));

    if (device == nullptr) {
        return nullptr;
    }

    struct libusb_device_descriptor desc;
    int err = libusb_get_device_descriptor(usb_device, &desc);
    if (err != LIBUSB_SUCCESS) {
        free(static_cast<void *>(device));
        return nullptr;
    }

    device->device_entry.vendor = nullptr;
    device->device_entry.vendor_id = desc.idVendor;
    device->device_entry.product = nullptr;
    device->device_entry.product_id = desc.idProduct;
    device->device_entry.device_flags = 0;

    device->bus_location = static_cast<uint32_t>(libusb_get_bus_number(usb_device));
    device->devnum = libusb_get_device_address(usb_device);

    return device;
}

LIBMTP_raw_device_t *SmtpfsRawDeviceNew(const std::string &path)
{
    uint8_t bnum;
    uint8_t dnum;
    if (!SmtpfsUsbDevPath(path, &bnum, &dnum)) {
        return nullptr;
    }

    if (libusb_init(NULL) != 0) {
        return nullptr;
    }

    libusb_device **devList;
    ssize_t numDevs = libusb_get_device_list(NULL, &devList);
    if (!numDevs) {
        libusb_exit(NULL);
        return nullptr;
    }

    libusb_device *dev = nullptr;
    for (auto i = 0; i < numDevs; ++i) {
        dev = devList[i];
        if (bnum == libusb_get_bus_number(devList[i]) && dnum == libusb_get_device_address(devList[i])) {
            break;
        }
        dev = nullptr;
    }

    LIBMTP_raw_device_t *rawDevice = smtpfs_raw_device_new_priv(dev);

    libusb_free_device_list(devList, 0);
    libusb_exit(NULL);

    return rawDevice;
}

bool SmtpfsResetDevice(LIBMTP_raw_device_t *device)
{
    if (device == nullptr) {
        LOGE("device is null");
        return false;
    }

    if (libusb_init(NULL) != 0) {
        return false;
    }

    libusb_device **devList;
    ssize_t numDevs = libusb_get_device_list(NULL, &devList);
    if (!numDevs) {
        libusb_exit(NULL);
        return false;
    }

    libusb_device_handle *devHandle = nullptr;
    for (auto i = 0; i < numDevs; ++i) {
        uint8_t bnum = libusb_get_bus_number(devList[i]);
        uint8_t dnum = libusb_get_device_address(devList[i]);
        if (static_cast<uint32_t>(bnum) == device->bus_location && dnum == device->devnum) {
            libusb_open(devList[i], &devHandle);
            libusb_reset_device(devHandle);
            libusb_close(devHandle);
            break;
        }
    }

    libusb_free_device_list(devList, 0);
    libusb_exit(NULL);

    return true;
}

void SmtpfsRawDeviceFree(LIBMTP_raw_device_t *device)
{
    if (device == nullptr) {
        return;
    }

    free(static_cast<void *>(device->device_entry.vendor));
    free(static_cast<void *>(device->device_entry.product));
    free(static_cast<void *>(device));
}

bool SmtpfsCheckDir(const std::string &path)
{
    struct stat buf;
    if (::stat(path.c_str(), &buf) == 0 && S_ISDIR(buf.st_mode) && ::access(path.c_str(), R_OK | W_OK | X_OK) == 0) {
        return true;
    }

    return false;
}

static inline int32_t HexVal(char c)
{
    if ((c >= DECIMAL_MIN_CHAR) && (c <= DECIMAL_MAX_CHAR)) {
        return static_cast<std::int32_t>(c - DECIMAL_MIN_CHAR);
    }

    if ((c >= LOWER_HEX_MIN_CHAR) && (c <= LOWER_HEX_MAX_CHAR)) {
        return static_cast<std::int32_t>(c - LOWER_HEX_MIN_CHAR) + HEX_DECIMAL_BASE;
    }

    if ((c >= UPPER_HEX_MIN_CHAR) && (c <= UPPER_HEX_MAX_CHAR)) {
        return static_cast<std::int32_t>(c - UPPER_HEX_MIN_CHAR) + HEX_DECIMAL_BASE;
    }
    return INVALID_HEX_VALUE;
}

static bool UrlDecodeOnce(const std::string& in, std::string& out)
{
    out.clear();
    out.reserve(in.size());

    size_t i = 0;
    while (i < in.size()) {
        if ((in[i] == URL_ESCAPE_CHAR) && (i + URL_ESCAPE_HEX_COUNT < in.size())) {
            int32_t h1 = HexVal(in[i + 1]);
            int32_t h2 = HexVal(in[i + 2]);
            if ((h1 >= 0) && (h2 >= 0)) {
                out.push_back(static_cast<char>((h1 << HEX_RADIX_SHIFT_BITS) | h2));
                i += URL_ESCAPE_ENCODED_LEN;
                continue;
            }
        }
        out.push_back(in[i]);
        ++i;
    }
    return true;
}

bool IsFilePathValid(const std::string &filePath)
{
    if (filePath.empty()) {
        LOGE("Empty path is not allowed");
        return false;
    }
    std::string decoded1;
    std::string decoded2;
    UrlDecodeOnce(filePath, decoded1);
    UrlDecodeOnce(decoded1, decoded2);

    std::replace(decoded2.begin(), decoded2.end(), '\\', '/');

    size_t i = 0;
    while (i < decoded2.size()) {
        while (i < decoded2.size() && decoded2[i] == '/') {
            i++;
        }
        size_t j = i;
        while (j < decoded2.size() && decoded2[j] != '/') {
            j++;
        }

        if (j > i) {
            std::string_view seg(decoded2.data() + i, j - i);
            if (seg == "." || seg == "..") {
                LOGE("Relative path segment '.' or '..' is not allowed");
                return false;
            }
        }
        i = j;
    }

    return true;
}
