/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <fstream>
#include <sstream>
#include <fcntl.h>
#include "unique_fd.h"
#include "storage_service_log.h"
#include "storage_statistics_radar.h"

#include "cJSON.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *PATH_STORAGE_RADAR = "/data/service/el1/public/storage_daemon/radar/StorageStatisticFile.json";
constexpr uint32_t OP_COUNT = 12;
constexpr uint32_t KEY_LOAD_SUCC_COUNT = 0;
constexpr uint32_t KEY_LOAD_FAIL_COUNT = 1;
constexpr uint32_t KEY_UNLOAD_SUCC_COUNT = 2;
constexpr uint32_t KEY_UNLOAD_FAIL_COUNT = 3;
constexpr uint32_t USER_ADD_SUCC_COUNT = 4;
constexpr uint32_t USER_ADD_FAIL_COUNT = 5;
constexpr uint32_t USER_REMOVE_SUCC_COUNT = 6;
constexpr uint32_t USER_REMOVE_FAIL_COUNT = 7;
constexpr uint32_t USER_START_SUCC_COUNT = 8;
constexpr uint32_t USER_START_FAIL_COUNT = 9;
constexpr uint32_t USER_STOP_SUCC_COUNT = 10;
constexpr uint32_t USER_STOP_FAIL_COUNT = 11;

bool StorageStatisticRadar::CreateStatisticFile()
{
    std::string filePath = PATH_STORAGE_RADAR;
    if (access(filePath.c_str(), F_OK) == 0) {
        LOGI("File exist filePath:%{public}s", filePath.c_str());
        return true;
    }
    LOGE("Failed to access filePath :%{public}s", filePath.c_str());
    UniqueFd fd(open(filePath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR));
    if (fd < 0) {
        LOGE("Failed to creat filePath :%{public}s", filePath.c_str());
        return false;
    }
    return true;
}

void StorageStatisticRadar::CleanStatisticFile()
{
    std::string filePath = PATH_STORAGE_RADAR;
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        LOGE("Open statistic file failed");
        return;
    }
    outFile.close();
    LOGI("Clean statistic file ok, filePath :%{public}s", filePath.c_str());
}

std::string StorageStatisticRadar::GetCountInfoString(const RadarStatisticInfo &info)
{
    std::string opCountStr = std::to_string(info.keyLoadSuccCount) + ',' +
        std::to_string(info.keyLoadFailCount) + ',' +
        std::to_string(info.keyUnloadSuccCount) + ',' +
        std::to_string(info.keyUnloadFailCount) + ',' +
        std::to_string(info.userAddSuccCount) + ',' +
        std::to_string(info.userAddFailCount) + ',' +
        std::to_string(info.userRemoveSuccCount) + ',' +
        std::to_string(info.userRemoveFailCount) + ',' +
        std::to_string(info.userStartSuccCount) + ',' +
        std::to_string(info.userStartFailCount) + ',' +
        std::to_string(info.userStopSuccCount) + ',' +
        std::to_string(info.userStopFailCount);
    return opCountStr;
}

std::string StorageStatisticRadar::CreateJsonString(const std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    // create statistic json string
    cJSON *jsonStorageObject = cJSON_CreateObject();
    if (jsonStorageObject == nullptr) {
        LOGE("Creat json failed.");
        return "";
    }
    cJSON *jsonArray = cJSON_CreateArray();
    if (jsonArray == nullptr) {
        LOGE("Creat json failed.");
        cJSON_Delete(jsonStorageObject);
        return "";
    }
    cJSON_AddItemToObject(jsonStorageObject, "storageStatisticFile", jsonArray);

    for (const auto &info : statistics) {
        cJSON *newItem = cJSON_CreateObject();
        if (newItem == nullptr) {
            LOGE("Create json item failed.");
            cJSON_Delete(jsonStorageObject);
            return "";
        }
        cJSON_AddNumberToObject(newItem, "userId", info.first);
        std::string oprateCountStr = GetCountInfoString(info.second);
        cJSON_AddStringToObject(newItem, "oprateCount", oprateCountStr.c_str());
        cJSON_AddItemToArray(jsonArray, newItem);
    }
    char *jsonStr = cJSON_Print(jsonStorageObject);
    if (jsonStr == nullptr) {
        LOGE("JSON Print failed");
        cJSON_Delete(jsonStorageObject);
        return "";
    }
    std::string statisticJsonStr(jsonStr);
    cJSON_free(jsonStr);
    cJSON_Delete(jsonStorageObject);
    return statisticJsonStr;
}

