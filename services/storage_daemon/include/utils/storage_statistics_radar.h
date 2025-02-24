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

#ifndef STORAGE_STATISTICS_FILE_H
#define STORAGE_STATISTICS_FILE_H

#include <string>
#include <map>

namespace OHOS {
namespace StorageDaemon {
struct RadarStatisticInfo {
    uint64_t keyLoadSuccCount;
    uint64_t keyLoadFailCount;
    uint64_t keyUnloadSuccCount;
    uint64_t keyUnloadFailCount;
    uint64_t userAddSuccCount;
    uint64_t userAddFailCount;
    uint64_t userRemoveSuccCount;
    uint64_t userRemoveFailCount;
    uint64_t userStartSuccCount;
    uint64_t userStartFailCount;
    uint64_t userStopSuccCount;
    uint64_t userStopFailCount;
};

class StorageStatisticRadar {
public:
    static StorageStatisticRadar &GetInstance()
    {
        static StorageStatisticRadar instance;
        return instance;
    }

    /**
     * @brief 创建统计文件
     *
     */
    bool CreateStatisticFile();

    /**
     * @brief 清理统计文件
     *
     */
    void CleanStatisticFile();
    
    /**
     * @brief 刷新统计文件
     *
     *@param statistics 业务执行成功失败次数统计信息
     */
    bool UpdateStatisticFile(const std::map<uint32_t, RadarStatisticInfo> &statistics);

    /**
     * @brief 读取统计文件
     *
     * @param statistics 业务执行成功失败次数统计信息
     */
    bool ReadStatisticFile(std::map<uint32_t, RadarStatisticInfo> &statistics);
private:
    std::string GetCountInfoString(const RadarStatisticInfo &info);
    std::string CreateJsonString(const std::map<uint32_t, RadarStatisticInfo> &statistics);
    bool ParseJsonInfo(uint32_t userId, const std::string &countInfo, std::map<uint32_t,
        RadarStatisticInfo> &statistics);
private:
    StorageStatisticRadar() = default;
    ~StorageStatisticRadar() = default;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // STORAGE_STATISTICS_FILE_H