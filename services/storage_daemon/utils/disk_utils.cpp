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

#include "utils/disk_utils.h"

#include <cerrno>
#include <unistd.h>
#include <unordered_map>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/sysmacros.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
static constexpr int32_t NODE_PERM = 0660;

int CreateDiskNode(const std::string &path, dev_t dev)
{
    const char *kPath = path.c_str();
    if (mknod(kPath, NODE_PERM | S_IFBLK, dev) < 0) {
        LOGE("create disk node failed");
        return E_ERR;
    }
    return E_OK;
}

int DestroyDiskNode(const std::string &path)
{
    const char *kPath = path.c_str();
    if (TEMP_FAILURE_RETRY(unlink(kPath)) < 0) {
        return E_ERR;
    }
    return E_OK;
}

int GetDevSize(std::string path, uint64_t *size)
{
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGE("open %s{private}s failed", path.c_str());
        return E_ERR;
    }

    if (ioctl(fd, BLKGETSIZE64, size)) {
        LOGE("get device %s{private}s size failed", path.c_str());
        return E_ERR;
    }

    close(fd);
    return E_OK;
}

int GetMaxVolume(dev_t device)
{
    unsigned int majorId = major(device);
    if (majorId == diskMmc) {
        std::string str;
        if (!ReadFile(SysfsMmcMaxVolumes, &str)) {
            LOGE("Get MmcMaxVolumes failed");
            return E_ERR;
        }
        return std::stoi(str);
    } else {
        return maxScsiVolumes;
    }
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
