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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H

#include "iremote_stub.h"
#include "ipc/istorage_daemon.h"

namespace OHOS {
namespace StorageDaemon {
constexpr int UID_ROOT = 0;
constexpr int UID_STORAGEMANAGER = 1090;
class StorageDaemonStub : public IRemoteStub<IStorageDaemon> {
public:
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    int32_t HandleShutdown();

    int32_t HandleMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCheck(MessageParcel &data, MessageParcel &reply);
    int32_t HandleFormat(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePartition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetVolDesc(MessageParcel &data, MessageParcel &reply);

    int32_t HandleStartUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareUserDirs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDestroyUserDirs(MessageParcel &data, MessageParcel &reply);

    // fscrypt api
    int32_t HandleInitGlobalKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInitGlobalUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply);
    int32_t HandleActiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply);

    // app file share api
    int32_t HandleCreateShareFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H