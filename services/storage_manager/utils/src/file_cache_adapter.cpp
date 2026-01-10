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

#include "file_cache_adapter.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <system_error>
#include <unistd.h>

#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
namespace {
constexpr const char *STORAGE_MANAGER_DATA_PATH = "/data/service/el1/public/storage_manager/database/";
constexpr const char *BUNDLE_JSON_FILE_NAME = "bundle_ext_stats.json";
constexpr const char *CLEAN_JSON_FILE_NAME = "clean_notify.json";
constexpr const char *TMP_FILE_SUFFIX = ".tmp";
} // namespace

FileCacheAdapter &FileCacheAdapter::GetInstance()
{
    static FileCacheAdapter instance_;
    return instance_;
}

// BundleExtStats 序列化和反序列化实现
bool BundleExtStats::FromJson(const nlohmann::json &j)
{
    if (j.is_discarded()) {
        LOGE("BundleExtStats::FromJson: json is discarded");
        return false;
    }

    // 检查必需字段
    if (!j.contains("businessName") || !j["businessName"].is_string()) {
        LOGE("BundleExtStats::FromJson: businessName is missing or not a string");
        return false;
    }
    businessName = j["businessName"].get<std::string>();

    if (!j.contains("businessSize") || !j["businessSize"].is_number_unsigned()) {
        LOGE("BundleExtStats::FromJson: businessSize is missing or not an unsigned integer");
        return false;
    }
    businessSize = j["businessSize"].get<uint64_t>();

    if (!j.contains("userId") || !j["userId"].is_number_unsigned()) {
        LOGE("BundleExtStats::FromJson: userId is missing or not an unsigned integer");
        return false;
    }
    userId = j["userId"].get<uint32_t>();

    if (!j.contains("bundleName") || !j["bundleName"].is_string()) {
        LOGE("BundleExtStats::FromJson: bundleName is missing or not a string");
        return false;
    }
    bundleName = j["bundleName"].get<std::string>();

    if (!j.contains("lastModifyTime") || !j["lastModifyTime"].is_number_unsigned()) {
        LOGE("BundleExtStats::FromJson: lastModifyTime is missing or not an unsigned integer");
        return false;
    }
    lastModifyTime = j["lastModifyTime"].get<uint64_t>();

    if (!j.contains("showFlag") || !j["showFlag"].is_boolean()) {
        LOGE("BundleExtStats::FromJson: showFlag is missing or not a boolean");
        return false;
    }
    showFlag = j["showFlag"].get<bool>();

    return true;
}

nlohmann::json BundleExtStats::ToJson() const
{
    nlohmann::json j;
    j["businessName"] = businessName;
    j["businessSize"] = businessSize;
    j["userId"] = userId;
    j["bundleName"] = bundleName;
    j["lastModifyTime"] = lastModifyTime;
    j["showFlag"] = showFlag;
    return j;
}

// CleanNotify 序列化和反序列化实现
bool CleanNotify::FromJson(const nlohmann::json &j)
{
    if (j.is_discarded()) {
        LOGE("CleanNotify::FromJson: json is discarded");
        return false;
    }

    // 检查必需字段
    if (!j.contains("cleanLevelName") || !j["cleanLevelName"].is_string()) {
        LOGE("CleanNotify::FromJson: cleanLevelName is missing or not a string");
        return false;
    }
    cleanLevelName = j["cleanLevelName"].get<std::string>();

    if (!j.contains("lastCleanNotifyTime") || !j["lastCleanNotifyTime"].is_number_integer()) {
        LOGE("CleanNotify::FromJson: lastCleanNotifyTime is missing or not an integer");
        return false;
    }
    lastCleanNotifyTime = j["lastCleanNotifyTime"].get<int64_t>();

    return true;
}

nlohmann::json CleanNotify::ToJson() const
{
    nlohmann::json j;
    j["cleanLevelName"] = cleanLevelName;
    j["lastCleanNotifyTime"] = lastCleanNotifyTime;
    return j;
}

