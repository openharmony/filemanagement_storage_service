/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_IPC_INTERFACE_CODE_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_IPC_INTERFACE_CODE_H

#include <string>
#include "iremote_broker.h"

/* SAID:5002 */
namespace OHOS {
namespace StorageDaemon {
    enum class StorageDaemonInterfaceCode {
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
        MOUNT_CRYPTO_PATH_AGAIN,
        CREATE_SHARE_FILE,
        DELETE_SHARE_FILE,
        LOCK_USER_SCREEN,
        UNLOCK_USER_SCREEN,
        LOCK_SCREEN_STATUS,
        SET_BUNDLE_QUOTA,
        GET_SPACE,
        UPDATE_MEM_PARA,
        GET_BUNDLE_STATS_INCREASE,
        MOUNT_DFS_DOCS,
        GENERATE_APP_KEY,
        DELETE_APP_KEY,
    };
} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_IPC_INTERFACE_CODE_H
