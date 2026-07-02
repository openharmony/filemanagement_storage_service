/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_ERRNO_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_ERRNO_H

namespace OHOS {
namespace StorageSpaceManager {
// 与 storage_service 中其他目录的 SYS_CAP_TAG(13600000) 错开；
constexpr int32_t STORAGE_SPACE_MANAGER_SYS_CAP_TAG = 13620000;

// Error codes for storage space manager
enum {
    E_OK = 0,
    E_FAIL = -1,

    E_SERVICE_NOT_READY = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 1,
    E_INVALID_ARGUMENT = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 2,
    E_NO_MEMORY = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 3,
    E_PERMISSION_DENIED = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 4,
    E_BUNDLE_NOT_FOUND = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 5,
    E_USER_NOT_FOUND = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 6,
    E_IO_ERROR = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 7,
    E_SERVICE_IS_NULLPTR = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 8,
    E_SA_IS_NULLPTR = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 9,
    E_REMOTE_IS_NULLPTR = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 10,
    E_STATVFS_FAILED = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 11,
    E_DEATH_RECIPIENT_IS_NULLPTR = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 12,
    E_CACHEING = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 13,
    E_SERVICE_ON_IDLE = STORAGE_SPACE_MANAGER_SYS_CAP_TAG + 14,
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_ERRNO_H