int32_t FileCacheAdapter::Init()
{
    std::lock_guard<std::mutex> initLock(initMutex_);
    if (initialized_) {
        return E_OK;
    }

    LOGI("JSON storage adapter lazy init start");

    // 构建JSON文件路径
    bundleJsonFilePath_ = std::string(STORAGE_MANAGER_DATA_PATH) + BUNDLE_JSON_FILE_NAME;
    cleanJsonFilePath_ = std::string(STORAGE_MANAGER_DATA_PATH) + CLEAN_JSON_FILE_NAME;

    // 分别加载两个JSON文件
    int32_t ret = LoadBundleData();
    if (ret != E_OK) {
        LOGE("Failed to load bundle data from JSON file");
        return ret;
    }

    ret = LoadCleanData();
    if (ret != E_OK) {
        LOGE("Failed to load clean data from JSON file");
        return ret;
    }

    initialized_ = true;
    LOGI("JSON storage adapter lazy init success");
    return E_OK;
}

int32_t FileCacheAdapter::UnInit()
{
    LOGI("JSON storage adapter uninit");
    std::lock_guard<std::mutex> initLock(initMutex_);

    if (!initialized_) {
        return E_OK;
    }

    std::unique_lock<std::shared_mutex> bundleLock(bundleMutex_);
    std::unique_lock<std::shared_mutex> cleanLock(cleanMutex_);

    // 保存数据到JSON文件
    if (bundleDirty_) {
        int32_t ret = SaveBundleData();
        if (ret != E_OK) {
            LOGE("Failed to save bundle data to JSON file during uninit");
        }
    }

    if (cleanDirty_) {
        int32_t ret = SaveCleanData();
        if (ret != E_OK) {
            LOGE("Failed to save clean data to JSON file during uninit");
        }
    }

    // 清空缓存
    bundleExtStatsMap_.clear();
    cleanNotifyMap_.clear();
    initialized_ = false;
    bundleDirty_ = false;
    cleanDirty_ = false;

    return E_OK;
}

// BundleExtStats表操作
int32_t FileCacheAdapter::InsertOrUpdateBundleExtStats(const BundleExtStats &stats)
{
    int32_t ret = Init();
    if (ret != E_OK) {
        return ret;
    }

    std::unique_lock<std::shared_mutex> lock(bundleMutex_);

    const std::string &key = stats.GetKey();
    auto oldNode = bundleExtStatsMap_.extract(key);

    bundleExtStatsMap_[key] = stats;
    bundleDirty_ = true;

    ret = SaveBundleData();

    // 保存失败则恢复
    if (ret != E_OK) {
        if (!oldNode.empty()) {
            bundleExtStatsMap_.insert(std::move(oldNode));
        } else {
            bundleExtStatsMap_.erase(key);
        }
    }
    bundleDirty_ = false;
    return ret;
}

int32_t FileCacheAdapter::DeleteBundleExtStats(const std::string &bundleName, uint32_t userId)
{
    LOGI("DeleteBundleExtStats by bundleName, bundle: %{public}s, userId: %{public}u", bundleName.c_str(), userId);

    int32_t ret = Init();
    if (ret != E_OK) {
        return ret;
    }

    std::unique_lock<std::shared_mutex> lock(bundleMutex_);

    int deleteCount = 0;

    // 遍历map查找并删除所有匹配的项
    for (auto it = bundleExtStatsMap_.begin(); it != bundleExtStatsMap_.end();) {
        if (it->second.bundleName == bundleName && it->second.userId == userId) {
            LOGI("Found matching record: key=%{public}s, bundle=%{public}s, userId=%{public}u", it->first.c_str(),
                 bundleName.c_str(), userId);

            it = bundleExtStatsMap_.erase(it);
            deleteCount++;
            bundleDirty_ = true;
        } else {
            ++it;
        }
    }

    if (deleteCount == 0) {
        LOGW("No BundleExtStats found for deletion: bundleName=%{public}s, userId=%{public}u", bundleName.c_str(),
             userId);
        return E_OK; // 删除不存在的记录返回成功
    }

    LOGI("Deleted %{public}d BundleExtStats records, bundleName: %{public}s, userId: %{public}u", deleteCount,
         bundleName.c_str(), userId);

    // 立即保存到文件
    return SaveBundleData();
}

