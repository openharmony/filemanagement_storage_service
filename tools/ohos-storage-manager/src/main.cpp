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

// LCOV_EXCL_START
#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <vector>

#include "storage_service_errno.h"
#include "storage_stats.h"
#include "bundle_stats.h"
#include "istorage_manager.h"
#include "storage_manager_proxy.h"
#include "storage_manager_connect.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace StorageManager {

constexpr int32_t ARGC_COUNT_TWO = 2;
constexpr int32_t BITS_UNIT = 1000;
constexpr int32_t SIGNIFICAND = 2;
constexpr int32_t UNITS_SIZE = 5;


static const char* TOOL_NAME = "ohos-storage-manager";
static const char* TOOL_DESC = "Storage management CLI tool for querying storage space and bundle statistics";

int OutputSuccess(const nlohmann::json& data)
{
    nlohmann::json root;
    root["type"] = "result";
    root["status"] = "success";
    root["data"] = data;

    std::cout << root.dump() << std::endl;
    return 0;
}

int OutputError(const std::string& code, const std::string& message, const std::string& suggestion = "")
{
    nlohmann::json root;
    root["type"] = "result";
    root["status"] = "failed";
    root["data"] = "";
    root["errCode"] = code;
    root["errMsg"] = message;
    root["suggestion"] = suggestion;

    std::cout << root.dump() << std::endl;
    return 1;
}

std::string FormatBytes(int64_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int32_t i = 0;
    double size = static_cast<double>(bytes);
    while (size >= BITS_UNIT && i < UNITS_SIZE) {
        size /= BITS_UNIT;
        i++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(SIGNIFICAND) << size << units[i];
    return ss.str();
}

sptr<IStorageManager> GetStorageManagerProxy()
{
    auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sam) {
        return nullptr;
    }
    auto obj = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
    return obj ? iface_cast<IStorageManager>(obj) : nullptr;
}

// JSON Output Helper Function
void PrintHelp(const std::string& subcommand = "")
{
    if (subcommand.empty()) {
        std::cout << TOOL_DESC << std::endl;
        std::cout << "Usage: " << TOOL_NAME << " <command> [options]" << std::endl;
        std::cout << "       " << TOOL_NAME << " <command> --help  Show help for a specific command" << std::endl;
        std::cout << std::endl;
        std::cout << "SubCommands:" << std::endl;
        std::cout << "  get-total-size                 Get total storage size" << std::endl;
        std::cout << "  get-free-size                  Get free storage size" << std::endl;
        std::cout << "  get-system-size                Get system partition size" << std::endl;
        std::cout << "  get-user-storage-stats         Get user storage statistics" << std::endl;
        std::cout << "  get-bundle-stats               Get bundle storage statistics" << std::endl;
        std::cout << "  get-current-bundle-stats       Get current bundle storage statistics" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --help, -h                   Show this help message" << std::endl;
        std::cout << std::endl;
        std::cout << "Examples:" << std::endl;
        std::cout << "  # Get total storage size" << std::endl;
        std::cout << "  ohos-storage-manager get-total-size" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Get free storage size" << std::endl;
        std::cout << "  ohos-storage-manager get-free-size" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Get user storage statistics" << std::endl;
        std::cout << "  ohos-storage-manager get-user-storage-stats --userId 100" << std::endl;
        std::cout << std::endl;
        std::cout << "  # Get bundle storage statistics" << std::endl;
        std::cout << "  ohos-storage-manager get-user-storage-stats --packageName com.example.demo --appIndex 0"
            << std::endl;
    } else {
        std::cout << "SubCommands: " << subcommand << std::endl;
        std::cout << "Usage: " << TOOL_NAME << " " << subcommand << " [options]" << std::endl;
        std::cout << std::endl;
        if (subcommand == "get-user-storage-stats") {
            std::cout << "Options:" << std::endl;
            std::cout << "  --userId     Optional integer. User ID to query. Default: current user." << std::endl;
        } else if (subcommand == "get-bundle-stats") {
            std::cout << "Options:" << std::endl;
            std::cout << "  --packageName    Required string. Package name of the application." << std::endl;
            std::cout << "  --appIndex       Optional integer. App index. Default: 0." << std::endl;
        } else if (subcommand == "get-total-size" || subcommand == "get-free-size" || subcommand == "get-system-size"
            || subcommand == "get-current-bundle-stats") {
            std::cout << "This subcommand has no options." << std::endl;
        } else {
            std::cout << "This subcommand is not supported. Please enter another one." << std::endl;
        }
    }
}

// Parsing parameters
std::string GetOption(const std::vector<std::string>& args, const std::string& option)
{
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == option && i + 1 < args.size()) {
            return args[i + 1];
        }
    }
    return "";
}

bool HasOption(const std::vector<std::string>& args, const std::string& option)
{
    for (const auto& arg : args) {
        if (arg == option) {
            return true;
        }
    }
    return false;
}

int CommandGetTotalSize(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    int64_t size = 0;
    int32_t ret = proxy->GetTotalSize(size);
    if (ret != E_OK) {
        return OutputError("ERR_GET_TOTAL_SIZE", "GetTotalSize failed with error code: " + std::to_string(ret),
            "Please check system storage status and try again");
    }

    nlohmann::json data;
    data["totalBytes"] = size;
    data["totalReadable"] = FormatBytes(size);
    return OutputSuccess(data);
}

