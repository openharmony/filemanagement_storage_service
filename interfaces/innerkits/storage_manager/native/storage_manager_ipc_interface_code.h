/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_IPC_INTERFACE_TOKEN_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_IPC_INTERFACE_TOKEN_H

/* SAID:5003 */
namespace OHOS {
namespace StorageManager {
    enum class StorageManagerInterfaceCode {
        PREPARE_ADD_USER = 1,
        REMOVE_USER,
        PREPARE_START_USER,
        STOP_USER,
        GET_TOTAL,
        GET_FREE,
        GET_BUNDLE_STATUS,
        GET_SYSTEM_SIZE,
        GET_TOTAL_SIZE,
        GET_FREE_SIZE,
        GET_CURR_USER_STATS,
        GET_USER_STATS,
        GET_CURR_BUNDLE_STATS,
        NOTIFY_VOLUME_CREATED,
        NOTIFY_VOLUME_MOUNTED,
        NOTIFY_VOLUME_STATE_CHANGED,
        MOUNT,
        UNMOUNT,
        GET_ALL_VOLUMES,
        NOTIFY_DISK_CREATED,
        NOTIFY_DISK_DESTROYED,
        PARTITION,
        GET_ALL_DISKS,
        CREATE_USER_KEYS,
        DELETE_USER_KEYS,
        UPDATE_USER_AUTH,
        ACTIVE_USER_KEY,
        INACTIVE_USER_KEY,
        LOCK_USER_SCREEN,
        UNLOCK_USER_SCREEN,
        LOCK_SCREEN_STATUS,
        UPDATE_KEY_CONTEXT,
        GET_VOL_BY_UUID,
        GET_VOL_BY_ID,
        SET_VOL_DESC,
        FORMAT,
        GET_DISK_BY_ID,
        CREATE_SHARE_FILE,
        DELETE_SHARE_FILE,
        SET_BUNDLE_QUOTA,
        GET_USER_STATS_BY_TYPE,
        UPDATE_MEM_PARA,
        GET_BUNDLE_STATS_INCREASE,
        MOUNT_DFS_DOCS,
        GENERATE_APP_KEY,
        DELETE_APP_KEY,
    };
} // namespace StorageManager
} // namespace OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MANAGER_IPC_INTERFACE_TOKEN_H