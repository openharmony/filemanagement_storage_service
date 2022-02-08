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

#include "system_ability_definition.h"
#include "ipc/storage_daemon.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "netlink/netlink_manager.h"
#include "disk/disk_manager.h"
#include "storage_service_log.h"

using namespace OHOS;

int main()
{
    StorageDaemon::NetlinkManager *nm = StorageDaemon::NetlinkManager::Instance();
    if (!nm) {
        LOGE("Unable to create NetlinkManager");
        return -1;
    };

    if (nm->Start()) {
        LOGE("Unable to start NetlinkManager");
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
