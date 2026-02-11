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

#ifndef OHOS_STORAGE_DAEMON_PROCESS_H
#define OHOS_STORAGE_DAEMON_PROCESS_H

#include <string>
#include <unordered_set>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
class Process {
public:
    Process(std::string path);

    int32_t UpdatePidAndKill(int signal);
    std::string GetPath();

private:
    std::string path_;

    std::string Readlink(std::string path);
    bool CheckSubDir(std::string line);

    bool CheckMaps(std::string pidPath);
    bool CheckSymlink(std::string path);
    bool CheckFds(std::string pidPath);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_PROCESS_H