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
#ifndef HELP_UTILS_H
#define HELP_UTILS_H

#include <string>
#include <vector>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
namespace StorageTest {
struct Dir {
    std::string path;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};

class StorageTestUtils {
public:
    static const uid_t OID_ROOT;
    static const uid_t OID_SYSTEM;

    static const int32_t USER_ID1;
    static const int32_t USER_ID2;
    static const int32_t USER_ID3;
    static const int32_t USER_ID4;
    static const int32_t USER_ID5;
    static const mode_t MODE;

    static const std::vector<Dir> g_rootDirs;
    static const std::vector<Dir> g_subDirs;

    static const std::vector<Dir> g_hmdfsDirs;

    static const std::string HMDFS_SOURCE;
    static const std::string HMDFS_TARGET;

    static bool CheckMount(const std::string& dstPath);
    static bool CheckDir(const std::string &path);
    static bool CheckUserDir(int32_t userId, uint32_t flags);
    static bool CreateFile(const std::string &path);
    static bool RmDirRecurse(const std::string &path);
    static void RmDir(const int32_t userId);
    static bool MkDir(const std::string &path, mode_t mode);
    static void ClearTestResource();
};
} // StorageTest
} // STORAGE_DAEMON
} // OHOS
#endif // HELP_UTILS_H