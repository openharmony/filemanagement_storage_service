/*
 * Copyright (C) 2021-2024 Huawei Device Co., Ltd.
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

constexpr int32_t ARG_CNT_2 = 2;
constexpr size_t INDEX_1 = 1;
constexpr size_t INDEX_2 = 2;

#ifdef SDC_TEST_ENABLE
constexpr int32_t ARG_CNT_3 = 3;
constexpr int32_t ARG_CNT_4 = 4;
constexpr int32_t ARG_CNT_5 = 5;
constexpr int32_t ARG_CNT_6 = 6;
constexpr size_t INDEX_3 = 3;
constexpr size_t INDEX_4 = 4;
constexpr size_t INDEX_5 = 5;
#endif

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

#ifdef SDC_TEST_ENABLE
static int32_t PrepareUserSpace(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_5) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t flags;
    // 3 means take the fourth argument of args, 4 means take the fifth argument of args
    if ((OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) ||
        (OHOS::StorageDaemon::StringToUint32(args[INDEX_4], flags) == false)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::PrepareUserDirs(userId, flags);
}

static int32_t DeleteUserKeys(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::DeleteUserKeys(userId);
}

static int32_t DestroyUserSpace(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_5) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t flags;
    // 3 means take the fourth argument of args, 4 means take the fifth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false ||
        OHOS::StorageDaemon::StringToUint32(args[INDEX_4], flags) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::DestroyUserDirs(userId, flags);
}

static int32_t UpdateUserAuth(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_5) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }

    // 4 means take the fifth argument of args, 5 means take the sixth argument of args
    if (args.size() == ARG_CNT_6) {
        std::vector<uint8_t> oldSecret(args[INDEX_4].begin(), args[INDEX_4].end());
        std::vector<uint8_t> newSecret(args[INDEX_5].begin(), args[INDEX_5].end());
        return OHOS::StorageDaemon::StorageDaemonClient::UpdateUserAuth(userId, 0, {}, oldSecret, newSecret);
    }
    std::vector<uint8_t> newSecret(args[INDEX_4].begin(), args[INDEX_4].end());
    return OHOS::StorageDaemon::StorageDaemonClient::UpdateUserAuth(userId, 0, {}, {}, newSecret);
}

static int32_t ActiveUserKey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    // 4 means take the fifth argument of args
    if (args.size() == ARG_CNT_5) {
        std::vector<uint8_t> secret(args[INDEX_4].begin(), args[INDEX_4].end());
        return OHOS::StorageDaemon::StorageDaemonClient::ActiveUserKey(userId, {}, secret);
    }
    return OHOS::StorageDaemon::StorageDaemonClient::ActiveUserKey(userId, {}, {});
}

static int32_t InactiveUserKey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::InactiveUserKey(userId);
}

static int32_t LockUserScreen(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::LockUserScreen(userId);
}

static int32_t UnlockUserScreen(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    // 4 means take the fifth argument of args
    if (args.size() == ARG_CNT_5) {
        std::vector<uint8_t> secret(args[INDEX_4].begin(), args[INDEX_4].end());
        return OHOS::StorageDaemon::StorageDaemonClient::UnlockUserScreen(userId, {}, secret);
    }
    return OHOS::StorageDaemon::StorageDaemonClient::UnlockUserScreen(userId, {}, {});
}

int32_t GetFileEncryptStatus(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_5) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    uint32_t judge;
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_4], judge) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    bool isEncrypted = true;
    bool judgeFlag = judge != 0;
    return OHOS::StorageDaemon::StorageDaemonClient::GetFileEncryptStatus(userId, isEncrypted, judgeFlag);
}

static int32_t EnableFscrypt(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    auto option = args[INDEX_3]; // cmd no.3 param is the option
    return OHOS::StorageDaemon::StorageDaemonClient::FscryptEnable(option);
}

static int32_t UpdateKeyContext(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_5) {
        LOGE("Parameter nums is less than 5, please retry");
        return -EINVAL;
    }

    uint32_t userId;
    bool needRemoveTmpKey;

    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    if (!OHOS::StorageDaemon::StringToBool(args[INDEX_4], needRemoveTmpKey)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    
    return OHOS::StorageDaemon::StorageDaemonClient::UpdateKeyContext(userId, needRemoveTmpKey);
}

static int32_t GenerateAppkey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    uint32_t hashId = 0;
    std::string keyId;
    return OHOS::StorageDaemon::StorageDaemonClient::GenerateAppkey(userId, hashId, keyId);
}

static int32_t DeleteAppkey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    // 3 means take the fourth argument of args
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    std::string keyId = args[INDEX_4];
    return OHOS::StorageDaemon::StorageDaemonClient::DeleteAppkey(userId, keyId);
}

static int32_t CreateRecoverKey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    uint32_t userType;

    if (!OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    if (!OHOS::StorageDaemon::StringToUint32(args[INDEX_4], userType)) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::CreateRecoverKey(userId, userType, {}, {});
}

static int32_t SetRecoverKey(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("SetRecoverKey Parameter nums is less than 4, please retry");
        return -EINVAL;
    }

    std::vector<uint8_t> key(args[INDEX_3].begin(), args[INDEX_3].end());
    return OHOS::StorageDaemon::StorageDaemonClient::SetRecoverKey(key);
}

static int32_t StopUser(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::StopUser(userId);
}

static int32_t StartUser(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_4) {
        LOGE("Parameter nums is less than 4, please retry");
        return -EINVAL;
    }
    uint32_t userId;
    if (OHOS::StorageDaemon::StringToUint32(args[INDEX_3], userId) == false) {
        LOGE("Parameter input error, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::StartUser(userId);
}

static int32_t EraseAllUserEncryptedKeys(const std::vector<std::string> &args)
{
    if (args.size() < ARG_CNT_3) {
        LOGE("Parameter nums is less than 3, please retry");
        return -EINVAL;
    }
    return OHOS::StorageDaemon::StorageDaemonClient::EraseAllUserEncryptedKeys();
}
#endif

static const auto g_fscryptCmdHandler = std::map<std::string,
    std::function<int32_t(const std::vector<std::string> &)>> {
    {"init_global_key", InitGlobalKey},
    {"init_main_user", InitMainUser},
#ifdef SDC_TEST_ENABLE
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
    {"generate_app_key", GenerateAppkey},
    {"delete_app_key", DeleteAppkey},
    {"Get_unlock_status", GetFileEncryptStatus},
    {"create_recover_key", CreateRecoverKey},
    {"set_recover_key", SetRecoverKey},
    {"stop_user", StopUser},
    {"start_user", StartUser},
    {"erase_all_user_encrypted_keys", EraseAllUserEncryptedKeys},
#endif
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

    if (argc < ARG_CNT_2) {
        LOGE("usage: sdc <subsystem> [cmd]");
        return 0;
    }

    int ret = 0;
    // no.1 param is the cmd
    if (args[INDEX_1] == "filecrypt") {
        ret = HandleFileCrypt(args[INDEX_2], args); // no.2 param is the cmd
    } else {
        LOGE("Unknown subsystem: %{public}s", args[INDEX_1].c_str()); // no.1 param is the cmd
        ret = -EINVAL;
    }

    LOGI("sdc end");
    std::cout << "ret: " << ret << std::endl;
    return ret;
}
