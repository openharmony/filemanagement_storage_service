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

#include "base_key.h"
#include "key_manager.h"
#include "utils/log.h"

static void HandleFileCrypt(const std::string &cmd, const std::vector<std::string> &args)
{
    LOGI("cmd: %s", cmd.c_str());

    OHOS::StorageDaemon::KeyManager *manager = OHOS::StorageDaemon::KeyManager::GetInstance();
    if (manager == nullptr) {
        LOGE("manager is nullptr");
        return;
    }

    if (cmd == "init_device_key") {
        manager->InitGlobalDeviceKey();
        return;
    } else if (cmd == "init_global_userkeys") {
        manager->InitGlobalUserKeys();
        return;
    }

    unsigned int userId = 0;
    if (userId == 0) {
        LOGE("get userId error");
        return;
    }

    if (cmd == "create_user") {
        manager->CreateUserKeys(userId, true);
    } else if (cmd == "delete_user") {
        manager->DeleteUserKeys(userId);
    } else if (cmd == "update_user_auth") {
        std::string token = "";
        std::string secret = "";
        manager->UpdateUserAuth(userId, token, secret);
    } else if (cmd == "active_user_key") {
        manager->ActiveUserKey(userId);
    } else if (cmd == "inactive_user_key") {
        manager->InActiveUserKey(userId);
    } else if (cmd == "prepare_user_space") {
        manager->PrepareUserSpace(userId);
    }
}

int main(int argc, char **argv)
{
    std::cout << "storage manager start" << std::endl;

    std::vector<std::string> args(argv, argv + argc);

    if (args[1] == "filecrypt") {
        HandleFileCrypt(args[2], args); // no.2 param is the cmd
    }
}
