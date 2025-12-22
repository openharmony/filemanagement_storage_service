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

#ifndef STORAGE_DAEMON_MEMORY_RECLAIM_MANAGER_H
#define STORAGE_DAEMON_MEMORY_RECLAIM_MANAGER_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace StorageDaemon {

static constexpr uint32_t ACTIVE_USER_KEY_DELAY_SECOND = 90;
static constexpr uint32_t LOCK_USER_SCREEN_DELAY_SECOND = 15;

class MemoryReclaimManager {
public:
    MemoryReclaimManager() = delete;
    ~MemoryReclaimManager() = delete;

public:
    /**
     * @brief 延迟回收指定进程内存
     * @param pid 目标进程ID
     * @param delaySeconds 延迟秒数
     */
    static void ScheduleReclaimCurrentProcess(uint32_t delaySeconds = 15);

private:
    // 获取当前内核类型
    static bool IsHarmonyKernel();

    // 获取回收内容字符串
    static std::string GetReclaimContent();

    // 执行实际的回收操作
    static bool ExecuteReclaim(int32_t pid);

    // 写内容到指定路径
    static bool WriteToProcFile(const std::string &path, const std::string &content);
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_MEMORY_RECLAIM_MANAGER_H