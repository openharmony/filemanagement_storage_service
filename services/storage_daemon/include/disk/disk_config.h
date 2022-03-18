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

#ifndef OHOS_STORAGE_DAEMON_DISK_CONFIG_H
#define OHOS_STORAGE_DAEMON_DISK_CONFIG_H

#include <string>

namespace OHOS {
namespace StorageDaemon {
class DiskConfig {
public:
    DiskConfig(const std::string &sysPattern, const std::string &lable, int flag);
    ~DiskConfig();
    bool IsMatch(std::string &sysPattern);
    int GetFlag() const;
private:
    std::string sysPattern_;
    std::string label_;
    int flag_ {};
};
} // namespace STORAGE_DAEMON
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_CONFIG_H
