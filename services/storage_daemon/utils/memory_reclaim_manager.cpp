/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "memory_reclaim_manager.h"
#include "parameters.h"
#include "storage_service_log.h"

#include <cstdio>
#include <chrono>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#define FDSAN_TAG_A 1

namespace OHOS {
namespace StorageDaemon {
constexpr const char *RECLAIM_FILEPAGE_STRING_FOR_HM = "1";
constexpr const char *RECLAIM_FILEPAGE_STRING_FOR_LINUX = "file";
constexpr const char *KERNEL_PARAM_KEY = "ohos.boot.kernel";
constexpr const char *KERNEL_TYPE_HM = "hongmeng";
constexpr uint64_t NEW_TAG_LOG = static_cast<uint64_t>(LOG_DOMAIN) << 32 | FDSAN_TAG_A;

void MemoryReclaimManager::ScheduleReclaimCurrentProcess(uint32_t delaySeconds)
{
    int32_t pid = getpid();
    LOGI("Schedule reclaim process (PID:%{public}d) after %{public}u seconds", pid, delaySeconds);

    std::thread([pid, delaySeconds]() {
        std::this_thread::sleep_for(std::chrono::seconds(delaySeconds));
        ExecuteReclaim(pid);
    }).detach();
}

bool MemoryReclaimManager::IsHarmonyKernel()
{
    return system::GetParameter(KERNEL_PARAM_KEY, "") == KERNEL_TYPE_HM;
}

std::string MemoryReclaimManager::GetReclaimContent()
{
    return IsHarmonyKernel() ? RECLAIM_FILEPAGE_STRING_FOR_HM : RECLAIM_FILEPAGE_STRING_FOR_LINUX;
}

bool MemoryReclaimManager::WriteToProcFile(const std::string &path, const std::string &content)
{
    int fd = open(path.c_str(), O_WRONLY);
    if (fd == -1) {
        LOGE("Failed to open %{public}s", path.c_str());
        return false;
    }

    ssize_t written = write(fd, content.c_str(), content.length());
    fdsan_exchange_owner_tag(fd, 0, NEW_TAG_LOG);
    fdsan_close_with_tag(fd, NEW_TAG_LOG);

    if (written < 0 || static_cast<size_t>(written) != content.size()) {
        LOGE("Failed to write to %{public}s", path.c_str());
        return false;
    }

    return true;
}

bool MemoryReclaimManager::ExecuteReclaim(int32_t pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/reclaim";
    std::string content = GetReclaimContent();

    LOGI("Start echo %{public}s to pid:%{public}d", content.c_str(), pid);

    return WriteToProcFile(path, content);
}

} // namespace StorageDaemon
} // namespace OHOS