/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "utils/cmd_utils.h"
#include <cstdlib>
#include <iostream>
#include <memory>
namespace OHOS {
namespace StorageDaemon {
CmdUtils::CmdUtils() {}
CmdUtils::~CmdUtils() {}
bool CmdUtils::RunCmd(const std::string &cmd, std::vector<std::string> &result)
{
    char buffer[1024] = {0};
    result.clear();
    if (cmd.empty()) {
        return false;
    }
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        return false;
    }
    while (!feof(fp)) {
        if (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            result.push_back(buffer);
        }
    }
    pclose(fp);
    return true;
}
} // namespace StorageDaemon
} // namespace OHOS