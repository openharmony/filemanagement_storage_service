/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "storagemanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "storage_manager_stub.h"
#include "storage_manager.h"
#include "securec.h"

using namespace OHOS::StorageManager;

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return Security::AccessToken::TOKEN_NATIVE;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo& nativeTokenInfoRes)
{
    nativeTokenInfoRes.processName = "foundation";
    return 0;
}
} // namespace OHOS::Security::AccessToken

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    pid_t callingUid = 5523;
    return callingUid;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    uint32_t callingTokenID = 100;
    return callingTokenID;
}
} // namespace OHOS

namespace OHOS::StorageManager {
constexpr uint8_t MAX_CALL_TRANSACTION = 64;
constexpr size_t U32_AT_SIZE = 4;
constexpr int32_t SERVICE_ID = 5003;

std::shared_ptr<StorageManager> storageManagerPtr =
    std::make_shared<StorageManager>(SERVICE_ID, true);

uint32_t GetU32Data(const char* ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool StorageManagerFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    uint32_t code = GetU32Data(data.get());
    if (code == 0) {
        return true;
    }
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;
    storageManagerPtr->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool HandlePrepareAddUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandlePrepareAddUser(datas, reply);
    return true;
}

bool HandleRemoveUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleRemoveUser(datas, reply);
    return true;
}

bool HandlePrepareStartUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandlePrepareStartUser(datas, reply);
    return true;
}
bool HandleStopUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleStopUser(datas, reply);
    return true;
}
bool HandleGetCurrentBundleStatsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetCurrentBundleStats(datas, reply);
    return true;
}

bool HandleMountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleMount(datas, reply);
    return true;
}

bool HandleUnmountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUnmount(datas, reply);
    return true;
}

bool HandlePartitionFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandlePartition(datas, reply);
    return true;
}

bool HandleFormatFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleFormat(datas, reply);
    return true;
}

bool HandleGenerateUserKeysFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGenerateUserKeys(datas, reply);
    return true;
}

bool HandleDeleteUserKeysFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleDeleteUserKeys(datas, reply);
    return true;
}

bool HandleUpdateUserAuthFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUpdateUserAuth(datas, reply);
    return true;
}

bool HandleActiveUserKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleActiveUserKey(datas, reply);
    return true;
}

bool HandleInactiveUserKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleInactiveUserKey(datas, reply);
    return true;
}

bool HandleLockUserScreenFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleLockUserScreen(datas, reply);
    return true;
}

bool HandleUnlockUserScreenFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUnlockUserScreen(datas, reply);
    return true;
}

bool HandleUpdateKeyContextFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUpdateKeyContext(datas, reply);
    return true;
}

bool HandleCreateShareFileFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleCreateShareFile(datas, reply);
    return true;
}

bool HandleDeleteShareFileFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleDeleteShareFile(datas, reply);
    return true;
}

bool HandleSetBundleQuotaFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleSetBundleQuota(datas, reply);
    return true;
}

bool HandleUpdateMemoryParaFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUpdateMemoryPara(datas, reply);
    return true;
}

bool HandleDeleteAppkeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleDeleteAppkey(datas, reply);
    return true;
}

bool HandleCompleteAddUserFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleCompleteAddUser(datas, reply);
    return true;
}

bool HandleGetTotalFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetTotal(datas, reply);
    return true;
}

bool HandleGetFreeFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetFree(datas, reply);
    return true;
}

bool HandleGetBundleStatusFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetBundleStatus(datas, reply);
    return true;
}

bool HandleGetSystemSizeFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetSystemSize(datas, reply);
    return true;
}

bool HandleGetTotalSizeFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetTotalSize(datas, reply);
    return true;
}
bool HandleGetFreeSizeFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetFreeSize(datas, reply);
    return true;
}

bool HandleGetCurrUserStorageStatsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetCurrUserStorageStats(datas, reply);
    return true;
}

bool HandleGetUserStorageStatsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetUserStorageStats(datas, reply);
    return true;
}

bool HandleGetAllVolumesFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetAllVolumes(datas, reply);
    return true;
}

bool HandleNotifyVolumeCreatedFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyVolumeCreated(datas, reply);
    return true;
}

bool HandleNotifyVolumeMountedFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyVolumeMounted(datas, reply);
    return true;
}

bool HandleNotifyVolumeStateChangedFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyVolumeStateChanged(datas, reply);
    return true;
}

bool HandleNotifyDiskCreatedFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyDiskCreated(datas, reply);
    return true;
}

bool HandleGetAllDisksFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetAllDisks(datas, reply);
    return true;
}

bool HandleGetVolumeByUuidFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetVolumeByUuid(datas, reply);
    return true;
}

bool HandleGetVolumeByIdFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetVolumeById(datas, reply);
    return true;
}

bool HandleSetVolDescFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleSetVolDesc(datas, reply);
    return true;
}

bool HandleGetDiskByIdFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetDiskById(datas, reply);
    return true;
}

bool HandleUpdateUseAuthWithRecoveryKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUpdateUseAuthWithRecoveryKey(datas, reply);
    return true;
}

bool HandleGetFileEncryptStatusFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetFileEncryptStatus(datas, reply);
    return true;
}

bool HandleGetLockScreenStatusFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetLockScreenStatus(datas, reply);
    return true;
}

bool HandleGenerateAppkeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGenerateAppkey(datas, reply);
    return true;
}

bool HandleCreateRecoverKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleCreateRecoverKey(datas, reply);
    return true;
}

bool HandleSetRecoverKeyFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleSetRecoverKey(datas, reply);
    return true;
}

bool HandleGetBundleStatsForIncreaseFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetBundleStatsForIncrease(datas, reply);
    return true;
}

bool HandleGetUserStorageStatsByTypeFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleGetUserStorageStatsByType(datas, reply);
    return true;
}

bool HandleMountDfsDocsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleMountDfsDocs(datas, reply);
    return true;
}

bool HandleUMountDfsDocsFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUMountDfsDocs(datas, reply);
    return true;
}

bool HandleNotifyMtpMountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyMtpMount(datas, reply);
    return true;
}

bool HandleNotifyMtpUnmountFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyMtpUnmount(datas, reply);
    return true;
}

bool HandleMountMediaFuseFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleMountMediaFuse(datas, reply);
    return true;
}

bool HandleUMountMediaFuseFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleUMountMediaFuse(datas, reply);
    return true;
}

bool HandleNotifyDiskDestroyedFuzzTest(std::unique_ptr<char[]> data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;

    storageManagerPtr->HandleNotifyDiskDestroyed(datas, reply);
    return true;
}
} // namespace OHOS::StorageManager

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::StorageManager::U32_AT_SIZE) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }

    OHOS::StorageManager::StorageManagerFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePrepareAddUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleRemoveUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePrepareStartUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleStopUserFuzzTest(move(str), size);
    OHOS::StorageManager::HandleGetCurrentBundleStatsFuzzTest(move(str), size);
    OHOS::StorageManager::HandleMountFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUnmountFuzzTest(move(str), size);
    OHOS::StorageManager::HandlePartitionFuzzTest(move(str), size);
    OHOS::StorageManager::HandleFormatFuzzTest(move(str), size);
    OHOS::StorageManager::HandleGenerateUserKeysFuzzTest(move(str), size);
    OHOS::StorageManager::HandleDeleteUserKeysFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUpdateUserAuthFuzzTest(move(str), size);
    OHOS::StorageManager::HandleActiveUserKeyFuzzTest(move(str), size);
    OHOS::StorageManager::HandleInactiveUserKeyFuzzTest(move(str), size);
    OHOS::StorageManager::HandleLockUserScreenFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUnlockUserScreenFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUpdateKeyContextFuzzTest(move(str), size);
    OHOS::StorageManager::HandleCreateShareFileFuzzTest(move(str), size);
    OHOS::StorageManager::HandleDeleteShareFileFuzzTest(move(str), size);
    OHOS::StorageManager::HandleSetBundleQuotaFuzzTest(move(str), size);
    OHOS::StorageManager::HandleUpdateMemoryParaFuzzTest;
    OHOS::StorageManager::HandleDeleteAppkeyFuzzTest;
    OHOS::StorageManager::HandleCompleteAddUserFuzzTest;
    OHOS::StorageManager::HandleGetTotalFuzzTest;
    OHOS::StorageManager::HandleGetFreeFuzzTest;
    OHOS::StorageManager::HandleGetBundleStatusFuzzTest;
    OHOS::StorageManager::HandleGetSystemSizeFuzzTest;
    OHOS::StorageManager::HandleGetTotalSizeFuzzTest;
    OHOS::StorageManager::HandleGetFreeSizeFuzzTest;
    OHOS::StorageManager::HandleGetCurrUserStorageStatsFuzzTest;
    OHOS::StorageManager::HandleGetUserStorageStatsFuzzTest;
    OHOS::StorageManager::HandleGetAllVolumesFuzzTest;
    OHOS::StorageManager::HandleNotifyVolumeCreatedFuzzTest;
    OHOS::StorageManager::HandleNotifyVolumeMountedFuzzTest;
    OHOS::StorageManager::HandleNotifyVolumeStateChangedFuzzTest;
    OHOS::StorageManager::HandleNotifyDiskCreatedFuzzTest;
    OHOS::StorageManager::HandleGetAllDisksFuzzTest;
    OHOS::StorageManager::HandleGetVolumeByUuidFuzzTest;
    OHOS::StorageManager::HandleGetVolumeByIdFuzzTest;
    OHOS::StorageManager::HandleSetVolDescFuzzTest;
    OHOS::StorageManager::HandleGetDiskByIdFuzzTest;
    OHOS::StorageManager::HandleUpdateUseAuthWithRecoveryKeyFuzzTest;
    OHOS::StorageManager::HandleGetFileEncryptStatusFuzzTest;
    OHOS::StorageManager::HandleGetLockScreenStatusFuzzTest;
    OHOS::StorageManager::HandleGenerateAppkeyFuzzTest;
    OHOS::StorageManager::HandleCreateRecoverKeyFuzzTest;
    OHOS::StorageManager::HandleSetRecoverKeyFuzzTest;
    OHOS::StorageManager::HandleGetBundleStatsForIncreaseFuzzTest;
    OHOS::StorageManager::HandleGetUserStorageStatsByTypeFuzzTest;
    OHOS::StorageManager::HandleMountDfsDocsFuzzTest;
    OHOS::StorageManager::HandleUMountDfsDocsFuzzTest;
    OHOS::StorageManager::HandleNotifyMtpMountFuzzTest;
    OHOS::StorageManager::HandleNotifyMtpUnmountFuzzTest;
    OHOS::StorageManager::HandleMountMediaFuseFuzzTest;
    OHOS::StorageManager::HandleUMountMediaFuseFuzzTest;
    OHOS::StorageManager::HandleNotifyDiskDestroyedFuzzTest;
    return 0;
}
