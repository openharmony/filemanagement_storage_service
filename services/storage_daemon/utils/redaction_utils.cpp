/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "utils/redaction_utils.h"

#include <mntent.h>
#include <unistd.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "user/mount_manager.h"
#include "utils/file_utils.h"


namespace OHOS {
namespace StorageDaemon {

constexpr size_t MNT_ENTRY_STRING_SIZE = 1024;

bool RedactionUtils::SupportedRedactionFs()
{
    return false;
}

bool RedactionUtils::CheckRedactionFsMounted(const int32_t userId)
{
    struct mntent mountEntry;
    char entryStr[MNT_ENTRY_STRING_SIZE] = {0};
    const std::string destPath = GetRedactionMountPoint(userId);
    FILE *mountTable = setmntent("/proc/mounts", "r");
    if (mountTable == nullptr) {
        LOGE("Failed to get mount table, errno:%{public}d", errno);
        return false;
    }

    do {
        struct mntent *mnt = getmntent_r(mountTable, &mountEntry, entryStr, sizeof(entryStr));
        if (mnt == nullptr) {
            endmntent(mountTable);
            break;
        }
        if ((mountEntry.mnt_type != nullptr) &&
            (mountEntry.mnt_dir != nullptr) &&
            (strcmp(mountEntry.mnt_type, "redaction") == 0) &&
            (strcmp(mountEntry.mnt_dir, destPath.c_str()) == 0)) {
            endmntent(mountTable);
            return true;
        }
    } while (true);
    return false;
}

int32_t RedactionUtils::MountRedactionFs(const int32_t userId)
{
    const std::string destPath = GetRedactionMountPoint(userId);
    if (CheckRedactionFsMounted(userId)) {
        return E_OK;
    }
    constexpr mode_t mode = 0711;
    if (!PrepareDir(destPath.c_str(), mode, OID_USER_DATA_RW, OID_USER_DATA_RW)) {
        LOGE("Failed to prepare dir: %{private}s", destPath.c_str());
        return E_MOUNT;
    }

    if ((Mount("none", destPath, "redaction", MS_NODEV, nullptr) < 0) && (errno != EEXIST) && (errno != EBUSY)) {
        if (errno == ENODEV) {
            LOGE("Redactionfs is not supported on this device");
            return E_OK;
        }
        LOGE("Failed to mount redactionfs, errno: %{public}d", errno);
        return E_MOUNT;
    }
    if (chown(destPath.c_str(), OID_USER_DATA_RW, OID_USER_DATA_RW) < 0) {
        LOGE("Failed to chown for redactionfs, path: %{private}s, errno: %{public}d", destPath.c_str(), errno);
        UMount2(destPath, MNT_DETACH);
        return E_MOUNT;
    }
    return E_OK;
}

void RedactionUtils::UMountRedactionFs(const int32_t userId)
{
    UMount2(GetRedactionMountPoint(userId), MNT_DETACH);
}

std::string RedactionUtils::GetRedactionMountPoint(const int32_t userId)
{
    return REDACTION_MOUNT_POINT_PREFIX + std::to_string(userId) + REDACTION_MOUNT_POINT_DIR;
}
} // STORAGE_DAEMON
} // OHOS
