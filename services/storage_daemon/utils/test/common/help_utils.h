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
#include <sys/types.h>
#include <vector>

namespace OHOS {
namespace StorageDaemon {
namespace StorageTest {
struct Dir {
    std::string path;
    mode_t mode;
    uid_t uid;
    gid_t gid;
};

extern const int32_t USER_ID1;
extern const int32_t USER_ID2;
extern const int32_t USER_ID3;
extern const int32_t USER_ID4;
extern const int32_t USER_ID5;
extern std::vector<Dir> g_rootDirs;
extern std::vector<Dir> g_subDirs;
extern std::vector<Dir> g_hmdfsDirs;
extern const std::string HMDFS_SOURCE;
extern const std::string HMDFS_TARGET;

bool CheckMount(const std::string& dstPath);
bool CheckDir(const std::string &path);
bool CheckUserDir(int32_t userId, uint32_t flags);
bool CreateFile(const std::string &path);
bool RmDirRecurse(const std::string &path);
void RmDir(const int32_t userId);
bool MkDir(const std::string &path, mode_t mode);
void ClearTestResource();
} // StorageTest
} // STORAGE_DAEMON
} // OHOS
#endif // HELP_UTILS_H