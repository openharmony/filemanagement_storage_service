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

#include <fstream>

#include "disk/disk_config.h"
#include "disk/disk_info.h"
#include "disk/disk_manager.h"
#include "ipc/storage_daemon.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "netlink/netlink_manager.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "utils/string_utils.h"

using namespace OHOS;

const int CONFIG_PARAM_NUM = 6;
static const std::string CONFIG_PTAH = "/system/etc/init/config.txt";

static bool ParasConfig(StorageDaemon::DiskManager *dm)
{
    std::ifstream infile;
    infile.open(CONFIG_PTAH);
    if (!infile) {
        LOGE("Cannot open config");
        return false;
    }

    while (infile) {
        std::string line;
        std::getline(infile, line);
        if (line.empty()) {
            LOGI("Param config complete");
            break;
        }

        std::string token = " ";
        auto split = StorageDaemon::SplitLine(line, token);
        if (split.size() != CONFIG_PARAM_NUM) {
            LOGE("Invalids config line: number of parameters is incorrect");
            continue;
        }

        auto it = split.begin();
        if (*it != "sysPattern") {
            LOGE("Invalids config line: no sysPattern");
            continue;
        }

        auto sysPattern = *(++it);
        if (*(++it) != "label") {
            LOGE("Invalids config line: no label");
            continue;
        }

        auto label = *(++it);
        if (*(++it) != "flag") {
            LOGE("Invalids config line: no flag");
            continue;
        }

        it++;
        int flag = std::atoi((*it).c_str());
        auto diskConfig =  std::make_shared<StorageDaemon::DiskConfig>(sysPattern, label, flag);
        dm->AddDiskConfig(diskConfig);
    }

    infile.close();
    return true;
}

int main()
{
    LOGI("storage_daemon start");
    StorageDaemon::NetlinkManager *nm = StorageDaemon::NetlinkManager::Instance();
    if (!nm) {
        LOGE("Unable to create NetlinkManager");
        return -1;
    };

    if (nm->Start()) {
        LOGE("Unable to start NetlinkManager");
        return -1;
    }

    StorageDaemon::DiskManager *dm = StorageDaemon::DiskManager::Instance();
    if (!dm) {
        LOGE("Unable to create DiskManger");
        return -1;
    }

    if (!ParasConfig(dm)) {
        LOGE("Paras config failed");
        return -1;
    }

    do {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr != nullptr) {
            sptr<StorageDaemon::StorageDaemon> sd = new StorageDaemon::StorageDaemon();
            samgr->AddSystemAbility(STORAGE_MANAGER_DAEMON_ID, sd);
            break;
        }
    } while (true);

    StorageDaemon::DiskManager::Instance()->ReplayUevent();
    IPCSkeleton::JoinWorkThread();

    return 0;
}
