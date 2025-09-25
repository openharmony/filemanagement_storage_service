/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "storagemanagerprovider_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <system_ability_definition.h>

#include "accesstoken_kit.h"
#include "ipc/storage_manager.h"
#include "ipc/storage_manager_provider.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "securec.h"
#include "storage_manager_stub.h"

using namespace OHOS::StorageManager;

namespace OHOS::Security::AccessToken {
ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    return Security::AccessToken::TOKEN_NATIVE;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string &permissionName)
{
    return Security::AccessToken::PermissionState::PERMISSION_GRANTED;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfoRes)
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

std::shared_ptr<StorageManagerProvider> storageManagerProvider =
    std::make_shared<StorageManagerProvider>(STORAGE_MANAGER_MANAGER_ID);

uint32_t GetU32Data(const char *ptr)
{
    // 将第0个数字左移24位，将第1个数字左移16位，将第2个数字左移8位，第3个数字不左移
    return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
}

bool g_storageManagerProviderFTest(std::unique_ptr<char[]> data, size_t size)
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
    storageManagerProvider->OnRemoteRequest(code % MAX_CALL_TRANSACTION, datas, reply, option);

    return true;
}

bool PrepareAddUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_PREPARE_ADD_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool RemoveUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_REMOVE_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool PrepareStartUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_PREPARE_START_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool StopUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_STOP_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetCurrentBundleStatsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_CURRENT_BUNDLE_STATS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool MountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_MOUNT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UnmountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_UNMOUNT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool PartitionFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_PARTITION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool FormatFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_FORMAT);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreateShareFileFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_CREATE_SHARE_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeleteShareFileFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_DELETE_SHARE_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CompleteAddUserFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_COMPLETE_ADD_USER);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetTotalSizeOfVolumeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_TOTAL_SIZE_OF_VOLUME);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetFreeSizeOfVolumeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_FREE_SIZE_OF_VOLUME);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetBundleStatsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_BUNDLE_STATS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetSystemSizeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_SYSTEM_SIZE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetTotalSizeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_TOTAL_SIZE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetFreeSizeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_FREE_SIZE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetUserStorageStatsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_USER_STORAGE_STATS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetUserStorageStatsIpcFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code =
        static_cast<int32_t>(IStorageManagerIpcCode::COMMAND_GET_USER_STORAGE_STATS_IN_INT_OUT_STORAGESTATS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetAllVolumesFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_ALL_VOLUMES);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyVolumeCreatedFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_VOLUME_CREATED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyVolumeMountedFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_VOLUME_MOUNTED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyVolumeStateChangedFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_VOLUME_STATE_CHANGED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyDiskCreatedFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_DISK_CREATED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetAllDisksFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_ALL_DISKS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetVolumeByUuidFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_VOLUME_BY_UUID);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetVolumeByIdFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_VOLUME_BY_ID);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool SetVolumeDescriptionFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_SET_VOLUME_DESCRIPTION);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool QueryUsbIsInUseFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_QUERY_USB_IS_IN_USE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetDiskByIdFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_DISK_BY_ID);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UpdateUseAuthWithRecoveryKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_UPDATE_USE_AUTH_WITH_RECOVERY_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetFileEncryptStatusFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_FILE_ENCRYPT_STATUS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetLockScreenStatusFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_LOCK_SCREEN_STATUS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GenerateAppkeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GENERATE_APPKEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreateRecoverKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_CREATE_RECOVER_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool SetRecoverKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_SET_RECOVER_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool ResetSecretWithRecoveryKeyFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_RESET_SECRET_WITH_RECOVERY_KEY);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool GetUserStorageStatsByTypeFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_GET_USER_STORAGE_STATS_BY_TYPE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool MountDfsDocsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_MOUNT_DFS_DOCS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UMountDfsDocsFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_UMOUNT_DFS_DOCS);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyMtpMountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_MTP_MOUNTED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyMtpUnmountFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_MTP_UNMOUNTED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool NotifyDiskDestroyedFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_NOTIFY_DISK_DESTROYED);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool MountMediaFuseFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_MOUNT_MEDIA_FUSE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool UMountMediaFuseFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_UMOUNT_MEDIA_FUSE);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool CreateUserDirFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_CREATE_USER_DIR);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}

bool DeleteUserDirFuzzTest(const uint8_t *data, size_t size)
{
    uint32_t code = static_cast<uint32_t>(IStorageManagerIpcCode::COMMAND_DELETE_USER_DIR);
    MessageParcel datas;
    datas.WriteInterfaceToken(StorageManagerStub::GetDescriptor());
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    storageManagerProvider->OnRemoteRequest(code, datas, reply, option);
    return true;
}
} // namespace OHOS::StorageManager