int CommandGetFreeSize(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    int64_t size = 0;
    int32_t ret = proxy->GetFreeSize(size);
    if (ret != E_OK) {
        return OutputError("ERR_GET_FREE_SIZE", "GetFreeSize failed with error code: " + std::to_string(ret),
            "Please check system storage status and try again");
    }

    nlohmann::json data;
    data["freeBytes"] = size;
    data["freeReadable"] = FormatBytes(size);
    return OutputSuccess(data);
}

int CommandGetSystemSize(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    int64_t size = 0;
    int32_t ret = proxy->GetSystemSize(size);
    if (ret != E_OK) {
        return OutputError("ERR_GET_SYSTEM_SIZE", "GetSystemSize failed with error code: " + std::to_string(ret),
            "Please check system storage status and try again");
    }

    nlohmann::json data;
    data["systemBytes"] = size;
    data["systemReadable"] = FormatBytes(size);
    return OutputSuccess(data);
}

int CommandGetUserStorageStats(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    std::string userIdStr = GetOption(args, "--userId");
    int32_t userId = -1;
    if (!userIdStr.empty()) {
        userId = std::atoi(userIdStr.c_str());
    }

    StorageStats stats;
    int32_t ret = (userId >= 0) ? proxy->GetUserStorageStats(userId, stats) : proxy->GetUserStorageStats(stats);
    if (ret != E_OK) {
        return OutputError("ERR_GET_USER_STATS", "GetUserStorageStats failed with error code: " + std::to_string(ret),
            "Please check user storage statistics and try again");
    }

    nlohmann::json data;
    data["total"] = stats.total_;
    data["audio"] = stats.audio_;
    data["video"] = stats.video_;
    data["image"] = stats.image_;
    data["file"] = stats.file_;
    data["app"] = stats.app_;
    data["totalReadable"] = FormatBytes(stats.total_);
    return OutputSuccess(data);
}

int CommandGetBundleStats(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    std::string packageName = GetOption(args, "--packageName");
    if (packageName.empty()) {
        return OutputError("E_PARAMS_INVALID", "Missing required parameter: --packageName",
            "Please provide package name. Example: --packageName com.example.app");
    }

    std::string appIndexStr = GetOption(args, "--appIndex");
    int32_t appIndex = appIndexStr.empty() ? 0 : std::atoi(appIndexStr.c_str());

    BundleStats stats;
    int32_t ret = proxy->GetBundleStats(packageName, stats, appIndex, 0);
    if (ret != E_OK) {
        return OutputError("ERR_GET_BUNDLE_STATS", "GetBundleStats failed with error code: " + std::to_string(ret),
            "Please check bundle name and try again");
    }

    nlohmann::json data;
    data["packageName"] = packageName;
    data["appIndex"] = appIndex;
    data["appSize"] = stats.appSize_;
    data["cacheSize"] = stats.cacheSize_;
    data["dataSize"] = stats.dataSize_;
    data["appSizeReadable"] = FormatBytes(stats.appSize_);
    data["cacheSizeReadable"] = FormatBytes(stats.cacheSize_);
    data["dataSizeReadable"] = FormatBytes(stats.dataSize_);
    return OutputSuccess(data);
}

int CommandGetCurrentBundleStats(const std::vector<std::string>& args)
{
    auto proxy = GetStorageManagerProxy();
    if (!proxy) {
        return OutputError("E_SERVICE_IS_NULLPTR", "Failed to get StorageManager proxy",
            "Please check if the storage manager service is running");
    }

    BundleStats stats;
    int32_t ret = proxy->GetCurrentBundleStats(stats, 0);
    if (ret != E_OK) {
        return OutputError("ERR_GET_CURR_BUNDLE_STATS", "GetCurrentBundleStats failed with error code: " +
            std::to_string(ret), "Please check current bundle storage statistics and try again");
    }

    nlohmann::json data;
    data["appSize"] = stats.appSize_;
    data["cacheSize"] = stats.cacheSize_;
    data["dataSize"] = stats.dataSize_;
    data["appSizeReadable"] = FormatBytes(stats.appSize_);
    data["cacheSizeReadable"] = FormatBytes(stats.cacheSize_);
    data["dataSizeReadable"] = FormatBytes(stats.dataSize_);
    return OutputSuccess(data);
}

typedef int (*CommandHandler)(const std::vector<std::string>&);

std::unordered_map<std::string, CommandHandler> subCommands_ = {
    {"get-total-size", CommandGetTotalSize},
    {"get-free-size", CommandGetFreeSize},
    {"get-system-size", CommandGetSystemSize},
    {"get-user-storage-stats", CommandGetUserStorageStats},
    {"get-bundle-stats", CommandGetBundleStats},
    {"get-current-bundle-stats", CommandGetCurrentBundleStats},
};

} // namespace StorageManager
} // namespace OHOS

int32_t main(int32_t argc, char* argv[])
{
    using namespace OHOS::StorageManager;

    if (argc < ARGC_COUNT_TWO) {
        PrintHelp();
        return 1;
    }

    std::vector<std::string> args;
    for (int32_t i = ARGC_COUNT_TWO; i < argc; i++) {
        args.push_back(argv[i]);
    }

    std::string command = argv[1];

    if (command == "--help" || command == "-h") {
        PrintHelp();
        return 0;
    }

    if (HasOption(args, "--help") || HasOption(args, "-h")) {
        PrintHelp(command);
        return 0;
    }

    auto it = subCommands_.find(command);
    if (it == subCommands_.end()) {
        return OutputError("ERR_UNKNOWN_COMMAND", "Unknown command: " + command,
            "Run 'ohos-storage-manager --help' for usage information");
    }

    return it->second(args);
}
// LCOV_EXCL_STOP
