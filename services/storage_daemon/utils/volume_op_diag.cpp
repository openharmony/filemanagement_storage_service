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

#include "utils/volume_op_diag.h"

#include <sstream>
#include <thread>

#ifdef DISK_MANAGER
#include "disk_manager_client.h"
#endif
#include "nlohmann/json.hpp"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
namespace {
constexpr size_t MAX_TOOL_OUTPUT_LINES = 16;
constexpr size_t MAX_TOOL_LINE_LEN = 256;

thread_local VolumeOpDiagContext g_tlsContext;

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

std::string JoinOutput(const std::vector<std::string> *output)
{
    if (output == nullptr || output->empty()) {
        return "";
    }
    std::ostringstream oss;
    size_t lines = 0;
    for (const auto &line : *output) {
        if (lines >= MAX_TOOL_OUTPUT_LINES) {
            break;
        }
        if (lines > 0) {
            oss << '\n';
        }
        oss << (line.size() > MAX_TOOL_LINE_LEN ? line.substr(0, MAX_TOOL_LINE_LEN) : line);
        ++lines;
    }
    return oss.str();
}

nlohmann::json BuildToolJson(const VolumeOpDiagToolEntry &entry)
{
    return {{"cmd", entry.cmd},
            {"ret", entry.ret},
            {"exitCode", entry.exitCode},
            {"output", entry.output}};
}

nlohmann::json BuildDiagJson(int32_t ret, const VolumeOpDiagContext &ctx)
{
    nlohmann::json js;
    js["ret"] = ret;
    js["funcName"] = ctx.funcName;
    js["bizStage"] = ctx.bizStage;
    js["opType"] = ctx.opType;
    if (!ctx.devPath.empty()) {
        js["devPath"] = ctx.devPath;
    }
    if (!ctx.fsType.empty()) {
        js["fsType"] = ctx.fsType;
    }
    if (!ctx.toolEntries.empty()) {
        nlohmann::json tools = nlohmann::json::array();
        for (const auto &entry : ctx.toolEntries) {
            tools.push_back(BuildToolJson(entry));
        }
        js["tools"] = tools;
    }
    return js;
}

void SendDiagReport(const nlohmann::json &js)
{
    const std::string diagJson = js.dump();
    LOGI("VolumeOpDiag report len=%{public}zu func=%{public}s hasTools=%{public}d",
         diagJson.size(), js.value("funcName", "").c_str(), js.contains("tools") ? 1 : 0);
#ifdef DISK_MANAGER
    (void)DiskManager::DiskManagerClient::GetInstance().ReportVolumeOpDiag(diagJson);
#endif
}
} // namespace

void VolumeOpDiagBegin(const VolumeOpDiagContext &ctx)
{
    g_tlsContext = ctx;
    g_tlsContext.active = true;
    g_tlsContext.reported = false;
    g_tlsContext.toolEntries.clear();
}

void VolumeOpDiagEnd()
{
    g_tlsContext = {};
}

VolumeOpDiagContext VolumeOpDiagCaptureContext()
{
    return g_tlsContext;
}

void VolumeOpDiagAttachContext(const VolumeOpDiagContext &ctx)
{
    g_tlsContext = ctx;
}

bool VolumeOpDiagWasReported()
{
    return g_tlsContext.reported;
}

void VolumeOpDiagAppendToolEntry(const VolumeOpDiagToolEntry &entry)
{
    if (!g_tlsContext.active) {
        return;
    }
    g_tlsContext.toolEntries.push_back(entry);
}

void VolumeOpDiagReportToolFailure(const std::vector<std::string> &cmd, int32_t ret, int32_t exitCode,
                                   const std::vector<std::string> *output)
{
    if (!g_tlsContext.active) {
        return;
    }
    VolumeOpDiagToolEntry entry;
    entry.cmd = JoinCmd(cmd);
    entry.ret = ret;
    entry.exitCode = exitCode;
    entry.output = JoinOutput(output);
    VolumeOpDiagAppendToolEntry(entry);
}

VolumeOpDiagToolEntry MakeFsckToolEntry(const FsckResult &fsck)
{
    VolumeOpDiagToolEntry entry;
    entry.cmd = fsck.cmd;
    entry.ret = fsck.exitCode >= 0 ? 0 : fsck.exitCode;
    entry.exitCode = fsck.exitCode;
    entry.output = fsck.output;
    return entry;
}

void SendFsckDiagReport(int32_t ret, const VolumeOpDiagContext &ctx, const FsckResult &fsck)
{
    VolumeOpDiagContext reportCtx = ctx;
    reportCtx.active = true;
    reportCtx.reported = false;
    reportCtx.toolEntries = {MakeFsckToolEntry(fsck)};
    SendDiagReport(BuildDiagJson(ret, reportCtx));
}

void AsyncFsckReportTask(int32_t ret, VolumeOpDiagContext ctx)
{
    const FsckResult fsck = FsckDiagnoseWithTimeout(ctx.devPath, ctx.fsType, FSCK_DIAGNOSE_TIMEOUT_S);
    if (fsck.cmd.empty()) {
        return;
    }
    SendFsckDiagReport(ret, ctx, fsck);
}

std::vector<VolumeOpDiagToolEntry> VolumeOpDiagTakeToolEntries()
{
    std::vector<VolumeOpDiagToolEntry> entries = std::move(g_tlsContext.toolEntries);
    g_tlsContext.toolEntries.clear();
    return entries;
}

void VolumeOpDiagMergeToolEntries(const std::vector<VolumeOpDiagToolEntry> &entries)
{
    if (!g_tlsContext.active || entries.empty()) {
        return;
    }
    g_tlsContext.toolEntries.insert(g_tlsContext.toolEntries.end(), entries.begin(), entries.end());
}

void VolumeOpDiagUpdateDevPath(const std::string &devPath)
{
    if (g_tlsContext.active && !devPath.empty()) {
        g_tlsContext.devPath = devPath;
    }
}

void VolumeOpDiagFlushFailureReport(int32_t ret)
{
    if (!g_tlsContext.active || g_tlsContext.reported) {
        return;
    }
    SendDiagReport(BuildDiagJson(ret, g_tlsContext));
    g_tlsContext.reported = true;
}

void VolumeOpDiagScheduleAsyncFsckReport(int32_t ret, const VolumeOpDiagContext &ctx)
{
    if (!ctx.active || ctx.devPath.empty() || ctx.fsType.empty()) {
        return;
    }
    std::thread([ret, ctx]() {
        AsyncFsckReportTask(ret, ctx);
    }).detach();
}
} // namespace StorageDaemon
} // namespace OHOS