void FuzzerTest1(const uint8_t *data, size_t size)
{
    OHOS::StorageManager::PrepareAddUserFuzzTest(data, size);
    OHOS::StorageManager::RemoveUserFuzzTest(data, size);
    OHOS::StorageManager::PrepareStartUserFuzzTest(data, size);
    OHOS::StorageManager::StopUserFuzzTest(data, size);
    OHOS::StorageManager::GetCurrentBundleStatsFuzzTest(data, size);
    OHOS::StorageManager::MountFuzzTest(data, size);
    OHOS::StorageManager::UnmountFuzzTest(data, size);
    OHOS::StorageManager::PartitionFuzzTest(data, size);
    OHOS::StorageManager::FormatFuzzTest(data, size);
    OHOS::StorageManager::CreateShareFileFuzzTest(data, size);
    OHOS::StorageManager::DeleteShareFileFuzzTest(data, size);
    OHOS::StorageManager::CompleteAddUserFuzzTest(data, size);
    OHOS::StorageManager::GetTotalSizeOfVolumeFuzzTest(data, size);
    OHOS::StorageManager::GetFreeSizeOfVolumeFuzzTest(data, size);
    OHOS::StorageManager::GetBundleStatsFuzzTest(data, size);
    OHOS::StorageManager::GetSystemSizeFuzzTest(data, size);
    OHOS::StorageManager::GetTotalSizeFuzzTest(data, size);
    OHOS::StorageManager::GetFreeSizeFuzzTest(data, size);
    OHOS::StorageManager::GetUserStorageStatsFuzzTest(data, size);
    OHOS::StorageManager::GetUserStorageStatsIpcFuzzTest(data, size);
    OHOS::StorageManager::GetAllVolumesFuzzTest(data, size);
    OHOS::StorageManager::NotifyVolumeCreatedFuzzTest(data, size);
    OHOS::StorageManager::NotifyVolumeMountedFuzzTest(data, size);
    OHOS::StorageManager::NotifyVolumeStateChangedFuzzTest(data, size);
}

void FuzzerTest2(const uint8_t *data, size_t size)
{
    OHOS::StorageManager::NotifyDiskCreatedFuzzTest(data, size);
    OHOS::StorageManager::GetAllDisksFuzzTest(data, size);
    OHOS::StorageManager::GetVolumeByUuidFuzzTest(data, size);
    OHOS::StorageManager::GetVolumeByIdFuzzTest(data, size);
    OHOS::StorageManager::SetVolumeDescriptionFuzzTest(data, size);
    OHOS::StorageManager::QueryUsbIsInUseFuzzTest(data, size);
    OHOS::StorageManager::GetDiskByIdFuzzTest(data, size);
    OHOS::StorageManager::UpdateUseAuthWithRecoveryKeyFuzzTest(data, size);
    OHOS::StorageManager::GetFileEncryptStatusFuzzTest(data, size);
    OHOS::StorageManager::GetLockScreenStatusFuzzTest(data, size);
    OHOS::StorageManager::GenerateAppkeyFuzzTest(data, size);
    OHOS::StorageManager::CreateRecoverKeyFuzzTest(data, size);
    OHOS::StorageManager::SetRecoverKeyFuzzTest(data, size);
    OHOS::StorageManager::ResetSecretWithRecoveryKeyFuzzTest(data, size);
    OHOS::StorageManager::GetUserStorageStatsByTypeFuzzTest(data, size);
    OHOS::StorageManager::MountDfsDocsFuzzTest(data, size);
    OHOS::StorageManager::UMountDfsDocsFuzzTest(data, size);
    OHOS::StorageManager::NotifyMtpMountFuzzTest(data, size);
    OHOS::StorageManager::NotifyMtpUnmountFuzzTest(data, size);
    OHOS::StorageManager::NotifyDiskDestroyedFuzzTest(data, size);
    OHOS::StorageManager::MountMediaFuseFuzzTest(data, size);
    OHOS::StorageManager::UMountMediaFuseFuzzTest(data, size);
    OHOS::StorageManager::CreateUserDirFuzzTest(data, size);
    OHOS::StorageManager::DeleteUserDirFuzzTest(data, size);
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

    OHOS::StorageManager::g_storageManagerProviderFTest(move(str), size);
    FuzzerTest1(data, size);
    FuzzerTest2(data, size);
    return 0;
}