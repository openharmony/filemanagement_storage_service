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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H
#define OHOS_STORAGE_MANAGER_STORAGE_DAEMON_COMMUNICATION_H

#include <singleton.h>
#include <nocopyable.h>
#include "system_ability.h"
#include "ipc/istorage_daemon.h"

namespace OHOS {
namespace StorageManager {
class StorageDaemonCommunication : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageDaemonCommunication);

public:
    enum {
        CRYPTO_FLAG_EL1 = 1,
        CRYPTO_FLAG_EL2,
    };
    int32_t Connect();

    int32_t PrepareAddUser(int32_t userId);
    int32_t RemoveUser(int32_t userId);
    int32_t PrepareStartUser(int32_t userId);
    int32_t StopUser(int32_t userId);

    int32_t Mount(std::string volumeId, int32_t flag);
    int32_t Unmount(std::string volumeId);
    int32_t Check(std::string volumeId);
    int32_t Partition(std::string diskId, int32_t type);
private:
    sptr<OHOS::StorageDaemon::IStorageDaemon> storageDaemon_;
};
} // StorageManager
} // OHOS

#endif