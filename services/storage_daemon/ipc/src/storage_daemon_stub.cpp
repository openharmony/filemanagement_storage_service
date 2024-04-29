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

#include "ipc/storage_daemon_stub.h"

#include "ipc/storage_daemon_ipc_interface_code.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "string_ex.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;

StorageDaemonStub::StorageDaemonStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SHUTDOWN)] =
        &StorageDaemonStub::HandleShutdown;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CHECK)] =
        &StorageDaemonStub::HandleCheck;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT)] =
        &StorageDaemonStub::HandleMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT)] =
        &StorageDaemonStub::HandleUMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::PARTITION)] =
        &StorageDaemonStub::HandlePartition;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::FORMAT)] =
        &StorageDaemonStub::HandleFormat;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC)] =
        &StorageDaemonStub::HandleSetVolDesc;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS)] =
        &StorageDaemonStub::HandlePrepareUserDirs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS)] =
        &StorageDaemonStub::HandleDestroyUserDirs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::START_USER)] =
        &StorageDaemonStub::HandleStartUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::STOP_USER)] =
        &StorageDaemonStub::HandleStopUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY)] =
        &StorageDaemonStub::HandleInitGlobalKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS)] =
        &StorageDaemonStub::HandleInitGlobalUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS)] =
        &StorageDaemonStub::HandleGenerateUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS)] =
        &StorageDaemonStub::HandleDeleteUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH)] =
        &StorageDaemonStub::HandleUpdateUserAuth;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY)] =
        &StorageDaemonStub::HandleActiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY)] =
        &StorageDaemonStub::HandleInactiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN)] =
        &StorageDaemonStub::HandleLockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN)] =
        &StorageDaemonStub::HandleUnlockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_SCREEN_STATUS)] =
        &StorageDaemonStub::HandleGetLockScreenStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT)] =
        &StorageDaemonStub::HandleUpdateKeyContext;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_CRYPTO_PATH_AGAIN)] =
        &StorageDaemonStub::HandleMountCryptoPathAgain;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE)] =
        &StorageDaemonStub::HandleCreateShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE)] =
        &StorageDaemonStub::HandleDeleteShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_BUNDLE_QUOTA)] =
        &StorageDaemonStub::HandleSetBundleQuota;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_SPACE)] =
        &StorageDaemonStub::HandleGetOccupiedSpace;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_MEM_PARA)] =
        &StorageDaemonStub::HandleUpdateMemoryPara;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_BUNDLE_STATS_INCREASE)] =
        &StorageDaemonStub::HandleGetBundleStatsForIncrease;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS)] =
        &StorageDaemonStub::HandleMountDfsDocs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY)] =
        &StorageDaemonStub::HandleGenerateAppkey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY)] =
        &StorageDaemonStub::HandleDeleteAppkey;
}

int32_t StorageDaemonStub::OnRemoteRequest(uint32_t code,
                                           MessageParcel &data,
                                           MessageParcel &reply,
                                           MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_PERMISSION_DENIED;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t StorageDaemonStub::HandleShutdown(MessageParcel &data, MessageParcel &reply)
{
    Shutdown();
    return E_OK;
}

int32_t StorageDaemonStub::HandleMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    uint32_t flags = data.ReadUint32();

    int err = Mount(volId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = UMount(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleCheck(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = Check(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleFormat(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string fsType = data.ReadString();

    int err = Format(volId, fsType);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandlePartition(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    int32_t type = data.ReadInt32();

    int err = Partition(volId, type);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleSetVolDesc(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string description = data.ReadString();

    int err = SetVolumeDescription(volId, description);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandlePrepareUserDirs(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();

    int err = PrepareUserDirs(userId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDestroyUserDirs(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();

    int err = DestroyUserDirs(userId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleStartUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();

    int32_t err = StartUser(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleStopUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();

    int32_t err = StopUser(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalKey(MessageParcel &data, MessageParcel &reply)
{
    int err = InitGlobalKey();
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalUserKeys(MessageParcel &data, MessageParcel &reply)
{
    int err = InitGlobalUserKeys();
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t flags = data.ReadUint32();

    int err = GenerateUserKeys(userId, flags);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = DeleteUserKeys(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint64_t secureUid = data.ReadUint64();

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&oldSecret);
    data.ReadUInt8Vector(&newSecret);

    int err = UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleActiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int err = ActiveUserKey(userId, token, secret);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = InactiveUserKey(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleLockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = LockUserScreen(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUnlockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int err = UnlockUserScreen(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleGetLockScreenStatus(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    bool lockScreenStatus = false;
    int err = GetLockScreenStatus(userId, lockScreenStatus);
    if (!reply.WriteBool(lockScreenStatus)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleGenerateAppkey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t appUid = data.ReadUint32();
    std::string keyId;
    int err = GenerateAppkey(userId, appUid, keyId);
    if (!reply.WriteString(keyId)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteAppkey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::string keyId = data.ReadString();
    int err = DeleteAppkey(userId, keyId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int err = UpdateKeyContext(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleMountCryptoPathAgain(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int32_t err = MountCryptoPathAgain(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleCreateShareFile(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> uriList;
    if (!data.ReadStringVector(&uriList)) {
        return E_WRITE_REPLY_ERR;
    }
    uint32_t tokenId = data.ReadUint32();
    uint32_t flag = data.ReadUint32();
    std::vector<int32_t> retList = CreateShareFile(uriList, tokenId, flag);
    if (!reply.WriteInt32Vector(retList)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply)
{
    uint32_t tokenId = data.ReadUint32();
    std::vector<std::string> uriList;
    if (!data.ReadStringVector(&uriList)) {
        return E_WRITE_REPLY_ERR;
    }
    int err = DeleteShareFile(tokenId, uriList);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleSetBundleQuota(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int32_t uid = data.ReadInt32();
    std::string bundleDataDirPath = data.ReadString();
    int32_t limitSizeMb = data.ReadInt32();
    int err = SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetOccupiedSpace(MessageParcel &data, MessageParcel &reply)
{
    int32_t idType = data.ReadInt32();
    int32_t id = data.ReadInt32();
    int64_t size = 0;
    int err = GetOccupiedSpace(idType, id, size);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(size)) {
        LOGE("StorageManagerStub::HandleGetFree call GetTotalSize failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}
int32_t StorageDaemonStub::HandleUpdateMemoryPara(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    int32_t oldSize = 0;
    int err = UpdateMemoryPara(size, oldSize);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(oldSize)) {
    LOGE("StorageManagerStub::HandleUpdateMemoryPara call Write oldSize failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetBundleStatsForIncrease(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::vector<std::string> bundleNames;
    if (!data.ReadStringVector(&bundleNames)) {
        return E_WRITE_REPLY_ERR;
    }
    std::vector<int64_t> incrementalBackTimes;
    if (!data.ReadInt64Vector(&incrementalBackTimes)) {
        return E_WRITE_REPLY_ERR;
    }

    std::vector<int64_t> pkgFileSizes;
    int32_t err = GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64Vector(pkgFileSizes)) {
        LOGE("StorageDaemonStub::HandleGetBundleStatsForIncrease call GetBundleStatsForIncrease failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleMountDfsDocs(MessageParcel &data, MessageParcel &reply)
{
    LOGI("StorageDaemonStub::HandleMountDfsDocs start.");
    int32_t userId = data.ReadInt32();
    std::string relativePath = data.ReadString();
    std::string networkId = data.ReadString();
    std::string deviceId = data.ReadString();

    int32_t err = MountDfsDocs(userId, relativePath, networkId, deviceId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

} // StorageDaemon
} // OHOS