std::shared_ptr<BundleExtStats> FileCacheAdapter::GetBundleExtStats(const std::string &businessName, uint32_t userId)
{
    int32_t ret = Init();
    if (ret != E_OK) {
        LOGE("Failed to initialize before GetBundleExtStats");
        return nullptr;
    }

    std::shared_lock<std::shared_mutex> lock(bundleMutex_);

    std::string key = BuildBundleKey(businessName, userId);
    auto it = bundleExtStatsMap_.find(key);
    if (it == bundleExtStatsMap_.end()) {
        return nullptr;
    }

    return std::make_shared<BundleExtStats>(it->second);
}

std::vector<BundleExtStats> FileCacheAdapter::GetAllBundleExtStats()
{
    int32_t ret = Init();
    if (ret != E_OK) {
        LOGE("Failed to initialize before GetAllBundleExtStats");
        return {};
    }

    std::shared_lock<std::shared_mutex> lock(bundleMutex_);

    std::vector<BundleExtStats> result;
    result.reserve(bundleExtStatsMap_.size());

    for (const auto &pair : bundleExtStatsMap_) {
        result.push_back(pair.second);
    }

    return result;
}

int32_t FileCacheAdapter::InsertOrUpdateCleanNotify(const CleanNotify &notify)
{
    int32_t ret = Init();
    if (ret != E_OK) {
        return ret;
    }

    std::unique_lock<std::shared_mutex> lock(cleanMutex_);

    const std::string &key = notify.GetKey();
    auto oldNode = cleanNotifyMap_.extract(key);

    cleanNotifyMap_[key] = notify;
    cleanDirty_ = true;

    ret = SaveCleanData();

    // 保存失败则恢复
    if (ret != E_OK) {
        if (!oldNode.empty()) {
            cleanNotifyMap_.insert(std::move(oldNode));
        } else {
            cleanNotifyMap_.erase(key);
        }
    }
    cleanDirty_ = false;
    return ret;
}

std::shared_ptr<CleanNotify> FileCacheAdapter::GetCleanNotify(const std::string &cleanLevelName)
{
    int32_t ret = Init();
    if (ret != E_OK) {
        LOGE("Failed to initialize before GetCleanNotify");
        return nullptr;
    }

    std::shared_lock<std::shared_mutex> lock(cleanMutex_);

    auto it = cleanNotifyMap_.find(cleanLevelName);
    if (it == cleanNotifyMap_.end()) {
        return nullptr;
    }

    return std::make_shared<CleanNotify>(it->second);
}

int32_t FileCacheAdapter::LoadBundleData()
{
    std::error_code errorCode;
    if (!std::filesystem::exists(bundleJsonFilePath_, errorCode)) {
        LOGI("Bundle JSON file not exists, will create new one: %{public}s", bundleJsonFilePath_.c_str());
        // 文件不存在是正常情况，创建空文件
        bundleExtStatsMap_.clear();
        return SaveBundleData();
    }

    // 读取文件内容
    std::ifstream file(bundleJsonFilePath_);
    if (!file.is_open()) {
        LOGE("Failed to open bundle JSON file for reading: %{public}s", bundleJsonFilePath_.c_str());
        return E_READ_RECORD_FILE_ERROR;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (jsonStr.empty()) {
        LOGI("Bundle JSON file is empty, initializing empty map");
        bundleExtStatsMap_.clear();
        return E_OK;
    }

    // 验证JSON格式
    if (!nlohmann::json::accept(jsonStr)) {
        LOGE("Bundle JSON file contains invalid JSON format: %{public}s", bundleJsonFilePath_.c_str());
        return E_PARSE_RECORD_FILE_ERROR;
    }

    // 解析JSON
    nlohmann::json jsonData = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonData.is_discarded()) {
        LOGE("Bundle JSON parse failed, json is discarded: %{public}s", bundleJsonFilePath_.c_str());
        return E_PARSE_RECORD_FILE_ERROR;
    }

    // 清空现有数据
    bundleExtStatsMap_.clear();

    // 解析BundleExtStats数据
    if (jsonData.is_array()) {
        for (const auto &item : jsonData) {
            BundleExtStats stats;
            if (stats.FromJson(item)) {
                bundleExtStatsMap_[stats.GetKey()] = stats;
            } else {
                LOGW("Failed to parse bundle stats from JSON item");
            }
        }
    }

    LOGI("Loaded %{public}zu bundle_ext_stats from JSON file", bundleExtStatsMap_.size());
    return E_OK;
}

