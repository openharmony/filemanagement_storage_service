/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_ISTORAGE_DAEMON_H
#define OHOS_STORAGE_DAEMON_ISTORAGE_DAEMON_H

#include <string>
#include "iremote_broker.h"

namespace OHOS {
namespace StorageDaemon {
class IStorageDaemon : public IRemoteBroker {
public:
    enum {
        SHUTDOWN = 1,

        MOUNT,
        UMOUNT,
        CHECK,
        FORMAT,
        PARTITION,
        SET_VOL_DESC,

        PREPARE_USER_DIRS,
        DESTROY_USER_DIRS,
        START_USER,
        STOP_USER,

        INIT_GLOBAL_KEY,
        INIT_GLOBAL_USER_KEYS,
        CREATE_USER_KEYS,
        DELETE_USER_KEYS,
        UPDATE_USER_AUTH,
        ACTIVE_USER_KEY,
        INACTIVE_USER_KEY,
        UPDATE_KEY_CONTEXT,
        CREATE_SHARE_FILE,
        DELETE_SHARE_FILE,
    };

    enum {
        CRYPTO_FLAG_EL1 = 1,
        CRYPTO_FLAG_EL2,
    };

    virtual int32_t Shutdown() = 0;

    virtual int32_t Mount(std::string volId, uint32_t flags) = 0;
    virtual int32_t UMount(std::string volId) = 0;
    virtual int32_t Check(std::string volId) = 0;
    virtual int32_t Format(std::string volId, std::string fsType) = 0;
    virtual int32_t Partition(std::string diskId, int32_t type) = 0;
    virtual int32_t SetVolumeDescription(std::string volId, std::string description) = 0;

    virtual int32_t StartUser(int32_t userId) = 0;
    virtual int32_t StopUser(int32_t userId) = 0;
    virtual int32_t PrepareUserDirs(int32_t userId, uint32_t flags) = 0;
    virtual int32_t DestroyUserDirs(int32_t userId, uint32_t flags) = 0;

    // fscrypt api
    virtual int32_t InitGlobalKey(void) = 0;
    virtual int32_t InitGlobalUserKeys(void) = 0;
    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) = 0;
    virtual int32_t DeleteUserKeys(uint32_t userId) = 0;
    virtual int32_t UpdateUserAuth(uint32_t userId,
                                   const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) = 0;
    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) = 0;
    virtual int32_t InactiveUserKey(uint32_t userId) = 0;
    virtual int32_t UpdateKeyContext(uint32_t userId) = 0;

    // app file share api
    virtual int32_t CreateShareFile(std::string uri, uint32_t tokenId, uint32_t flag) = 0;
    virtual int32_t DeleteShareFile(uint32_t tokenId, std::vector<std::string>sharePathList) = 0;

    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.StorageDaemon");
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_ISTORAGE_DAEMON_H
