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

#ifndef STORAGE_FILE_CACHE_ADAPTER_H
#define STORAGE_FILE_CACHE_ADAPTER_H

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace StorageManager {

// 定义表结构对应的结构体
struct BundleExtStats {
    std::string businessName;
    uint64_t businessSize = 0;
    uint32_t userId = 0;
    std::string bundleName;
    uint64_t lastModifyTime = 0;
    bool showFlag = false;

    std::string GetKey() const
    {
        return businessName + "_" + std::to_string(userId);
    }

    bool FromJson(const nlohmann::json &j);
    nlohmann::json ToJson() const;
};

struct CleanNotify {
    std::string cleanLevelName;
    int64_t lastCleanNotifyTime = 0;

    std::string GetKey() const
    {
        return cleanLevelName;
    }

    bool FromJson(const nlohmann::json &j);
    nlohmann::json ToJson() const;
};

class FileCacheAdapter final {
public:
    static FileCacheAdapter &GetInstance();
    virtual ~FileCacheAdapter() = default;

    // 初始化和反初始化
    int32_t Init();
    int32_t UnInit();

    // BundleExtStats表操作
    int32_t InsertOrUpdateBundleExtStats(const BundleExtStats &stats);
    std::shared_ptr<BundleExtStats> GetBundleExtStats(const std::string &businessName, uint32_t userId);
    std::vector<BundleExtStats> GetAllBundleExtStats();
    // 包卸载后清除残留信息，不是通过主键查询实现
    int32_t DeleteBundleExtStats(const std::string &bundleName, uint32_t userId);

    // CleanNotify表操作
    int32_t InsertOrUpdateCleanNotify(const CleanNotify &notify);
    int32_t DeleteCleanNotify(const std::string &cleanLevelName);
    std::shared_ptr<CleanNotify> GetCleanNotify(const std::string &cleanLevelName);
    std::vector<CleanNotify> GetAllCleanNotify();

private:
    FileCacheAdapter() = default;

    // 禁止拷贝
    FileCacheAdapter(const FileCacheAdapter &) = delete;
    FileCacheAdapter &operator=(const FileCacheAdapter &) = delete;

    // JSON文件操作
    int32_t LoadBundleData();
    int32_t SaveBundleData();
    int32_t LoadCleanData();
    int32_t SaveCleanData();
    int32_t SaveJsonToFile(const std::string &filePath, const nlohmann::json &jsonData);

    // 构建key的辅助函数
    std::string BuildBundleKey(const std::string &businessName, uint32_t userId) const
    {
        return businessName + "_" + std::to_string(userId);
    }

private:
    // 初始化状态锁 - 保护初始化过程
    std::mutex initMutex_;

    // BundleExtStats数据相关
    std::shared_mutex bundleMutex_;
    std::unordered_map<std::string, BundleExtStats> bundleExtStatsMap_;
    std::string bundleJsonFilePath_;
    bool bundleDirty_ = false;

    // CleanNotify数据相关
    std::shared_mutex cleanMutex_;
    std::unordered_map<std::string, CleanNotify> cleanNotifyMap_;
    std::string cleanJsonFilePath_;
    bool cleanDirty_ = false;

    // 全局状态
    bool initialized_ = false;
};

} // namespace StorageManager
} // namespace OHOS
#endif // STORAGE_FILE_CACHE_ADAPTER_H