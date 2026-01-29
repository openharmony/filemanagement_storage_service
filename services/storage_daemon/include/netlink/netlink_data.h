/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_NETLINK_DATA_H
#define OHOS_STORAGE_DAEMON_NETLINK_DATA_H

#include <map>
#include <string>

namespace OHOS {
namespace StorageDaemon {
class NetlinkData {
public:
    enum Actions {
        ADD,
        REMOVE,
        CHANGE,
        MOVE,
        ONLINE,
        OFFLINE,
        BIND,
        UNBIND,
        UNKNOWN,
    };
    std::map<std::string, Actions> actionMaps = {
        {"add", Actions::ADD},
        {"remove", Actions::REMOVE},
        {"move", Actions::MOVE},
        {"change", Actions::CHANGE},
        {"online", Actions::ONLINE},
        {"offline", Actions::OFFLINE},
        {"bind", Actions::BIND},
        {"unbind", Actions::UNBIND}
    };

    std::string GetSyspath();
    std::string GetDevpath();
    std::string GetSubsystem();
    std::string GetEjectRequest();
    Actions GetAction();
    const std::string GetParam(const std::string paramName);
    void Decode(const char *msg);

private:
    std::string sysPath_;
    std::string subSystem_;
    std::string devPath_;
    std::string ejectRequest_;
    std::vector<std::string> params_;
    Actions action_ = Actions::UNKNOWN;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_NETLINK_DATA_H
