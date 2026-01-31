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
#include "storage_service_log.h"

constexpr const char *DEV_NULL = "/dev/null";
static constexpr char PATH_INVALID_FLAG1[] = "../";
static constexpr char PATH_INVALID_FLAG2[] = "/..";
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
    OHOS::StorageDaemon::DelTemp(cTmp);
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

bool SmtpfsCreateDir(const std::string &dirName)
{
    return ::mkdir(dirName.c_str(), S_IRWXU) == 0;
}

bool SmtpfsRemoveDir(const std::string &dirName)
{
    DIR *dir;
    struct dirent *entry;
    std::string path;

    dir = ::opendir(dirName.c_str());
    if (dir == nullptr) {
        return false;
    }
    while ((entry = ::readdir(dir))) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            path = dirName + "/" + entry->d_name;
            if (entry->d_type == DT_DIR) {
                ::closedir(dir);
                return SmtpfsRemoveDir(path);
            }
            ::unlink(path.c_str());
        }
    }
    ::closedir(dir);
    ::remove(dirName.c_str());
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

bool MtpFileSystem::IsFilePathValid(const std::string &filePath)
{
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("Relative path is not allowed, path contain ../");
            return false;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("Relative path is not allowed, path tail is /..");
        return false;
    }
    return true;
}
