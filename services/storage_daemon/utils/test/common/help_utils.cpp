/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "help_utils.h"

#include <cerrno>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <mntent.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include "istorage_daemon.h"
#include "user/mount_constant.h"
#include "utils/file_utils.h"
#include "storage_service_constant.h"
namespace OHOS {
namespace StorageDaemon {
namespace StorageTest {
const std::string hmdfsTarget = "/storage/media/%d/local";
constexpr uid_t OID_ROOT = 0;
constexpr uid_t OID_SYSTEM = 1000;
const std::vector<Dir> StorageTestUtils::gRootDirs = {
    {"/data/app/%s/%d", MODE_0711, OID_ROOT, OID_ROOT},
    {"/data/service/%s/%d", MODE_0711, OID_ROOT, OID_ROOT},
    {"/data/chipset/%s/%d", MODE_0711, OID_ROOT, OID_ROOT}
};

const std::vector<Dir> StorageTestUtils::gSubDirs = {
    {"/data/app/%s/%d/base", MODE_0711, OID_ROOT, OID_ROOT},
    {"/data/app/%s/%d/database", MODE_0711, OID_ROOT, OID_ROOT}
};

const std::vector<Dir> StorageTestUtils::gHmdfsDirs = {
    {"/data/service/el2/%d/hmdfs", MODE_0711, OID_SYSTEM, OID_SYSTEM},
    {"/data/service/el2/%d/hmdfs/files", MODE_0711, OID_SYSTEM, OID_SYSTEM},
    {"/data/service/el2/%d/hmdfs/data", MODE_0711, OID_SYSTEM, OID_SYSTEM},
    {"/storage/media/%d", MODE_0711, OID_ROOT, OID_ROOT},
    {"/storage/media/%d/local", MODE_0711, OID_ROOT, OID_ROOT}
};

bool StorageTestUtils::CheckMount(const std::string& dstPath)
{
    const std::string fileName = "/proc/mounts";
    FILE *mntFile;
    struct mntent *mntent = nullptr;

    mntFile = setmntent(fileName.c_str(), "r");
    if (!mntFile) {
        return false;
    }

    while ((mntent = getmntent(mntFile)) != nullptr) {
        if (dstPath.compare(mntent->mnt_dir) == 0) {
            endmntent(mntFile);
            return true;
        }
    }
    endmntent(mntFile);
    return false;
}

bool StorageTestUtils::CheckDir(const std::string &path)
{
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode) == 1;
}

bool StorageTestUtils::CheckUserDir(int32_t userId, uint32_t flags)
{
    for (const Dir &dir : gRootDirs) {
        std::string path(dir.path);
        path.replace(path.find("%d"), strlen("%d"), std::to_string(userId));

        if (flags & IStorageDaemonEnum::CRYPTO_FLAG_EL1) {
            std::string realPath(path);
            realPath.replace(realPath.find("%s"), strlen("%s"), "el1");
            if (CheckDir(realPath) == false) {
                return false;
            }
        }
        if (flags & IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
            std::string realPath(path);
            realPath.replace(realPath.find("%s"), strlen("%s"), "el2");
            if (CheckDir(realPath) == false) {
                return false;
            }
        }
    }

    for (const Dir &dir : gSubDirs) {
        if (flags & IStorageDaemonEnum::CRYPTO_FLAG_EL1) {
            std::string path(dir.path);
            path.replace(path.find("%d"), strlen("%d"), std::to_string(userId));
            path.replace(path.find("%s"), strlen("%s"), "el1");
            if (CheckDir(path) == false) {
                return false;
            }
        }

        if (flags & IStorageDaemonEnum::CRYPTO_FLAG_EL2) {
            std::string path(dir.path);
            path.replace(path.find("%d"), strlen("%d"), std::to_string(userId));
            path.replace(path.find("%s"), strlen("%s"), "el2");
            if (CheckDir(path) == false) {
                return false;
            }
        }
    }

    for (const Dir &dir : gHmdfsDirs) {
        std::string path(dir.path);
        path.replace(path.find("%d"), strlen("%d"), std::to_string(userId));
        if (CheckDir(path) == false) {
            return false;
        }
    }
    return true;
}

bool StorageTestUtils::CreateFile(const std::string &path)
{
    (void)RmDirRecurse(path);
    FILE *f = fopen(path.c_str(), "w+");
    if (f == nullptr) {
        return false;
    }
    ChMod(path.c_str(), MODE);
    int fd = fileno(f);
    if (fd == -1) {
        return false;
    }
    (void)fclose(f);
    return true;
}

bool StorageTestUtils::MkDir(const std::string &path, mode_t mode)
{
    if (access(path.c_str(), 0) == 0) {
        if (rmdir(path.c_str()) != 0) {
            return false;
        }
    }
    if (mkdir(path.c_str(), mode) != 0) {
        return false;
    }
    if (access(path.c_str(), 0) != 0) {
        return false;
    }
    return true;
}

bool StorageTestUtils::RmDirRecurse(const std::string &path)
{
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) {
        return false;
    }
    if (S_ISDIR(st.st_mode) != 1) {
        return (unlink(path.c_str()) == 0);
    }

    DIR *dir = opendir(path.c_str());
    if (!dir) {
        if (errno == ENOENT) {
            return true;
        }
        return false;
    }

    for (struct dirent *ent = readdir(dir); ent != nullptr; ent = readdir(dir)) {
        if (ent->d_type == DT_DIR) {
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                continue;
            }

            if (!RmDirRecurse(path + "/" + ent->d_name)) {
                (void)closedir(dir);
                return false;
            }
        } else {
            if (unlink((path + "/" + ent->d_name).c_str())) {
                (void)closedir(dir);
                return false;
            }
        }
    }

    (void)closedir(dir);
    if (rmdir(path.c_str())) {
        return false;
    }

    return true;
}

void StorageTestUtils::RmDir(const int32_t userId)
{
    std::vector<std::string> paths = {
        "/data/app/el1/",
        "/data/app/el2/",
        "/data/service/el1/",
        "/data/service/el2/",
        "/data/chipset/el1/",
        "/data/chipset/el2/",
        "/storage/media/"
    };

    for (auto path : paths) {
        path.append(std::to_string(userId));
        RmDirRecurse(path);
    }
}

void StorageTestUtils::ClearTestResource()
{
    const int32_t userIds[] = {
        USER_ID1,
        USER_ID2,
        USER_ID3,
        USER_ID4,
        USER_ID5
    };
    for (const auto id : userIds) {
        std::string dstPath(hmdfsTarget);
        dstPath.replace(dstPath.find("%d"), strlen("%d"), std::to_string(id));
        UMount(dstPath);
        RmDir(id);
    }
}
} // StorageTest
} // STORAGE_DAEMON
} // OHOS