int32_t FileCacheAdapter::LoadCleanData()
{
    std::error_code errorCode;
    if (!std::filesystem::exists(cleanJsonFilePath_, errorCode)) {
        LOGI("Clean JSON file not exists, will create new one: %{public}s", cleanJsonFilePath_.c_str());
        // 文件不存在是正常情况，创建空文件
        cleanNotifyMap_.clear();
        return SaveCleanData();
    }

    // 读取文件内容
    std::ifstream file(cleanJsonFilePath_);
    if (!file.is_open()) {
        LOGE("Failed to open clean JSON file for reading: %{public}s", cleanJsonFilePath_.c_str());
        return E_READ_RECORD_FILE_ERROR;
    }

    std::string jsonStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (jsonStr.empty()) {
        LOGI("Clean JSON file is empty, initializing empty map");
        cleanNotifyMap_.clear();
        return E_OK;
    }

    // 验证JSON格式
    if (!nlohmann::json::accept(jsonStr)) {
        LOGE("Clean JSON file contains invalid JSON format: %{public}s", cleanJsonFilePath_.c_str());
        return E_PARSE_RECORD_FILE_ERROR;
    }

    // 解析JSON
    nlohmann::json jsonData = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonData.is_discarded()) {
        LOGE("Clean JSON parse failed, json is discarded: %{public}s", cleanJsonFilePath_.c_str());
        return E_PARSE_RECORD_FILE_ERROR;
    }

    // 清空现有数据
    cleanNotifyMap_.clear();

    // 解析CleanNotify数据
    if (jsonData.is_array()) {
        for (const auto &item : jsonData) {
            CleanNotify notify;
            if (notify.FromJson(item)) {
                cleanNotifyMap_[notify.GetKey()] = notify;
            } else {
                LOGW("Failed to parse clean notify from JSON item");
            }
        }
    }

    LOGI("Loaded %{public}zu clean_notify from JSON file", cleanNotifyMap_.size());
    return E_OK;
}

int32_t FileCacheAdapter::SaveJsonToFile(const std::string &filePath, const nlohmann::json &jsonData)
{
    std::string jsonStr = jsonData.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace);
    if (jsonStr.empty()) {
        LOGE("Failed to serialize JSON data");
        return E_WRITE_RECORD_FILE_ERROR;
    }

    std::string tempFilePath = filePath + TMP_FILE_SUFFIX;

    // 写入临时文件
    std::ofstream tempFile(tempFilePath, std::ios::trunc);
    if (!tempFile.is_open()) {
        LOGE("Failed to open temp file for writing: %{public}s", tempFilePath.c_str());
        return E_WRITE_RECORD_FILE_ERROR;
    }

    tempFile << jsonStr;
    tempFile.close();

    if (tempFile.fail()) {
        LOGE("Failed to write data to temp file: %{public}s", tempFilePath.c_str());
        std::remove(tempFilePath.c_str());
        return E_WRITE_RECORD_FILE_ERROR;
    }

    // 原子性重命名
    if (rename(tempFilePath.c_str(), filePath.c_str()) != 0) {
        LOGE("Failed to rename temp file to target file, errno: %{public}d", errno);
        std::remove(tempFilePath.c_str());
        return E_WRITE_RECORD_FILE_ERROR;
    }

    return E_OK;
}

int32_t FileCacheAdapter::SaveBundleData()
{
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto &pair : bundleExtStatsMap_) {
        jsonArray.push_back(pair.second.ToJson());
    }

    int32_t result = SaveJsonToFile(bundleJsonFilePath_, jsonArray);
    if (result == E_OK) {
        bundleDirty_ = false;
        LOGI("Saved %{public}zu bundle_ext_stats to JSON file", bundleExtStatsMap_.size());
    }
    return result;
}

int32_t FileCacheAdapter::SaveCleanData()
{
    nlohmann::json jsonArray = nlohmann::json::array();
    for (const auto &pair : cleanNotifyMap_) {
        jsonArray.push_back(pair.second.ToJson());
    }

    int32_t result = SaveJsonToFile(cleanJsonFilePath_, jsonArray);
    if (result == E_OK) {
        cleanDirty_ = false;
        LOGI("Saved %{public}zu clean_notify to JSON file", cleanNotifyMap_.size());
    }
    return result;
}

} // namespace StorageManager
} // namespace OHOS