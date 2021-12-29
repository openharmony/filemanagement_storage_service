/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include <string>
#include <iostream>
#include <vector>

#include "utils/log.h"
#include "storage_daemon_client.h"

static void HandleFileCrypt(const std::string &cmd, const std::vector<std::string> &args)
{
    LOGI("HandleFileCrypt::cmd: %s", cmd.c_str());
    if (cmd == "initglobalkey") {
        // sdc filecrypt initglobalkey /data
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::InitGlobalKey();
        if (ret) {
            LOGE("Init global Key failed");
            return;
        }
    } else if (cmd == "initmainuser") {
        // sdc filecrypt initglobalkey /data
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::InitGlobalUserKeys();
        if (ret) {
            LOGE("Init global user keys failed");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    LOGI("sdc start");
    std::vector<std::string> args(argv, argv + argc);

    if (args[1] == "filecrypt") {
        HandleFileCrypt(args[2], args); // no.2 param is the cmd
    }
}
