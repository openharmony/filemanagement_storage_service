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

#include "utils/fsck_diagnose.h"

#include <chrono>
#include <future>
#include <sstream>
#include <thread>
#include <vector>

#include "file_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
constexpr size_t MAX_OUTPUT_LEN = 1024;

std::string TruncateOutput(const std::string &text)
{
    return text.size() > MAX_OUTPUT_LEN ? text.substr(0, MAX_OUTPUT_LEN) : text;
}

std::string JoinOutputLines(const std::vector<std::string> &output)
{
    std::ostringstream oss;
    for (size_t i = 0; i < output.size(); ++i) {
        if (i > 0) {
            oss << '\n';
        }
        oss << output[i];
    }
    return oss.str();
}

std::string JoinCmd(const std::vector<std::string> &cmd)
{
    std::ostringstream oss;
    for (size_t i = 0; i < cmd.size(); ++i) {
        if (i > 0) {
            oss << ' ';
        }
        oss << cmd[i];
    }
    return oss.str();
}

bool BuildFsckCmd(const std::string &devPath, const std::string &fsType, std::vector<std::string> &cmd)
{
    if (fsType == "exfat") {
        cmd = {"fsck.exfat", "-n", devPath};
        return true;
    }
    if (fsType == "ntfs") {
        cmd = {"fsck.ntfs", devPath};
        return true;
    }
    if (fsType == "vfat" || fsType == "fat32") {
        cmd = {"fsck_msdos", devPath};
        return true;
    }
    if (fsType == "ext4") {
        cmd = {"e2fsck", "-n", devPath};
        return true;
    }
    if (fsType == "f2fs" || fsType == "hmfs") {
        cmd = {"fsck.f2fs", devPath};
        return true;
    }
    return false;
}

FsckResult RunFsckExec(const std::string &devPath, const std::string &fsType)
{
    FsckResult result;
    std::vector<std::string> cmd;
    if (!BuildFsckCmd(devPath, fsType, cmd)) {
        LOGW("FsckDiagnose: unsupported fsType %{public}s", fsType.c_str());
        return result;
    }

    result.cmd = JoinCmd(cmd);
    int32_t exitStatus = -1;
    std::vector<std::string> output;
    const int32_t execRet = ForkExecWithExit(cmd, &exitStatus, &output);
    result.exitCode = (execRet == E_OK) ? exitStatus : execRet;
    result.output = TruncateOutput(JoinOutputLines(output));
    LOGI("FsckDiagnose: fsType=%{public}s exitCode=%{public}d", fsType.c_str(), result.exitCode);
    return result;
}

FsckResult MakeFsckTimeoutResult(const std::string &devPath, const std::string &fsType, int32_t timeoutSec)
{
    FsckResult result;
    std::vector<std::string> cmd;
    if (BuildFsckCmd(devPath, fsType, cmd)) {
        result.cmd = JoinCmd(cmd);
    }
    result.exitCode = -1;
    result.output = "fsck diagnose timeout after " + std::to_string(timeoutSec) + "s";
    LOGE("FsckDiagnose: timeout fsType=%{public}s timeout=%{public}d", fsType.c_str(), timeoutSec);
    return result;
}
} // namespace

std::string GetFsckDiagnoseCmd(const std::string &devPath, const std::string &fsType)
{
    std::vector<std::string> cmd;
    return BuildFsckCmd(devPath, fsType, cmd) ? JoinCmd(cmd) : "";
}

FsckResult FsckDiagnose(const std::string &devPath, const std::string &fsType)
{
    return RunFsckExec(devPath, fsType);
}

FsckResult FsckDiagnoseWithTimeout(const std::string &devPath, const std::string &fsType, int32_t timeoutSec)
{
    if (GetFsckDiagnoseCmd(devPath, fsType).empty()) {
        LOGW("FsckDiagnoseWithTimeout: unsupported fsType %{public}s", fsType.c_str());
        return FsckResult {};
    }

    std::promise<FsckResult> promise;
    std::future<FsckResult> future = promise.get_future();
    std::thread worker([devPath, fsType, p = std::move(promise)]() mutable {
        p.set_value(RunFsckExec(devPath, fsType));
    });
    if (future.wait_for(std::chrono::seconds(timeoutSec)) == std::future_status::timeout) {
        worker.detach();
        return MakeFsckTimeoutResult(devPath, fsType, timeoutSec);
    }
    FsckResult result = future.get();
    worker.join();
    return result;
}
} // namespace StorageDaemon
} // namespace OHOS
