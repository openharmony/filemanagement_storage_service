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

#include <iostream>
#include <string>
#include <vector>

#include "storage_daemon_client.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

static void HandleFileCrypt(const std::string &cmd, const std::vector<std::string> &args)
{
    LOGI("fscrypt cmd: %{public}s", cmd.c_str());
    if (cmd == "init_global_key") {
        // sdc filecrypt init_global_key /data
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::InitGlobalKey();
        if (ret) {
            LOGE("Init global Key failed ret %{public}d", ret);
            return;
        }
    } else if (cmd == "init_main_user") {
        // sdc filecrypt init_main_user
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::InitGlobalUserKeys();
        if (ret) {
            LOGE("Init global user keys failed ret %{public}d", ret);
            return;
        }
    } else if (cmd == "generate_user_keys") {
        // sdc filecrypt generate_user_keys userId flag
        if (args.size() < 5) {
            LOGE("Parameter nums is less than 5, please retry");
            return;
        }
        uint32_t userId, flags;
        if ((OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) ||
            (OHOS::StorageDaemon::StringToUint32(args[4], flags) == false)) {
            LOGE("Parameter input error, please retry");
            return;
        }
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::GenerateUserKeys(userId, flags);
        if (ret) {
            LOGE("Create user %{public}u el failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "prepare_user_space") {
        // sdc filecrypt prepare_user_space userId flag
        if (args.size() < 5) {
            LOGE("Parameter nums is less than 5, please retry");
            return;
        }
        uint32_t userId, flags;
        if ((OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) ||
            (OHOS::StorageDaemon::StringToUint32(args[4], flags) == false)) {
            LOGE("Parameter input error, please retry");
            return;
        }
        std::string volumId = "";
        int32_t ret = OHOS::StorageDaemon::StorageDaemonClient::PrepareUserSpace(userId, volumId, flags);
        if (ret) {
            LOGE("Prepare user %{public}u storage failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "delete_user_keys") {
        // sdc filecrypt delete_user_keys userId
        if (args.size() < 4) {
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        uint32_t userId;
        if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
            LOGE("Parameter input error, please retry");
            return;
        }
        int ret = OHOS::StorageDaemon::StorageDaemonClient::DeleteUserKeys(userId);
        if (ret) {
            LOGE("Delete user %{public}u key failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "destory_user_space") {
        // sdc filecrypt destory_user_space userId flags
        if (args.size() < 5) {
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        uint32_t userId, flags;
        if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false ||
            OHOS::StorageDaemon::StringToUint32(args[4], flags) == false) {
            LOGE("Parameter input error, please retry");
            return;
        }
        std::string volumId = "";
        int ret = OHOS::StorageDaemon::StorageDaemonClient::DestroyUserSpace(userId, volumId, flags);
        if (ret) {
            LOGE("Destroy user %{public}u space failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "update_user_auth") {
        // sdc filecrypt update_user_auth userId token secret
        if (args.size() < 6) {
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        uint32_t userId;
        if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
            LOGE("Parameter input error, please retry");
            return;
        }
        std::string token = args[4];
        std::string secret = args[5];
        int ret = OHOS::StorageDaemon::StorageDaemonClient::UpdateUserAuth(userId, token, secret);
        if (ret) {
            LOGE("Update user %{public}u auth failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "active_user_key") {
        // sdc filecrypt active_user_key userId token secret
        if (args.size() < 6) {
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        uint32_t userId;
        if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
            LOGE("Parameter input error, please retry");
            return;
        }
        std::string token = args[4];
        std::string secret = args[5];
        int ret = OHOS::StorageDaemon::StorageDaemonClient::ActiveUserKey(userId, token, secret);
        if (ret) {
            LOGE("Active user %{public}u key failed ret %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "inactive_user_key") {
        // sdc filecrypt inactive_user_key userId
        if (args.size() < 4) {
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        uint32_t userId;
        if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
            LOGE("Parameter input error, please retry");
            return;
        }
        int ret = OHOS::StorageDaemon::StorageDaemonClient::InactiveUserKey(userId);
        if (ret) {
            LOGE("Inactive user %{public}u key failed %{public}d", userId, ret);
            return;
        }
    } else if (cmd == "enable") {
        if (args.size() < 4) { // para.4: sdc filecrypt enable xxx
            LOGE("Parameter nums is less than 4, please retry");
            return;
        }
        int ret = OHOS::StorageDaemon::StorageDaemonClient::FscryptEnable(args[3]); // para.3: fscrypt options
        if (ret) {
            LOGE("Fscrypt enable failed %{public}d", ret);
            return;
        }
    }
}

int main(int argc, char **argv)
{
    LOGI("sdc start");
    std::vector<std::string> args(argv, argv + argc);

    if (argc < 2) {
        LOGE("usage: sdc <subsystem> [cmd]");
        return 0;
    }

    if (args[1] == "filecrypt") {
        HandleFileCrypt(args[2], args); // no.2 param is the cmd
    }
    LOGI("sdc end");

    return 0;
}
