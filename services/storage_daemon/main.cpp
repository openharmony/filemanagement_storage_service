/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_config.h"
#include "disk/disk_info.h"
#include "disk/disk_manager.h"
#include "netlink/netlink_manager.h"
#endif
#include "ipc/storage_daemon_provider.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "user/user_manager.h"
#include "utils/string_utils.h"
#ifdef DFS_SERVICE
#include "cloud_daemon_manager.h"
#endif
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
#include "mtp/mtp_device_monitor.h"
#endif
using namespace OHOS;
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif
using CloudListener = StorageDaemon::StorageDaemonProvider::SystemAbilityStatusChangeListener;

#ifdef EXTERNAL_STORAGE_MANAGER
const int CONFIG_PARAM_NUM = 6;
static const std::string CONFIG_PTAH = "/system/etc/storage_daemon/disk_config";

static bool ParasConfig(StorageDaemon::DiskManager *dm)
{
    if (dm == nullptr) {
        LOGE("Unable to get DiskManger");
        return false;
    }
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
#endif

static void SetPriority()
{
    int tid = syscall(SYS_gettid);
    if (setpriority(PRIO_PROCESS, tid, OHOS::StorageService::PRIORITY_LEVEL) != 0) {
        LOGE("failed to set priority");
    }
    LOGW("set main priority: %{public}d", tid);
}

static const int32_t SLEEP_TIME_INTERVAL_3MS = 3 * 1000;

int main()
{
    LOGW("storage_daemon start");
    do {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr != nullptr) {
            LOGE("samgr is not null");
            sptr<StorageDaemon::StorageDaemonProvider> sd(new StorageDaemon::StorageDaemonProvider());
            int ret = samgr->AddSystemAbility(STORAGE_MANAGER_DAEMON_ID, sd);
            LOGI("AddSystemAbility: ret: %{public}d, errno: %{public}d", ret, errno);
            sptr<CloudListener> listenter(new CloudListener());
            ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, listenter);
            LOGI("SubscribeSystemAbility for CLOUD_DAEMON_SERVICE: ret: %{public}d, errno: %{public}d", ret, errno);
            ret = samgr->SubscribeSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, listenter);
            LOGI("SubscribeSystemAbility for MANAGER_SERVICE: ret: %{public}d, errno: %{public}d", ret, errno);
            break;
        }
        usleep(SLEEP_TIME_INTERVAL_3MS);
    } while (true);
    LOGW("samgr GetSystemAbilityManager finish");

    (void)SetPriority();
#ifdef EXTERNAL_STORAGE_MANAGER
    StorageDaemon::NetlinkManager *nm = StorageDaemon::NetlinkManager::Instance();
    if ((nm == nullptr) || (nm->Start() != E_OK)) {
        LOGE("Unable to create or start NetlinkManager");
    };
    StorageDaemon::DiskManager *dm = StorageDaemon::DiskManager::Instance();
    if ((dm == nullptr) || !ParasConfig(dm)) {
        LOGE("Unable to create DiskManger or parse config failed.");
    };
#endif

#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
    DelayedSingleton<OHOS::StorageDaemon::MtpDeviceMonitor>::GetInstance()->StartMonitor();
#endif
    LOGW("storage_daemon main function execute finish.");
    IPCSkeleton::JoinWorkThread();
    return 0;
}
