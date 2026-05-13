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

#include "burn_params.h"
#include "mock/storage_daemon_proxy_mock.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
StorageDaemonProxy::StorageDaemonProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IStorageDaemon>(impl)
{}

int32_t StorageDaemonProxy::Shutdown()
{
    return E_OK;
}

int32_t StorageDaemonProxy::Mount(const std::string &volId, uint32_t flags)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMount(const std::string &volId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::TryToFix(const std::string &volId, uint32_t flags)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Check(const std::string &volId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Format(const std::string &volId, const std::string &fsType)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Partition(const std::string &diskId, int32_t type)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SetVolumeDescription(const std::string &volId, const std::string &description)
{
    return E_OK;
}

int32_t StorageDaemonProxy::QueryUsbIsInUse(const std::string &diskPath, bool &isInUse)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize)
{
    return E_OK;
}

int32_t StorageDaemonProxy::PrepareUserDirs(int32_t userId, uint32_t flags)
{
    return E_OK;
}

int32_t StorageDaemonProxy::DestroyUserDirs(int32_t userId, uint32_t flags)
{
    return E_OK;
}

int32_t StorageDaemonProxy::StartUser(int32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::StopUser(int32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::CompleteAddUser(int32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::InitGlobalKey(void)
{
    return E_OK;
}

int32_t StorageDaemonProxy::InitGlobalUserKeys(void)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t type)
{
    return E_OK;
}

int32_t StorageDaemonProxy::EraseAllUserEncryptedKeys(const std::vector<int32_t> &localIdList)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                           const std::vector<uint8_t> &token,
                                           const std::vector<uint8_t> &oldSecret,
                                           const std::vector<uint8_t> &newSecret)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                         const std::vector<uint8_t> &newSecret,
                                                         uint64_t secureUid,
                                                         uint32_t userId,
                                                         const std::vector<std::vector<uint8_t>> &plainText)
{
    return E_OK;
}

int32_t StorageDaemonProxy::ResetSecretWithRecoveryKey(uint32_t userId,
                                                       uint32_t rkType,
                                                       const std::vector<uint8_t> &key)
{
    return E_OK;
}

int32_t StorageDaemonProxy::ActiveUserKey(uint32_t userId,
                                          const std::vector<uint8_t> &token,
                                          const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int32_t StorageDaemonProxy::InactiveUserKey(uint32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::LockUserScreen(uint32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UnlockUserScreen(uint32_t userId,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId, bool needReSet)
{
    return E_OK;
}

int32_t StorageDaemonProxy::DeleteAppkey(uint32_t userId, const std::string &keyId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::CreateRecoverKey(uint32_t userId,
                                             uint32_t userType,
                                             const std::vector<uint8_t> &token,
                                             const std::vector<uint8_t> &secret)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SetRecoverKey(const std::vector<uint8_t> &key)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey)
{
    return E_OK;
}

int32_t StorageDaemonProxy::CreateShareFile(const StorageFileRawData &rawData,
                                            uint32_t tokenId,
                                            uint32_t flag,
                                            std::vector<int32_t> &funcResult)
{
    funcResult = {1};
    return E_OK;
}

int32_t StorageDaemonProxy::DeleteShareFile(uint32_t tokenId, const StorageFileRawData &rawData)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SetBundleQuota(int32_t uid,
    const std::string &bundleDataDirPath, int32_t limitSizeMb)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size)
{
    return E_OK;
}

int32_t StorageDaemonProxy::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetUserNeedActiveStatus(uint32_t userId, bool &needActive)
{
    return E_OK;
}

int32_t StorageDaemonProxy::MountMediaFuse(int32_t userId, int32_t &devFd)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMountMediaFuse(int32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    return E_OK;
}

int32_t StorageDaemonProxy::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    return E_OK;
}

int32_t StorageDaemonProxy::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::InactiveUserPublicDirKey(uint32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UpdateUserPublicDirPolicy(uint32_t userId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::QueryOccupiedSpaceForSa(std::vector<UidSaInfo> &vec, int64_t &totalSize,
    const std::map<int32_t, std::string> &bundleNameAndUid, int32_t type)
{
    return E_OK;
}

int32_t StorageDaemonProxy::MountUsbFuse(const std::string &volumeId, std::string &fsUuid, int &fuseFd)
{
    return E_OK;
}

int32_t StorageDaemonProxy::RegisterUeceActivationCallback(const sptr<IUeceActivationCallback>& callback)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UnregisterUeceActivationCallback()
{
    return E_OK;
}

int32_t StorageDaemonProxy::CreateUserDir(const std::string &path, mode_t mode, uid_t uid, gid_t gid)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetDirListSpace(const std::vector<DirSpaceInfo> &inDirs,
    std::vector<DirSpaceInfo> &outDirs)
{
    return E_OK;
}

int32_t StorageDaemonProxy::SetStopScanFlag(bool stop)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetAncoSizeData(std::string &outExtraData)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetDataSizeByPath(const std::string &path, int64_t &size)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetRmgResourceSize(const std::string &rgmName, uint64_t &totalSize)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetSystemDataSize(int64_t &otherUidSizeSum)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetDirListSpaceByPaths(const std::vector<std::string> &paths,
    const std::vector<int32_t> &uids, std::vector<DirSpaceInfo> &resultDirs,
    std::vector<LargeFileInfo> &largeFiles, std::vector<LargeDirInfo> &largeDirs)
{
    return E_OK;
}

int32_t StorageDaemonProxy::ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Encrypt(const std::string &volumeId, const std::string &pazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetCryptProgressById(const std::string &volumeId, int32_t &progress)
{
    progress = 0;
    return E_OK;
}

int32_t StorageDaemonProxy::GetCryptUuidById(const std::string &volumeId, std::string &uuid)
{
    return E_OK;
}

int32_t StorageDaemonProxy::BindRecoverKeyToPasswd(const std::string &volumeId,
                                                   const std::string &pazzword,
                                                   const std::string &recoverKey)
{
    return E_OK;
}

int32_t StorageDaemonProxy::UpdateCryptPasswd(const std::string &volumeId,
                                              const std::string &pazzword,
                                              const std::string &newPazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::ResetCryptPasswd(const std::string &volumeId,
                                             const std::string &recoverKey,
                                             const std::string &newPazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Unlock(const std::string &volumeId, const std::string &pazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Decrypt(const std::string &volumeId, const std::string &pazzword)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Eject(const std::string &volumeId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetOpticalDriveOpsProgress(const std::string &volumeId, uint32_t &progress)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Erase(const std::string &volumeId)
{
    return E_OK;
}

int32_t StorageDaemonProxy::CreateIsoImage(const std::string &volumeId, const std::string &filePath)
{
    return E_OK;
}

int32_t StorageDaemonProxy::GetPartitionTable(const std::string &diskId, PartitionTableInfo &partitionTableInfo)
{
    return E_OK;
}

int32_t StorageDaemonProxy::CreatePartition(const std::string &diskId, const PartitionOptions &partitionOption)
{
    return E_OK;
}

int32_t StorageDaemonProxy::DeletePartition(const std::string &diskId, uint32_t partitionNum)
{
    return E_OK;
}

int32_t StorageDaemonProxy::FormatPartition(const std::string &diskId, uint32_t partitionNum,
    const FormatOptions &options)
{
    return E_OK;
}

int32_t StorageDaemonProxy::Burn(const std::string &volumeId, const BurnParams &params)
{
    return E_OK;
}

int32_t StorageDaemonProxy::VerifyBurnData(const std::string &volumeId, uint32_t verType)
{
    return E_OK;
}
} // StorageDaemon
} // namespace OHOS
