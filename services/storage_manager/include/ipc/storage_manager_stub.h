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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_MANAGER_STUB_H
#define OHOS_STORAGE_MANAGER_STORAGE_MANAGER_STUB_H

#include "iremote_stub.h"
#include "istorage_manager.h"

namespace OHOS {
namespace StorageManager {
class StorageManagerStub : public IRemoteStub<IStorageManager> {
public:
    StorageManagerStub();
    virtual ~StorageManagerStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using ManagerInterface = int32_t (StorageManagerStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, ManagerInterface> opToInterfaceMap_;
    
    int32_t HandlePrepareAddUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRemoveUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePrepareStartUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCompleteAddUser(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetTotal(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetFree(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetBundleStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetSystemSize(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetTotalSize(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetFreeSize(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetCurrUserStorageStats(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetUserStorageStats(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetUserStorageStatsByType(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetCurrentBundleStats(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyVolumeCreated(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyVolumeMounted(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyVolumeStateChanged(MessageParcel &data, MessageParcel &reply);
    int32_t HandleMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnmount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllVolumes(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyDiskCreated(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyDiskDestroyed(MessageParcel &data, MessageParcel &reply);
    int32_t HandlePartition(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetAllDisks(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetVolumeByUuid(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetVolumeById(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetVolDesc(MessageParcel &data, MessageParcel &reply);
    int32_t HandleFormat(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetDiskById(MessageParcel &data, MessageParcel &reply);

    int32_t HandleNotifyMtpMount(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyMtpUnmount(MessageParcel &data, MessageParcel &reply);

    // fscrypt api
    int32_t HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateUseAuthWithRecoveryKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleActiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleLockUserScreen(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnlockUserScreen(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetLockScreenStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGenerateAppkey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteAppkey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetFileEncryptStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetUserNeedActiveStatus(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCreateRecoverKey(MessageParcel &data, MessageParcel &reply);
    int32_t HandleSetRecoverKey(MessageParcel &data, MessageParcel &reply);

    // app file share api
    int32_t HandleCreateShareFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply);

    int32_t HandleSetBundleQuota(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUpdateMemoryPara(MessageParcel &data, MessageParcel &reply);

    int32_t HandleGetBundleStatsForIncrease(MessageParcel &data, MessageParcel &reply);

    // dfs service
    int32_t HandleMountDfsDocs(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountDfsDocs(MessageParcel &data, MessageParcel &reply);

    // mount media fuse
    int32_t HandleMountMediaFuse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountMediaFuse(MessageParcel &data, MessageParcel &reply);
    // file mgr fuse
    int32_t HandleMountFileMgrFuse(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUMountFileMgrFuse(MessageParcel &data, MessageParcel &reply);

    bool IsCalledByFileMgr();
    bool IsPathStartWithFileMgr(int32_t userId, const std::string &path);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_MANAGER_STUB_H
