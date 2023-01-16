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
#ifndef STORAGE_SERVICE_UTILS_ERRNO_H
#define STORAGE_SERVICE_UTILS_ERRNO_H

#include <sys/types.h>
#include <unordered_map>

namespace OHOS {
constexpr int32_t E_ERR = -1;
constexpr int32_t STORAGE_SERVICE_SYS_CAP_TAG = 13600000;

enum ErrNo {
    E_OK = 0,
    E_PERMISSION_DENIED = 1,    // no permission
    E_WRITE_DESCRIPTOR_ERR = 2, // ipc error
    E_EXIST = 3,                // exist
    E_WRONG_TYPE = 4,           // wrong file type
    E_USER_STATE = 5,           // wrong user state
    E_NON_EXIST = 6,            // no such item
    E_PREPARE_DIR = 7,          // failed to prepare dir
    E_DESTROY_DIR = 8,          // failed to destroy dir
    E_MOUNT,                    // mount error
    E_UMOUNT,                   // umount error
    E_SET_POLICY,               // set policy error
    E_USERID_RANGE,             // id out or range
    E_VOL_STATE,                // volume state check fail
    E_WAIT,                     // wait
    E_NOT_SUPPORT,              // not support
    E_SYS_CALL,                 // syscall error
    E_NO_CHILD,                 // child not exist
    E_WRITE_PARCEL_ERR,
    E_WRITE_REPLY_ERR,
    E_SA_IS_NULLPTR,
    E_REMOTE_IS_NULLPTR,
    E_SERVICE_IS_NULLPTR,
    E_DEATH_RECIPIENT_IS_NULLPTR,
    E_BUNDLEMGR_ERROR,
    E_MEDIALIBRARY_ERROR,
};

enum JsErrCode {
    E_PERMISSION = 201,
    E_PERMISSION_SYS = 202,
    E_PARAMS = 401,
    E_DEVICENOTSUPPORT = 801,
    E_OSNOTSUPPORT = 901,
    E_IPCSS = STORAGE_SERVICE_SYS_CAP_TAG + 1,
    E_SUPPORTEDFS,
    E_MOUNT_ERR,
    E_UNMOUNT,
    E_VOLUMESTATE,
    E_PREPARE,
    E_DELETE,
    E_NOOBJECT,
    E_OUTOFRANGE,
};

const std::unordered_map<int32_t, int32_t> errCodeTable {
    { E_PERMISSION_DENIED, E_PERMISSION },
    { E_WRITE_DESCRIPTOR_ERR, E_IPCSS},
    { E_EXIST, E_PREPARE},
    { E_WRONG_TYPE, E_WRONG_TYPE},
    { E_USER_STATE, E_USER_STATE},
    { E_NON_EXIST, E_NOOBJECT},
    { E_PREPARE_DIR, E_PREPARE_DIR},
    { E_DESTROY_DIR, E_DESTROY_DIR},
    { E_MOUNT, E_MOUNT_ERR},
    { E_UMOUNT, E_UNMOUNT},
    { E_SET_POLICY, E_SET_POLICY},
    { E_USERID_RANGE, E_OUTOFRANGE},
    { E_VOL_STATE, E_VOLUMESTATE},
    { E_WAIT, E_VOLUMESTATE},
    { E_NOT_SUPPORT, E_SUPPORTEDFS},
    { E_SYS_CALL, E_UNMOUNT},
    { E_NO_CHILD, E_NO_CHILD},
    { E_WRITE_PARCEL_ERR, E_IPCSS},
    { E_WRITE_REPLY_ERR, E_IPCSS},
    { E_SA_IS_NULLPTR, E_IPCSS},
    { E_REMOTE_IS_NULLPTR, E_IPCSS},
    { E_SERVICE_IS_NULLPTR, E_IPCSS},
    { E_DEATH_RECIPIENT_IS_NULLPTR, E_IPCSS},
    { E_BUNDLEMGR_ERROR, E_IPCSS},
    { E_MEDIALIBRARY_ERROR, E_IPCSS},
};
}
#endif // STORAGE_SERVICE_UTILS_ERRNO_H