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

#ifndef STORAGE_DAEMON_UTILS_VOLUME_OP_DIAG_H
#define STORAGE_DAEMON_UTILS_VOLUME_OP_DIAG_H

#include <cstdint>
#include <string>
#include <vector>

#include "utils/fsck_diagnose.h"

namespace OHOS {
namespace StorageDaemon {

struct VolumeOpDiagToolEntry {
    std::string cmd;
    int32_t ret = 0;
    int32_t exitCode = 0;
    std::string output;
};

// Provider sets TLS context at IPC entry (DISK_MANAGER IPC only). Tool failures
// are accumulated via VolumeOpDiagReportToolFailure; Provider flushes on exit via
// VolumeOpDiagFlushFailureReport, then schedules async fsck diagnose report.
// Actual IPC upload requires DISK_MANAGER.
// For worker threads, capture context on the IPC thread, AttachContext before ForkExec*,
// then MergeToolEntries after join.

struct VolumeOpDiagContext {
    bool active = false;
    bool reported = false;
    std::string funcName;
    int32_t bizStage = 0;
    int32_t opType = 0;
    std::string devPath;
    std::string fsType;
    std::vector<VolumeOpDiagToolEntry> toolEntries;
};

void VolumeOpDiagBegin(const VolumeOpDiagContext &ctx);
void VolumeOpDiagEnd();
VolumeOpDiagContext VolumeOpDiagCaptureContext();
void VolumeOpDiagAttachContext(const VolumeOpDiagContext &ctx);
bool VolumeOpDiagWasReported();

void VolumeOpDiagReportToolFailure(const std::vector<std::string> &cmd, int32_t ret, int32_t exitCode,
                                   const std::vector<std::string> *output);
void VolumeOpDiagAppendToolEntry(const VolumeOpDiagToolEntry &entry);
std::vector<VolumeOpDiagToolEntry> VolumeOpDiagTakeToolEntries();
void VolumeOpDiagMergeToolEntries(const std::vector<VolumeOpDiagToolEntry> &entries);
void VolumeOpDiagUpdateDevPath(const std::string &devPath);
void VolumeOpDiagFlushFailureReport(int32_t ret);
void VolumeOpDiagScheduleAsyncFsckReport(int32_t ret, const VolumeOpDiagContext &ctx);

} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_UTILS_VOLUME_OP_DIAG_H
