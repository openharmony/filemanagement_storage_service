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

#ifndef OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H
#define OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H

#include <thread>
#include "iremote_stub.h"
#include "ipc/istorage_daemon.h"
#include "utils/storage_statistics_radar.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonStub : public IRemoteStub<IStorageDaemon> {
public:
    StorageDaemonStub();
    ~StorageDaemonStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    using DaemonInterface = int32_t (StorageDaemonStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, DaemonInterface> opToInterfaceMap_;
    int32_t OnRemoteRequestForBase(uint32_t code, MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoteRequestForUser(uint32_t code, MessageParcel &data, MessageParcel &reply);
    int32_t OnRemoteRequestForApp(uint32_t code, MessageParcel &data, MessageParcel &reply);
    int32_t HandleShutdown(MessageParcel &data, MessageParcel &reply);

    int32_t HandleMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCheck(MessageParcel &data, MessageParcel &reply);
    int32_t HandleFormat(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePartition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetVolDesc(MessageParcel &data, MessageParcel &reply);

    int32_t HandleStartUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareUserDirs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDestroyUserDirs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCompleteAddUser(MessageParcel &data, MessageParcel &reply);

    // fscrypt api
    int32_t HandleInitGlobalKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInitGlobalUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateUseAuthWithRecoveryKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleActiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMountCryptoPathAgain(MessageParcel &data, MessageParcel &reply);
    int32_t HandleLockUserScreen(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnlockUserScreen(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetLockScreenStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGenerateAppkey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteAppkey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetFileEncryptStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCreateRecoverKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetRecoverKey(MessageParcel &data, MessageParcel &reply);

    // app file share api
    int32_t HandleCreateShareFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply);

    int32_t HandleSetBundleQuota(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetOccupiedSpace(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateMemoryPara(MessageParcel &data, MessageParcel &reply);

    int32_t HandleGetBundleStatsForIncrease(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMountDfsDocs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountDfsDocs(MessageParcel &data, MessageParcel &reply);

    int32_t HandleMountMediaFuse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountMediaFuse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetUserNeedActiveStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMountFileMgrFuse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountFileMgrFuse(MessageParcel &data, MessageParcel &reply);
    std::mutex mutex_;
    std::mutex mutexStats_;

    void StorageRadarThd(void);
private:
    std::atomic<bool> stopRadarReport_ { false };
    std::condition_variable execRadarReportCon_;
    std::mutex onRadarReportLock_;
    std::atomic<bool> isNeedUpdateRadarFile_ { false };
    std::thread callRadarStatisticReportThread_;
    std::map<uint32_t, RadarStatisticInfo> opStatistics_;
    std::chrono::time_point<std::chrono::system_clock> lastRadarReportTime_;
    std::map<uint32_t, RadarStatisticInfo>::iterator GetUserStatistics(const uint32_t userId);
    void GetTempStatistics(std::map<uint32_t, RadarStatisticInfo> &statistics);
};
} // StorageDaemon
} // OHOS

#endif // OHOS_STORAGE_DAEMON_STORAGE_DAEMON_STUB_H
