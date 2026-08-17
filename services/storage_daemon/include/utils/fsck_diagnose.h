/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef STORAGE_DAEMON_UTILS_FSCK_DIAGNOSE_H
#define STORAGE_DAEMON_UTILS_FSCK_DIAGNOSE_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace StorageDaemon {

constexpr int32_t FSCK_DIAGNOSE_TIMEOUT_S = 30;

struct FsckResult {
    int32_t exitCode = -1;
    std::string cmd;
    std::string output;
};

std::string GetFsckDiagnoseCmd(const std::string &devPath, const std::string &fsType);
FsckResult FsckDiagnose(const std::string &devPath, const std::string &fsType);
FsckResult FsckDiagnoseWithTimeout(const std::string &devPath, const std::string &fsType,
                                   int32_t timeoutSec = FSCK_DIAGNOSE_TIMEOUT_S);

} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_UTILS_FSCK_DIAGNOSE_H
