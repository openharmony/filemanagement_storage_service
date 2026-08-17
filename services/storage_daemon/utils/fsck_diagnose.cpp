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
#include <csignal>
#include <poll.h>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "file_utils.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
constexpr size_t MAX_OUTPUT_LEN = 1024;
constexpr int32_t PIPE_FD_LEN = 2;
constexpr int32_t SIGNAL_EXIT_BASE = 128;
constexpr int32_t SIGKILL_EXIT = SIGNAL_EXIT_BASE + SIGKILL;
constexpr int32_t PIPE_BUF_LEN = 1024;

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
        cmd = {"fsck.f2fs", "--dry-run", "-p1", devPath};
        return true;
    }
    return false;
}

std::string JoinOutput(const std::vector<std::string> &chunks)
{
    std::string text;
    for (const auto &chunk : chunks) {
        text += chunk;
    }
    return text.size() > MAX_OUTPUT_LEN ? text.substr(0, MAX_OUTPUT_LEN) : text;
}

int32_t GetExitCode(int status)
{
    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return SIGNAL_EXIT_BASE + WTERMSIG(status);
    }
    return SIGKILL_EXIT;
}

void ReadPipe(int fd, std::vector<std::string> &output)
{
    char buf[PIPE_BUF_LEN] = {0};
    ssize_t n = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        output.emplace_back(buf, static_cast<size_t>(n));
    }
}

bool WaitFsck(pid_t pid, int pipeRead, int32_t timeoutSec, int &status,
              std::vector<std::string> &output)
{
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeoutSec);
    while (std::chrono::steady_clock::now() < deadline) {
        if (waitpid(pid, &status, WNOHANG) == pid) {
            ReadPipe(pipeRead, output);
            return false;
        }
        int ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now()).count());
        pollfd pfd {};
        pfd.fd = pipeRead;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, ms < 0 ? 0 : ms) <= 0) {
            continue;
        }
        char buf[PIPE_BUF_LEN] = {0};
        ssize_t n = read(pipeRead, buf, sizeof(buf));
        if (n > 0) {
            output.emplace_back(buf, static_cast<size_t>(n));
            continue;
        }
        (void)waitpid(pid, &status, 0);
        return false;
    }
    if (waitpid(pid, &status, WNOHANG) == pid) {
        ReadPipe(pipeRead, output);
        return false;
    }
    return true;
}

void FillTimeoutOutput(FsckResult &result, int32_t timeoutSec)
{
    const std::string mark = "fsck diagnose timeout after " + std::to_string(timeoutSec) + "s";
    if (!result.output.empty() && result.output.back() != '\n') {
        result.output += '\n';
    }
    result.output += mark;
    if (result.output.size() > MAX_OUTPUT_LEN) {
        result.output.resize(MAX_OUTPUT_LEN);
    }
}

void ExecFsckChild(std::vector<std::string> &cmd, int pipeFd[PIPE_FD_LEN])
{
    (void)setpgid(0, 0);
    if (RedirectStdToPipe(pipeFd, PIPE_FD_LEN) != E_OK) {
        _exit(1);
    }
    std::vector<char *> args;
    for (auto &item : cmd) {
        args.push_back(const_cast<char *>(item.c_str()));
    }
    args.push_back(nullptr);
    execvp(args[0], args.data());
    _exit(1);
}

FsckResult RunFsck(const std::string &devPath, const std::string &fsType, int32_t timeoutSec)
{
    FsckResult result;
    std::vector<std::string> cmd;
    if (!BuildFsckCmd(devPath, fsType, cmd)) {
        LOGW("FsckDiagnose: unsupported fsType %{public}s", fsType.c_str());
        return result;
    }
    result.cmd = JoinCmd(cmd);
    int pipeFd[PIPE_FD_LEN] = {-1, -1};
    if (pipe(pipeFd) < 0) {
        result.ret = E_CREATE_PIPE;
        return result;
    }
    pid_t pid = fork();
    if (pid < 0) {
        (void)close(pipeFd[0]);
        (void)close(pipeFd[1]);
        result.ret = E_FORK;
        return result;
    }
    if (pid == 0) {
        ExecFsckChild(cmd, pipeFd);
    }
    (void)setpgid(pid, pid);
    (void)close(pipeFd[1]);
    int status = 0;
    std::vector<std::string> output;
    int32_t waitSec = timeoutSec < 0 ? 0 : timeoutSec;
    bool timedOut = WaitFsck(pid, pipeFd[0], waitSec, status, output);
    if (timedOut) {
        (void)killpg(pid, SIGKILL);
        (void)waitpid(pid, &status, 0);
        ReadPipe(pipeFd[0], output);
        LOGE("FsckDiagnose: timeout fsType=%{public}s timeout=%{public}d", fsType.c_str(), waitSec);
    }
    (void)close(pipeFd[0]);
    result.exitCode = GetExitCode(status);
    result.output = JoinOutput(output);
    if (timedOut) {
        FillTimeoutOutput(result, waitSec);
    }
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
    return RunFsck(devPath, fsType, FSCK_DIAGNOSE_TIMEOUT_S);
}

FsckResult FsckDiagnoseWithTimeout(const std::string &devPath, const std::string &fsType, int32_t timeoutSec)
{
    return RunFsck(devPath, fsType, timeoutSec);
}
} // namespace StorageDaemon
} // namespace OHOS
