/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_UTILS_DISK_H
#define STORAGE_DAEMON_UTILS_DISK_H

#include <string>

#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
constexpr int DISK_MMC_MAJOR = 179;
constexpr int MAX_SCSI_VOLUMES = 15;

int CreateDiskNode(const std::string &path, dev_t dev);
int DestroyDiskNode(const std::string &path);
int GetDevSize(const std::string &path, uint64_t *size);
int GetMaxVolume(dev_t device);
int32_t ReadMetadata(const std::string &path, std::string &uuid, std::string &type, std::string &label);
int32_t ReadVolumeUuid(const std::string &devPath, std::string &uuid);
std::string GetBlkidData(const std::string &devPath, const std::string &type);
std::string GetBlkidDataByCmd(std::vector<std::string> &cmd);
std::string GetAnonyString(const std::string &value);
std::string GenerateRandomUuid();
} // namespace STORAGE_DAEMON
} // namespace OHOS

#endif // STORAGE_DAEMON_UTILS_DISK_H