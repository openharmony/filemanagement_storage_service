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


#include "storage/storage_quota_controller.h"

#include <fstream>
#include <string>
#include <vector>

#include "config_policy_utils.h"
#include "storage_daemon_communication/storage_daemon_communication.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageManager {
constexpr const char* QUOTA_DEVICE_DATA_PATH = "/data";
constexpr const char* PATH_CCM_CONFIG = "/etc/storage_statistic_baseline.json";
constexpr const char* CCM_CONFIG_STORAGE_STATISTIC_BASELINE = "storage.statistic.baseline";
constexpr const char* CCM_CONFIG_UID = "uid";
constexpr const char* CCM_CONFIG_BASELINE = "baseline";

StorageQuotaController &StorageQuotaController::GetInstance()
{
    static StorageQuotaController instance_;
    return instance_;
}

void StorageQuotaController::UpdateBaseLineByUid()
{
    LOGI("UpdateBaseLineByUid start");
    std::lock_guard<std::mutex> lock(quotaControllerMtx_);
    std::vector<BaselineCfg> params;
    int32_t ret = ReadCcmConfigFile(params);
    if (ret != E_OK) {
        LOGE("ReadCcmConfigFile failed");
        return;
    }
    std::shared_ptr<StorageDaemonCommunication> sdCommunication;
    sdCommunication = DelayedSingleton<StorageDaemonCommunication>::GetInstance();
    if (sdCommunication == nullptr) {
        LOGE("StorageDaemonCommunication instance is nullptr");
        return;
    }

    std::map<int32_t, int32_t> resultMap;
    for (const auto &param : params) {
        int32_t ret = sdCommunication->SetBundleQuota(param.uid, QUOTA_DEVICE_DATA_PATH,  param.baseline);
        if (ret != E_OK) {
            LOGE("SetBundleQuota failed for uid=%{public}d, baseline=%{public}d, ret=%{public}d",
                    param.uid, param.baseline, ret);
            resultMap[param.uid] = param.baseline;
        }
    }
    if (!resultMap.empty()) {
        std::string errorInfo;
        for (const auto &kv : resultMap) {
            errorInfo += "uid=" + std::to_string(kv.first) + ",baseline=" + std::to_string(kv.second) + ";";
        }
        StorageService::StorageRadar::ReportSetQuotaByBaseline("SetQuotaByBaselineBatch", errorInfo);
    }
}

int32_t StorageQuotaController::ReadCcmConfigFile(std::vector<BaselineCfg> &params)
{
    std::string path = PATH_CCM_CONFIG;
    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(path.c_str(), buf, MAX_PATH_LEN);
    if (configPath == nullptr) {
        LOGE("config path is nullptr");
        return E_PARAMS_INVALID;
    }
    char canonicalBuf[PATH_MAX] = { 0 };
    char *canonicalPath = realpath(configPath, canonicalBuf);
    if (canonicalPath == nullptr || canonicalPath[0] == '\0' || strlen(canonicalPath) >= MAX_PATH_LEN) {
        LOGE("get ccm config file path failed");
        canonicalPath = nullptr;
        return E_PARAMS_INVALID;
    }
    canonicalBuf[PATH_MAX - 1] = '\0';
    std::ifstream inFile(canonicalPath);
    if (!inFile.is_open()) {
        LOGE("open file failed, path: %{private}s, errno: %{public}d", canonicalPath, errno);
        canonicalPath = nullptr;
        return E_PARAMS_INVALID;
    }
    std::string jsonString((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    canonicalPath = nullptr;
    if (jsonString.empty()) {
        LOGE("file is empty");
        return E_PARAMS_INVALID;
    }
    cJSON* root = cJSON_Parse(jsonString.c_str());
    if (root == nullptr) {
        LOGE("parse json failed");
        return E_PARAMS_INVALID;
    }
    auto rootDeleter = [](cJSON* ptr) { if (ptr) cJSON_Delete(ptr); };
    std::unique_ptr<cJSON, decltype(rootDeleter)> rootGuard(root, rootDeleter);
    cJSON* configArray = cJSON_GetObjectItem(root, CCM_CONFIG_STORAGE_STATISTIC_BASELINE);
    if (configArray == nullptr || !cJSON_IsArray(configArray)) {
        LOGE("ccmConfig array is empty or not array");
        return E_PARAMS_INVALID;
    }

    int32_t ret = SaveUidAndBaseLine(configArray, params);
    if (ret != E_OK) {
        LOGE("save uid and baseline failed");
        return E_PARAMS_INVALID;
    }
    return E_OK;
}

int32_t StorageQuotaController::SaveUidAndBaseLine(cJSON* configArray, std::vector<BaselineCfg> &params)
{
    if (configArray == nullptr) {
        LOGE("parse json failed");
        return E_PARAMS_INVALID;
    }

    for (int i = 0; i < cJSON_GetArraySize(configArray); ++i) {
        cJSON* item = cJSON_GetArrayItem(configArray, i);
        if (item == nullptr) {
            LOGE("array item is nullptr");
            continue;
        }
        cJSON* uidItem = cJSON_GetObjectItem(item, CCM_CONFIG_UID);
        cJSON* baselineItem = cJSON_GetObjectItem(item, CCM_CONFIG_BASELINE);
        if (uidItem != nullptr && baselineItem != nullptr
            && cJSON_IsNumber(uidItem) && cJSON_IsNumber(baselineItem)) {
            if (uidItem->valueint < 0 || baselineItem->valueint < 0) {
                LOGE("uid or baseline is negative, skip");
                continue;
            }
            BaselineCfg param;
            param.uid = uidItem->valueint;
            param.baseline = baselineItem->valueint;
            LOGD("parsed uid=%{public}d, baseline=%{public}d", param.uid, param.baseline);
            params.push_back(param);
        }
    }
    return E_OK;
}
}
}  // namespace OHOS