bool StorageStatisticRadar::UpdateStatisticFile(const std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    if (statistics.empty()) {
        LOGI("Statistics is empty");
        return false;
    }
    std::string filePath = PATH_STORAGE_RADAR;
    std::ofstream outFile(filePath, std::ios::trunc);
    if (!outFile.is_open()) {
        LOGE("Open statistic file failed");
        return false;
    }
    std::string statisticJsonStr = CreateJsonString(statistics);
    if (statisticJsonStr.empty()) {
        LOGE("CreateJsonString failed, return empty string");
        outFile.close();
        return false;
    }
    // write StorageStatisticFile.json
    outFile << statisticJsonStr;
    outFile.close();
    LOGI("Storage update radar statistic is %{public}s.", statisticJsonStr.c_str());
    return true;
}

bool StorageStatisticRadar::ReadStatisticFile(std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    std::string filePath = PATH_STORAGE_RADAR;
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        LOGE("Open json failed");
        return false;
    }
    std::string jsonString((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    if (jsonString.length() == 0) {
        LOGI("Statistic file is empty.");
        return true;
    }
    LOGI("Storage read radar statistic is %{public}s.", jsonString.c_str());

    cJSON *jsonStorageObject = cJSON_Parse(jsonString.c_str());
    if (jsonStorageObject == nullptr) {
        LOGE("Parse json failed");
        return false;
    }

    cJSON *statisticArray = cJSON_GetObjectItem(jsonStorageObject, "storageStatisticFile");
    if (statisticArray == nullptr || !cJSON_IsArray(statisticArray)) {
        LOGE("Parse json failed");
        cJSON_Delete(jsonStorageObject);
        return false;
    }
    uint32_t userId = 0;
    std::string operateCountStr;
    for (int i = 0; i < cJSON_GetArraySize(statisticArray); ++i) {
        cJSON *item = cJSON_GetArrayItem(statisticArray, i);
        if (item == nullptr) {
            LOGE("Get json item failed");
            cJSON_Delete(jsonStorageObject);
            return false;
        }
        cJSON *userIdItem = cJSON_GetObjectItem(item, "userId");
        if (userIdItem != nullptr && cJSON_IsNumber(userIdItem)) {
            userId = static_cast<uint32_t>(userIdItem->valueint);
        }
        cJSON *countItem = cJSON_GetObjectItem(item, "oprateCount");
        if (countItem != nullptr && cJSON_IsString(countItem)) {
            operateCountStr = countItem->valuestring;
        }
        // update statistics
        ParseJsonInfo(userId, operateCountStr, statistics);
    }
    cJSON_Delete(jsonStorageObject);
    return true;
}

bool StorageStatisticRadar::ParseJsonInfo(uint32_t userId, const std::string &countInfo,
    std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    std::stringstream iss(countInfo);
    std::string count;
    std::vector<uint64_t> countVec;
    while (std::getline(iss, count, ',')) {
        countVec.push_back(std::atoll(count.c_str()));
    }
    if (countVec.size() != OP_COUNT) {
        LOGE("Statistic count num is invalid");
        return false;
    }
    RadarStatisticInfo info;
    info.keyLoadSuccCount = countVec.at(KEY_LOAD_SUCC_COUNT);
    info.keyLoadFailCount = countVec.at(KEY_LOAD_FAIL_COUNT);
    info.keyUnloadSuccCount = countVec.at(KEY_UNLOAD_SUCC_COUNT);
    info.keyUnloadFailCount = countVec.at(KEY_UNLOAD_FAIL_COUNT);
    info.userAddSuccCount = countVec.at(USER_ADD_SUCC_COUNT);
    info.userAddFailCount = countVec.at(USER_ADD_FAIL_COUNT);
    info.userRemoveSuccCount = countVec.at(USER_REMOVE_SUCC_COUNT);
    info.userRemoveFailCount = countVec.at(USER_REMOVE_FAIL_COUNT);
    info.userStartSuccCount = countVec.at(USER_START_SUCC_COUNT);
    info.userStartFailCount = countVec.at(USER_START_FAIL_COUNT);
    info.userStopSuccCount = countVec.at(USER_STOP_SUCC_COUNT);
    info.userStopFailCount = countVec.at(USER_STOP_FAIL_COUNT);
    statistics.insert(std::make_pair(userId, info));
    return true;
}
} // namespace StorageService
} // namespace OHOS
