/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

constexpr int32_t USER_ID1 = 301;
constexpr int32_t USER_ID2 = 302;
constexpr int32_t USER_ID3 = 303;
constexpr int32_t USER_ID4 = 304;
constexpr int32_t USER_ID5 = 305;
constexpr mode_t MODE = 0711;

class StorageTestUtils {
public:
    static const std::vector<Dir> gRootDirs;
    static const std::vector<Dir> gSubDirs;
    static const std::vector<Dir> gHmdfsDirs;

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