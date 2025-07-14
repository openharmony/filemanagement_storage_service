/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

sptr<StorageManager> storageManagerPtr (new (std::nothrow) StorageManager(SERVICE_ID, true));

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
    if (storageManagerPtr != nullptr) {
        storageManagerPtr->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);
    }

    return true;
}

bool HandlePrepareAddUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandlePrepareAddUser(datas, reply);
    }
    return true;
}

bool HandleRemoveUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleRemoveUser(datas, reply);
    }
    return true;
}

bool HandlePrepareStartUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandlePrepareStartUser(datas, reply);
    }
    return true;
}
bool HandleStopUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleStopUser(datas, reply);
    }
    return true;
}
bool HandleGetCurrentBundleStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetCurrentBundleStats(datas, reply);
    }
    return true;
}

bool HandleMountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleMount(datas, reply);
    }
    return true;
}

bool HandleUnmountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUnmount(datas, reply);
    }
    return true;
}

bool HandlePartitionFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandlePartition(datas, reply);
    }
    return true;
}

bool HandleFormatFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleFormat(datas, reply);
    }
    return true;
}

bool HandleGenerateUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGenerateUserKeys(datas, reply);
    }
    return true;
}

bool HandleDeleteUserKeysFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleDeleteUserKeys(datas, reply);
    }
    return true;
}

bool HandleUpdateUserAuthFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUpdateUserAuth(datas, reply);
    }
    return true;
}

bool HandleActiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleActiveUserKey(datas, reply);
    }
    return true;
}

bool HandleInactiveUserKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleInactiveUserKey(datas, reply);
    }
    return true;
}

bool HandleLockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleLockUserScreen(datas, reply);
    }
    return true;
}

bool HandleUnlockUserScreenFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUnlockUserScreen(datas, reply);
    }
    return true;
}

bool HandleUpdateKeyContextFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUpdateKeyContext(datas, reply);
    }
    return true;
}

bool HandleCreateShareFileFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleCreateShareFile(datas, reply);
    }
    return true;
}

bool HandleDeleteShareFileFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleDeleteShareFile(datas, reply);
    }
    return true;
}

bool HandleSetBundleQuotaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleSetBundleQuota(datas, reply);
    }
    return true;
}

bool HandleUpdateMemoryParaFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUpdateMemoryPara(datas, reply);
    }
    return true;
}

bool HandleDeleteAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleDeleteAppkey(datas, reply);
    }
    return true;
}

bool HandleCompleteAddUserFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleCompleteAddUser(datas, reply);
    }
    return true;
}

bool HandleGetTotalFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetTotal(datas, reply);
    }
    return true;
}

bool HandleGetFreeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetFree(datas, reply);
    }
    return true;
}

bool HandleGetBundleStatusFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetBundleStatus(datas, reply);
    }
    return true;
}

bool HandleGetSystemSizeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetSystemSize(datas, reply);
    }
    return true;
}

bool HandleGetTotalSizeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetTotalSize(datas, reply);
    }
    return true;
}
bool HandleGetFreeSizeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetFreeSize(datas, reply);
    }
    return true;
}

bool HandleGetCurrUserStorageStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetCurrUserStorageStats(datas, reply);
    }
    return true;
}

bool HandleGetUserStorageStatsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetUserStorageStats(datas, reply);
    }
    return true;
}

bool HandleGetAllVolumesFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetAllVolumes(datas, reply);
    }
    return true;
}

bool HandleNotifyVolumeCreatedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyVolumeCreated(datas, reply);
    }
    return true;
}

bool HandleNotifyVolumeMountedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyVolumeMounted(datas, reply);
    }
    return true;
}

bool HandleNotifyVolumeStateChangedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyVolumeStateChanged(datas, reply);
    }
    return true;
}

bool HandleNotifyDiskCreatedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyDiskCreated(datas, reply);
    }
    return true;
}

bool HandleGetAllDisksFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetAllDisks(datas, reply);
    }
    return true;
}

bool HandleGetVolumeByUuidFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetVolumeByUuid(datas, reply);
    }
    return true;
}

bool HandleGetVolumeByIdFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetVolumeById(datas, reply);
    }
    return true;
}

bool HandleSetVolDescFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleSetVolDesc(datas, reply);
    }
    return true;
}

bool HandleQueryUsbIsInUseFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleQueryUsbIsInUse(datas, reply);
    }
    return true;
}

bool HandleGetDiskByIdFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetDiskById(datas, reply);
    }
    return true;
}

bool HandleUpdateUseAuthWithRecoveryKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUpdateUseAuthWithRecoveryKey(datas, reply);
    }
    return true;
}

bool HandleGetFileEncryptStatusFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetFileEncryptStatus(datas, reply);
    }
    return true;
}

bool HandleGetLockScreenStatusFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetLockScreenStatus(datas, reply);
    }
    return true;
}

bool HandleGenerateAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGenerateAppkey(datas, reply);
    }
    return true;
}

bool HandleCreateRecoverKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleCreateRecoverKey(datas, reply);
    }
    return true;
}

bool HandleSetRecoverKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleSetRecoverKey(datas, reply);
    }
    return true;
}

bool HandleResetSecretWithRecoveryKeyFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleResetSecretWithRecoveryKey(datas, reply);
    }
    return true;
}

bool HandleGetUserStorageStatsByTypeFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleGetUserStorageStatsByType(datas, reply);
    }
    return true;
}

bool HandleMountDfsDocsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleMountDfsDocs(datas, reply);
    }
    return true;
}

bool HandleUMountDfsDocsFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUMountDfsDocs(datas, reply);
    }
    return true;
}

bool HandleNotifyMtpMountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyMtpMount(datas, reply);
    }
    return true;
}

bool HandleNotifyMtpUnmountFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyMtpUnmount(datas, reply);
    }
    return true;
}

bool HandleNotifyDiskDestroyedFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleNotifyDiskDestroyed(datas, reply);
    }
    return true;
}

bool HandleMountMediaFuseFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleMountMediaFuse(datas, reply);
    }
    return true;
}

bool HandleUMountMediaFuseFuzzTest(const uint8_t *data, size_t size)
{
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;

    if (storageManagerPtr != nullptr) {
        storageManagerPtr->HandleUMountMediaFuse(datas, reply);
    }
    return true;
}
} // namespace OHOS::StorageManager

void FuzzerTest1(const uint8_t *data, size_t size)
{
    OHOS::StorageManager::HandlePrepareAddUserFuzzTest(data, size);
    OHOS::StorageManager::HandleRemoveUserFuzzTest(data, size);
    OHOS::StorageManager::HandlePrepareStartUserFuzzTest(data, size);
    OHOS::StorageManager::HandleStopUserFuzzTest(data, size);
    OHOS::StorageManager::HandleGetCurrentBundleStatsFuzzTest(data, size);
    OHOS::StorageManager::HandleMountFuzzTest(data, size);
    OHOS::StorageManager::HandleUnmountFuzzTest(data, size);
    OHOS::StorageManager::HandlePartitionFuzzTest(data, size);
    OHOS::StorageManager::HandleFormatFuzzTest(data, size);
    OHOS::StorageManager::HandleGenerateUserKeysFuzzTest(data, size);
    OHOS::StorageManager::HandleDeleteUserKeysFuzzTest(data, size);
    OHOS::StorageManager::HandleUpdateUserAuthFuzzTest(data, size);
    OHOS::StorageManager::HandleActiveUserKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleInactiveUserKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleLockUserScreenFuzzTest(data, size);
    OHOS::StorageManager::HandleUnlockUserScreenFuzzTest(data, size);
    OHOS::StorageManager::HandleUpdateKeyContextFuzzTest(data, size);
    OHOS::StorageManager::HandleCreateShareFileFuzzTest(data, size);
    OHOS::StorageManager::HandleDeleteShareFileFuzzTest(data, size);
    OHOS::StorageManager::HandleSetBundleQuotaFuzzTest(data, size);
    OHOS::StorageManager::HandleUpdateMemoryParaFuzzTest(data, size);
    OHOS::StorageManager::HandleDeleteAppkeyFuzzTest(data, size);
    OHOS::StorageManager::HandleCompleteAddUserFuzzTest(data, size);
    OHOS::StorageManager::HandleGetTotalFuzzTest(data, size);
    OHOS::StorageManager::HandleGetFreeFuzzTest(data, size);
    OHOS::StorageManager::HandleGetBundleStatusFuzzTest(data, size);
    OHOS::StorageManager::HandleGetSystemSizeFuzzTest(data, size);
    OHOS::StorageManager::HandleGetTotalSizeFuzzTest(data, size);
    OHOS::StorageManager::HandleGetFreeSizeFuzzTest(data, size);
    OHOS::StorageManager::HandleGetCurrUserStorageStatsFuzzTest(data, size);
    OHOS::StorageManager::HandleGetUserStorageStatsFuzzTest(data, size);
    OHOS::StorageManager::HandleGetAllVolumesFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyVolumeCreatedFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyVolumeMountedFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyVolumeStateChangedFuzzTest(data, size);
}

void FuzzerTest2(const uint8_t *data, size_t size)
{
    OHOS::StorageManager::HandleNotifyDiskCreatedFuzzTest(data, size);
    OHOS::StorageManager::HandleGetAllDisksFuzzTest(data, size);
    OHOS::StorageManager::HandleGetVolumeByUuidFuzzTest(data, size);
    OHOS::StorageManager::HandleGetVolumeByIdFuzzTest(data, size);
    OHOS::StorageManager::HandleSetVolDescFuzzTest(data, size);
    OHOS::StorageManager::HandleQueryUsbIsInUseFuzzTest(data, size);
    OHOS::StorageManager::HandleGetDiskByIdFuzzTest(data, size);
    OHOS::StorageManager::HandleUpdateUseAuthWithRecoveryKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleGetFileEncryptStatusFuzzTest(data, size);
    OHOS::StorageManager::HandleGetLockScreenStatusFuzzTest(data, size);
    OHOS::StorageManager::HandleGenerateAppkeyFuzzTest(data, size);
    OHOS::StorageManager::HandleCreateRecoverKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleSetRecoverKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleResetSecretWithRecoveryKeyFuzzTest(data, size);
    OHOS::StorageManager::HandleGetUserStorageStatsByTypeFuzzTest(data, size);
    OHOS::StorageManager::HandleMountDfsDocsFuzzTest(data, size);
    OHOS::StorageManager::HandleUMountDfsDocsFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyMtpMountFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyMtpUnmountFuzzTest(data, size);
    OHOS::StorageManager::HandleNotifyDiskDestroyedFuzzTest(data, size);
    OHOS::StorageManager::HandleMountMediaFuseFuzzTest(data, size);
    OHOS::StorageManager::HandleUMountMediaFuseFuzzTest(data, size);
}

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
    FuzzerTest1(data, size);
    FuzzerTest2(data, size);
    return 0;
}
