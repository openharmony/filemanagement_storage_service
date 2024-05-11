/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "anco_key_manager.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "crypto/key_manager.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
const std::string MKDIR = "mkdir";
const std::string ANCO_TYPE_NONE = "encryption=None";
const std::string ANCO_TYPE_SYS_EL1 = "encryption=Require_Sys_EL1";
const std::string ANCO_TYPE_USER_EL1 = "encryption=Require_User_EL1";
const std::string DATA_PATH = "/data/virt_service/rgm_hmos/anco_hmos_data/";
const int MIN_NUM = 4;
const int MAX_NUM = 5;
std::map<std::string, std::string> AncoKeyManager::ownerMap_;

int32_t AncoKeyManager::SetAncoDirectoryElPolicy(const std::string &path, const std::string &policyType,
                                                 unsigned int user)
{
    std::vector<FileList> vec;
    SetUserPermissionMap();
    auto ret = ReadFileAndCreateDir(path, policyType, vec);
    if (ret != E_OK) {
        LOGE("Read file and create dir failed, ret = %{public}d", ret);
        return ret;
    }

    KeyType type;
    if (policyType == ANCO_TYPE_SYS_EL1 || policyType == ANCO_TYPE_USER_EL1) {
        type = EL1_KEY;
    } else {
        type = EL2_KEY;
    }

    ret = KeyManager::GetInstance()->SetDirectoryElPolicy(user, type, vec);
    if (ret != E_OK) {
        LOGE(" Set directory el policy failed, ret = %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t AncoKeyManager::ReadFileAndCreateDir(const std::string &path, const std::string &type,
                                             std::vector<FileList> &vec)
{
    char realPath[PATH_MAX] = {0x00};
    if (realpath(path.c_str(), realPath) == nullptr) {
        LOGE("path not valid, path = %{private}s", path.c_str());
        return E_JSON_PARSE_ERROR;
    }

    std::ifstream infile(std::string(realPath), std::ios::in);
    if (!infile.is_open()) {
        LOGE("Open file failed, errno = %{public}d", errno);
        return E_OPEN_JSON_FILE_ERROR;
    }

    std::string line;
    while (getline(infile, line)) {
        if (line == "") {
            continue;
        }
        std::istringstream iss(line);
        AncoDirInfo ancoDirInfo;
        iss >> ancoDirInfo.mkdir;
        if (ancoDirInfo.mkdir != MKDIR) {
            continue;
        }
        if (!(iss >> ancoDirInfo.path >> ancoDirInfo.mode >> ancoDirInfo.uid >> ancoDirInfo.gid >>
              ancoDirInfo.policy)) {
            continue;
        }

        auto ret = CreatePolicyDir(ancoDirInfo, type, vec);
        if (ret != E_OK) {
            infile.close();
            LOGE(" Create policy dir failed, ret = %{public}d", ret);
            return ret;
        }
    }

    infile.close();
    return E_OK;
}

int32_t AncoKeyManager::CreatePolicyDir(const AncoDirInfo &ancoDirInfo,
                                        const std::string &type,
                                        std::vector<FileList> &vec)
{
    auto ret = CheckMemberValid(ancoDirInfo);
    if (ret != E_OK) {
        LOGE("Check Valid failed, ret = %{public}d", ret);
        return E_JSON_PARSE_ERROR;
    }
    auto mode = std::stoi(ancoDirInfo.mode, nullptr, 8);
    auto iter = AncoKeyManager::ownerMap_.find(ancoDirInfo.uid);
    if (iter == AncoKeyManager::ownerMap_.end()) {
        LOGE("AncoDirInfo.uid not found, uid = %{public}s", ancoDirInfo.uid.c_str());
        return E_JSON_PARSE_ERROR;
    }
    auto uid = static_cast<uid_t>(std::stoi(iter->second));
    iter = AncoKeyManager::ownerMap_.find(ancoDirInfo.gid);
    if (iter == AncoKeyManager::ownerMap_.end()) {
        LOGE("AncoDirInfo.gid not found, gid = %{public}s", ancoDirInfo.gid.c_str());
        return E_JSON_PARSE_ERROR;
    }
    auto gid = static_cast<gid_t>(std::stoi(iter->second));

    if (ancoDirInfo.policy == type) {
        std::error_code errorCode;
        if (!std::filesystem::exists(ancoDirInfo.path, errorCode)) {
            FileList fileList;
            fileList.path = ancoDirInfo.path;
            vec.push_back(fileList);
        }
        if (!PrepareDir(ancoDirInfo.path, mode, uid, gid)) {
            LOGE("Prepare dir failed");
            return E_PREPARE_DIR;
        }
    }
    if (ancoDirInfo.policy == ANCO_TYPE_NONE && type == ANCO_TYPE_SYS_EL1) {
        if (!PrepareDir(ancoDirInfo.path, mode, uid, gid)) {
            LOGE("Prepare dir failed");
            return E_PREPARE_DIR;
        }
    }
    return E_OK;
}

int32_t AncoKeyManager::CheckMemberValid(const AncoDirInfo &ancoDirInfo)
{
    if (ancoDirInfo.mode.empty() || ancoDirInfo.mode.length() < MIN_NUM || ancoDirInfo.mode.length() > MAX_NUM) {
        LOGE("Check ancoDirInfo.mode rule failed, mode = %{public}s", ancoDirInfo.mode.c_str());
        return E_JSON_PARSE_ERROR;
    }
    for (auto c : ancoDirInfo.mode) {
        if (!isdigit(c)) {
            LOGE("AncoDirInfo.mod not number type, mode = %{public}s", ancoDirInfo.mode.c_str());
            return E_JSON_PARSE_ERROR;
        }
    }

    if (ancoDirInfo.path.find(DATA_PATH) != 0) {
        LOGE("AncoDirInfo.path not valid, path = %{public}s", ancoDirInfo.path.c_str());
        return E_JSON_PARSE_ERROR;
    }

    auto size = strnlen(ancoDirInfo.path.c_str(), PATH_MAX);
    if (size == 0 || size == PATH_MAX) {
        LOGE("AncoDirInfo.path not valid, path = %{public}s", ancoDirInfo.path.c_str());
        return E_JSON_PARSE_ERROR;
    }
    char realPath[PATH_MAX] = {0x00};
    if (realpath(ancoDirInfo.path.c_str(), realPath) == nullptr) {
        LOGE("AncoDirInfo.path not valid, path = %{public}s", ancoDirInfo.path.c_str());
        return E_JSON_PARSE_ERROR;
    }
    return E_OK;
}

void AncoKeyManager::SetUserPermissionMap()
{
    std::lock_guard<std::mutex> lock(Mutex_);
    AncoKeyManager::ownerMap_["root"] = "0";
    AncoKeyManager::ownerMap_["system"] = "1000";
    AncoKeyManager::ownerMap_["media_rw"] = "1023";
    AncoKeyManager::ownerMap_["media"] = "1013";
    AncoKeyManager::ownerMap_["misc"] = "9998";
    AncoKeyManager::ownerMap_["shell"] = "2000";
    AncoKeyManager::ownerMap_["cache"] = "2001";
    AncoKeyManager::ownerMap_["log"] = "1007";
    AncoKeyManager::ownerMap_["file_manager"] = "1006";
    AncoKeyManager::ownerMap_["drm"] = "1019";
    AncoKeyManager::ownerMap_["1003"] = "1003";
    AncoKeyManager::ownerMap_["nfc"] = "1027";
    AncoKeyManager::ownerMap_["5506"] = "5506";
    AncoKeyManager::ownerMap_["1031"] = "1031";
}
} // namespace StorageDaemon
} // namespace OHOS