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

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "client/storage_manager_client.h"
#include "storage_daemon_client.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"
constexpr const uint32_t ARGS_NUMS_SIX = 6;
constexpr const uint32_t ARGS_NUMS_FIVE = 5;
constexpr const uint32_t ARGS_NUMS_FOUR = 4;
static int32_t InitGlobalKey(const std::vector<std::string> &args)
{
    (void)args;
    return OHOS::StorageDaemon::StorageDaemonClient::InitGlobalKey();
}

static int32_t InitMainUser(const std::vector<std::string> &args)
{
    (void)args;
    return OHOS::StorageDaemon::StorageDaemonClient::InitGlobalUserKeys();
}

static int32_t GenerateUserKeys(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FIVE) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t flags;
    // 3 means take the fourth argument of args, 4 means take the fifth argument of args
    if ((OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) ||
        (OHOS::StorageDaemon::StringToUint32(args[4], flags) == false)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::GenerateUserKeys(userId, flags);
}

static int32_t PrepareUserSpace(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FIVE) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t flags;
    // 3 means take the fourth argument of args, 4 means take the fifth argument of args
    if ((OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) ||
        (OHOS::StorageDaemon::StringToUint32(args[4], flags) == false)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::PrepareUserDirs(userId, flags);
}

static int32_t DeleteUserKeys(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::DeleteUserKeys(userId);
}

static int32_t DestroyUserSpace(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FIVE) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t flags;
    // 3 means take the fourth argument of args, 4 means take the fifth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false ||
        OHOS::StorageDaemon::StringToUint32(args[4], flags) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::DestroyUserDirs(userId, flags);
}

static int32_t UpdateUserAuth(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FIVE) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }

    // 4 means take the fifth argument of args, 5 means take the sixth argument of args
    if (args.size() == ARGS_NUMS_SIX) {
        std::vector<uint8_t> oldSecret(args[4].begin(), args[4].end());
        std::vector<uint8_t> newSecret(args[5].begin(), args[5].end());
        return OHOS::StorageDaemon::StorageDaemonClient::UpdateUserAuth(userId, 0, {}, oldSecret, newSecret);
    }
    std::vector<uint8_t> newSecret(args[4].begin(), args[4].end());
    return OHOS::StorageDaemon::StorageDaemonClient::UpdateUserAuth(userId, 0, {}, {}, newSecret);
}

static int32_t ActiveUserKey(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    // 4 means take the fifth argument of args
    if (args.size() == ARGS_NUMS_FIVE) {
        std::vector<uint8_t> secret(args[4].begin(), args[4].end());
        return OHOS::StorageDaemon::StorageDaemonClient::ActiveUserKey(userId, {}, secret);
    }
    return OHOS::StorageDaemon::StorageDaemonClient::ActiveUserKey(userId, {}, {});
}

static int32_t InactiveUserKey(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::InactiveUserKey(userId);
}

static int32_t LockUserScreen(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::LockUserScreen(userId);
}

static int32_t UnlockUserScreen(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::UnlockUserScreen(userId);
}

static int32_t EnableFscrypt(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    auto option = args[3]; // cmd no.3 param is the option
    return OHOS::StorageDaemon::StorageDaemonClient::FscryptEnable(option);
}

static int32_t UpdateKeyContext(const std::vector<std::string> &args)
{
    if (args.size() < ARGS_NUMS_FOUR) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::UpdateKeyContext(userId);
}

static const auto g_fscryptCmdHandler = std::map<std::string,
    std::function<int32_t(const std::vector<std::string> &)>> {
    {"init_global_key", InitGlobalKey},
    {"init_main_user", InitMainUser},
    {"generate_user_keys", GenerateUserKeys},
    {"prepare_user_space", PrepareUserSpace},
    {"delete_user_keys", DeleteUserKeys},
    {"destroy_user_space", DestroyUserSpace},
    {"update_user_auth", UpdateUserAuth},
    {"active_user_key", ActiveUserKey},
    {"inactive_user_key", InactiveUserKey},
    {"enable", EnableFscrypt},
    {"update_key_context", UpdateKeyContext},
    {"lock_user_screen", LockUserScreen},
    {"unlock_user_screen", UnlockUserScreen},
};

static int HandleFileCrypt(const std::string &cmd, const std::vector<std::string> &args)
{
    LOGI("fscrypt cmd: %{public}s", cmd.c_str());

    auto handler = g_fscryptCmdHandler.find(cmd);
    if (handler == g_fscryptCmdHandler.end()) {
        LOGE("Unknown fscrypt cmd: %{public}s", cmd.c_str());
        return -EINVAL;
    }
    auto ret = handler->second(args);
    if (ret != 0) {
        LOGE("fscrypt cmd: %{public}s failed, ret: %{public}d", cmd.c_str(), ret);
    } else {
        LOGI("fscrypt cmd: %{public}s success", cmd.c_str());
    }
    return ret;
}

int main(int argc, char **argv)
{
    LOGI("sdc start");
    std::vector<std::string> args(argv, argv + argc);

    if (argc < 2) {
        LOGE("usage: sdc <subsystem> [cmd]");
        return 0;
    }

    int ret = 0;
    // no.1 param is the cmd
    if (args[1] == "filecrypt") {
        ret = HandleFileCrypt(args[2], args); // no.2 param is the cmd
    } else {
        LOGE("Unknown subsystem: %{public}s", args[1].c_str()); // no.1 param is the cmd
        ret = -EINVAL;
    }

    LOGI("sdc end");
    std::cout << "ret: " << ret << std::endl;
    return ret;
}
