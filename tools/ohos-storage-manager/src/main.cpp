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

#include <cstring>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
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
constexpr int32_t ARGC_COUNT_THREE = 3;
constexpr int32_t ARGC_COUNT_FOUR = 4;
constexpr int32_t ARGC_COUNT_FIVE = 5;
constexpr int32_t BITS_UNIT = 1024;
constexpr int32_t SIGNIFICAND = 2;
constexpr int32_t UNITS_SIZE = 5;


std::string FormatBytes(int64_t bytes)
{
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int32_t i = 1;
    double size = static_cast<double>(bytes);
    while (size >= BITS_UNIT && i < UNITS_SIZE) {
        size /= BITS_UNIT;
        i++;
    }
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(SIGNIFICAND) << size << units[i];
    return ss.str();
}

std::string GetOption(const std::vector<std::string>& args, const std::string& option)
{
    for (size_t i = 0; i < args.size(); i++) {
        if (args[i] == option && i + 1 < args.size()) {
            return args[i + 1];
        }
    }
    return "";
}

void PrintSuccess(const std::string& data)
{
    std::cout << "{\"success\":true,\"data\":" << data << "}" << std::endl;
}

void PrintError(int32_t code, const std::string& message)
{
    std::cout << "{\"success\":false,\"error\":{\"code\":" << code
              << ",\"message\":\"" << message << "\"}}" << std::endl;
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

int32_t CmdGetTotalSize(sptr<IStorageManager> proxy)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    int64_t size = 0;
    int32_t ret = proxy->GetTotalSize(size);
    if (ret != E_OK) {
        PrintError(ret, "GetTotalSize failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"totalBytes\":" << size << ",\"totalReadable\":\"" << FormatBytes(size) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

int32_t CmdGetFreeSize(sptr<IStorageManager> proxy)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    int64_t size = 0;
    int32_t ret = proxy->GetFreeSize(size);
    if (ret != E_OK) {
        PrintError(ret, "GetFreeSize failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"freeBytes\":" << size << ",\"freeReadable\":\"" << FormatBytes(size) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

int32_t CmdGetSystemSize(sptr<IStorageManager> proxy)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    int64_t size = 0;
    int32_t ret = proxy->GetSystemSize(size);
    if (ret != E_OK) {
        PrintError(ret, "GetSystemSize failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"systemBytes\":" << size << ",\"systemReadable\":\"" << FormatBytes(size) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

int32_t CmdGetUserStorageStats(sptr<IStorageManager> proxy, int32_t userId)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    StorageStats stats;
    int32_t ret = (userId >= 0) ? proxy->GetUserStorageStats(userId, stats) : proxy->GetUserStorageStats(stats);
    if (ret != E_OK) {
        PrintError(ret, "GetUserStorageStats failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"total\":" << stats.total_
       << ",\"audio\":" << stats.audio_
       << ",\"video\":" << stats.video_
       << ",\"image\":" << stats.image_
       << ",\"file\":" << stats.file_
       << ",\"app\":" << stats.app_
       << ",\"totalReadable\":\"" << FormatBytes(stats.total_) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

int32_t CmdGetBundleStats(sptr<IStorageManager> proxy, const std::string& packageName, int32_t appIndex)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    BundleStats stats;
    int32_t ret = proxy->GetBundleStats(packageName, stats, appIndex, 0);
    if (ret != E_OK) {
        PrintError(ret, "GetBundleStats failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"packageName\":\"" << packageName << "\","
       << "\"appIndex\":" << appIndex << ","
       << "\"appSize\":" << stats.appSize_
       << ",\"cacheSize\":" << stats.cacheSize_
       << ",\"dataSize\":" << stats.dataSize_
       << ",\"appSizeReadable\":\"" << FormatBytes(stats.appSize_) << "\","
       << "\"cacheSizeReadable\":\"" << FormatBytes(stats.cacheSize_) << "\","
       << "\"dataSizeReadable\":\"" << FormatBytes(stats.dataSize_) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

int32_t CmdGetCurrentBundleStats(sptr<IStorageManager> proxy)
{
    if (!proxy) {
        PrintError(E_SA_IS_NULLPTR, "no storage manager proxy");
        return 1;
    }
    BundleStats stats;
    int32_t ret = proxy->GetCurrentBundleStats(stats, 0);
    if (ret != E_OK) {
        PrintError(ret, "GetCurrentBundleStats failed");
        return 1;
    }
    std::ostringstream ss;
    ss << "{\"appSize\":" << stats.appSize_
       << ",\"cacheSize\":" << stats.cacheSize_
       << ",\"dataSize\":" << stats.dataSize_
       << ",\"appSizeReadable\":\"" << FormatBytes(stats.appSize_) << "\","
       << "\"cacheSizeReadable\":\"" << FormatBytes(stats.cacheSize_) << "\","
       << "\"dataSizeReadable\":\"" << FormatBytes(stats.dataSize_) << "\"}";
    PrintSuccess(ss.str());
    return 0;
}

using CommandHandler = int(*)(sptr<IStorageManager>, int, char**);

int32_t HandleGetTotalSize(sptr<IStorageManager> proxy, int32_t argc, char**)
{
    return CmdGetTotalSize(proxy);
}

int32_t HandleGetFreeSize(sptr<IStorageManager> proxy, int32_t argc, char**)
{
    return CmdGetFreeSize(proxy);
}

int32_t HandleGetSystemSize(sptr<IStorageManager> proxy, int32_t argc, char**)
{
    return CmdGetSystemSize(proxy);
}

int32_t HandleGetUserStorageStats(sptr<IStorageManager> proxy, int32_t argc, char** argv)
{
    int32_t userId = -1;
    if (argc > ARGC_COUNT_THREE) {
        std::vector<std::string> args;
        for (int32_t i = 2; i < argc; i++) {
            args.push_back(argv[i]);
        }
        userId = atoi(GetOption(args, "--userId").c_str());
    }
    return CmdGetUserStorageStats(proxy, userId);
}

int32_t HandleGetBundleStats(sptr<IStorageManager> proxy, int32_t argc, char** argv)
{
    if (argc < ARGC_COUNT_FIVE) {
        PrintError(E_PARAMS_INVALID, "missing packageName argument");
        return 1;
    }
    std::vector<std::string> args;
    for (int32_t i = 2; i < argc; i++) {
        args.push_back(argv[i]);
    }
    std::string packageName = GetOption(args, "--packageName");
    if (packageName.empty()) {
        PrintError(E_PARAMS_INVALID, "missing packageName argument");
        return 1;
    }
    int32_t appIndex = (argc > ARGC_COUNT_FIVE) ? atoi(GetOption(args, "--appIndex").c_str()) : 0;
    return CmdGetBundleStats(proxy, packageName, appIndex);
}

int32_t HandleGetCurrentBundleStats(sptr<IStorageManager> proxy, int32_t argc, char**)
{
    return CmdGetCurrentBundleStats(proxy);
}

void PrintUsage()
{
    std::cerr << "Usage: ohos-storageManager <command> [options]" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Commands:" << std::endl;
    std::cerr << "  get-total-size                    Get total storage size" << std::endl;
    std::cerr << "  get-free-size                     Get free storage size" << std::endl;
    std::cerr << "  get-system-size                   Get system partition size" << std::endl;
    std::cerr << "  get-user-storage-stats [userId]   Get user storage statistics" << std::endl;
    std::cerr << "  get-bundle-stats <pkg> [index]    Get bundle storage statistics" << std::endl;
    std::cerr << "  get-current-bundle-stats          Get current bundle storage statistics" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Options:" << std::endl;
    std::cerr << "  -h, --help                        Show this help message" << std::endl;
}

} // namespace StorageManager
} // namespace OHOS

int32_t main(int32_t argc, char** argv)
{
    using namespace OHOS::StorageManager;

    if (argc < ARGC_COUNT_TWO) {
        PrintUsage();
        return 1;
    }

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "help") == 0) {
        PrintUsage();
        return 0;
    }

    static const std::unordered_map<std::string, CommandHandler> commandMap = {
        {"get-total-size", HandleGetTotalSize},
        {"get-free-size", HandleGetFreeSize},
        {"get-system-size", HandleGetSystemSize},
        {"get-user-storage-stats", HandleGetUserStorageStats},
        {"get-bundle-stats", HandleGetBundleStats},
        {"get-current-bundle-stats", HandleGetCurrentBundleStats},
    };

    auto proxy = GetStorageManagerProxy();
    const char* cmd = argv[1];

    auto it = commandMap.find(cmd);
    if (it == commandMap.end()) {
        PrintError(OHOS::E_PARAMS_INVALID, std::string("unknown command: ") + cmd);
        return 1;
    }

    return it->second(proxy, argc, argv);
}
