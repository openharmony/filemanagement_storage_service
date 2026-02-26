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

#ifndef MTPFS_UTIL_H
#define MTPFS_UTIL_H

#include <config.h>
#include <cstdint>
#include <string>

#include <libmtp.h>

class MtpFsUtil {
public:
    MtpFsUtil() = delete;

    static void On();
    static void Off();

private:
    static bool enabled_;
    static int stdOut_;
    static int stdErr_;
};

std::string SmtpfsDirName(const std::string &path);
std::string SmtpfsBaseName(const std::string &path);
std::string SmtpfsRealPath(const std::string &path);
std::string SmtpfsGetTmpDir();

bool SmtpfsCreateDir(const std::string &dirName);
bool SmtpfsRemoveDir(const std::string &dirName);
bool SmtpfsCheckDir(const std::string &path);
bool SmtpfsUsbDevPath(const std::string &path, uint8_t *bnum, uint8_t *dnum);
bool IsFilePathValid(const std::string &filePath);
void DelTemp(const std::string &path);

LIBMTP_raw_device_t *SmtpfsRawDeviceNew(const std::string &path);
void SmtpfsRawDeviceFree(LIBMTP_raw_device_t *device);
bool SmtpfsResetDevice(LIBMTP_raw_device_t *device);

#endif // MTPFS_UTIL_H
