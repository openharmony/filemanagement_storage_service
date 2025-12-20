/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "fscrypt_enable.h"
#include "fscrypt_log.h"
#include <fstream>
#include <string>

constexpr const char *CRYPTED_DISABLE = "false";
constexpr const char *CRYPTED_NOT_ENABLE = "0";
constexpr const char *CRYPTED_FORCE_DISABLE = "2";

const std::string PARAM_FILE = "/proc/cmdline";
const std::string PARAM_NAME = "ohos.boot.fileEncryption=";
const size_t PARAM_SIZE = PARAM_NAME.size();
constexpr char SPLIT_CHAR = ' ';

extern "C" {
bool IsFsCryptEnableByOemInfo()
{
    std::ifstream file(PARAM_FILE);
    if (!file.is_open()) {
        LOGE("Open %{public}s failed. ", PARAM_FILE.c_str());
        return true;
    }
    std::string cmdline;
    std::getline(file, cmdline);
    file.close();

    size_t pos = cmdline.find(PARAM_NAME);
    if (pos == std::string::npos) {
        LOGE("Not config PARAM_NAME.");
        return true;
    }

    size_t start = pos + PARAM_SIZE;
    size_t end = cmdline.find(SPLIT_CHAR, start);
    if (end == std::string::npos) {
        end = cmdline.size();
    }
    std::string paramValue = cmdline.substr(start, end - start);
    if (paramValue == CRYPTED_DISABLE || paramValue == CRYPTED_NOT_ENABLE || paramValue == CRYPTED_FORCE_DISABLE) {
        LOGI("config not crypted");
        return false;
    } else {
        LOGI("config crypted");
        return true;
    }
}
}
