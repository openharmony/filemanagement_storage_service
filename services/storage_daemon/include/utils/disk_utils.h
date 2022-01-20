/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
const int diskMmc = 179;
const int maxScsiVolumes = 15;
const std::string SysfsMmcMaxVolumes = "/sys/module/mmc_block/parameters/perdev_minors";

int CreateDiskNode(const std::string &path, dev_t dev);
int DestroyDiskNode(const std::string &path);
int GetDevSize(std::string path, uint64_t *size);
int GetMaxVolume(dev_t device);

} // namespace STORAGE_DAEMON
} // namespace OHOS

#endif // STORAGE_DAEMON_UTILS_DISK_H