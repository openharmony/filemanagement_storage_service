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
#ifndef MOCK_STORAGE_DAEMON_STUB_H
#define MOCK_STORAGE_DAEMON_STUB_H

#include "gmock/gmock.h"

#include "storage_daemon_stub.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonStubMock : public StorageDaemonStub {
public:
    MOCK_METHOD0(Shutdown, int32_t(void));

    MOCK_METHOD2(Mount, int32_t(const std::string &, uint32_t));
    MOCK_METHOD1(UMount, int32_t(const std::string &));
    MOCK_METHOD2(TryToFix, int32_t(const std::string &, uint32_t));
    MOCK_METHOD1(Check, int32_t(const std::string &));
    MOCK_METHOD2(Format, int32_t(const std::string &, const std::string &));
    MOCK_METHOD2(Partition, int32_t(const std::string &, int32_t));
    MOCK_METHOD2(SetVolumeDescription, int32_t(const std::string &, const std::string &));
    MOCK_METHOD2(QueryUsbIsInUse, int32_t(const std::string &, bool &));

    MOCK_METHOD1(StartUser, int32_t(int32_t));
    MOCK_METHOD1(StopUser, int32_t(int32_t));
    MOCK_METHOD1(CompleteAddUser, int32_t(int32_t));
    MOCK_METHOD2(PrepareUserDirs, int32_t(int32_t, uint32_t));
    MOCK_METHOD2(DestroyUserDirs, int32_t(int32_t, uint32_t));

    MOCK_METHOD0(InitGlobalKey, int32_t(void));
    MOCK_METHOD0(InitGlobalUserKeys, int32_t(void));
    MOCK_METHOD1(DeleteUserKeys, int32_t(uint32_t));
    MOCK_METHOD0(EraseAllUserEncryptedKeys, int32_t(void));
    MOCK_METHOD5(UpdateUserAuth,  int32_t(uint32_t, uint64_t, const std::vector<uint8_t> &,
        const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD5(UpdateUseAuthWithRecoveryKey, int32_t(const std::vector<uint8_t> &,
        const std::vector<uint8_t> &, uint64_t, uint32_t, const std::vector<std::vector<uint8_t>> &));
    MOCK_METHOD3(ActiveUserKey,  int32_t (uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD1(InactiveUserKey, int32_t (uint32_t));
    MOCK_METHOD1(UpdateUserPublicDirPolicy, int32_t (uint32_t));
    MOCK_METHOD2(UpdateKeyContext, int32_t (uint32_t, bool));
    MOCK_METHOD1(MountCryptoPathAgain, int32_t (uint32_t));
    MOCK_METHOD4(CreateShareFile,
                 int32_t(const StorageManager::StorageFileRawData &, uint32_t, uint32_t, std::vector<int32_t> &));
    MOCK_METHOD2(DeleteShareFile, int32_t (uint32_t, const StorageManager::StorageFileRawData &));
    MOCK_METHOD4(SetBundleQuota, int32_t (const std::string&, int32_t, const std::string&, int32_t));
    MOCK_METHOD3(GetOccupiedSpace, int32_t (int32_t, int32_t, int64_t &));
    MOCK_METHOD1(LockUserScreen, int32_t (uint32_t));
    MOCK_METHOD3(UnlockUserScreen, int32_t (uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD2(GetLockScreenStatus, int32_t (uint32_t, bool &));
    MOCK_METHOD4(MountDfsDocs, int32_t(int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD4(UMountDfsDocs, int32_t(int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD2(UpdateMemoryPara, int32_t (int32_t, int32_t &));
    MOCK_METHOD4(GenerateAppkey, int32_t (uint32_t, uint32_t, std::string &, bool));
    MOCK_METHOD2(DeleteAppkey, int32_t (uint32_t, const std::string &));
    MOCK_METHOD3(GetFileEncryptStatus, int32_t (uint32_t, bool &, bool));
    MOCK_METHOD2(GetUserNeedActiveStatus, int32_t(uint32_t, bool &));
    MOCK_METHOD4(CreateRecoverKey, int32_t (uint32_t, uint32_t, const std::vector<uint8_t> &,
                                            const std::vector<uint8_t> &));
    MOCK_METHOD1(SetRecoverKey, int32_t (const std::vector<uint8_t> &));
    MOCK_METHOD3(ResetSecretWithRecoveryKey, int32_t(uint32_t, uint32_t, const std::vector<uint8_t> &));
    MOCK_METHOD2(MountMediaFuse, int32_t (int32_t, int32_t &));
    MOCK_METHOD1(UMountMediaFuse, int32_t (int32_t));
    MOCK_METHOD3(MountFileMgrFuse, int32_t (int32_t, const std::string &, int32_t &));
    MOCK_METHOD2(UMountFileMgrFuse, int32_t (int32_t, const std::string &));
    MOCK_METHOD1(AddDeathRecipient, bool(const sptr<DeathRecipient> &));
    MOCK_METHOD1(RemoveDeathRecipient, bool(const sptr<DeathRecipient> &));
    MOCK_METHOD4(IsFileOccupied, int32_t (const std::string &, const std::vector<std::string> &,
        std::vector<std::string> &, bool &));
    MOCK_METHOD2(MountDisShareFile, int32_t (int32_t, const std::map<std::string, std::string> &));
    MOCK_METHOD2(UMountDisShareFile, int32_t (int32_t, const std::string &));
    MOCK_METHOD1(InactiveUserPublicDirKey, int32_t(uint32_t));
    MOCK_METHOD2(QueryOccupiedSpaceForSa, int32_t(const std::string &, const std::map<int32_t, std::string> &));
    MOCK_METHOD3(MountUsbFuse, int32_t(const std::string &, std::string &, int &));
    MOCK_METHOD1(RegisterUeceActivationCallback, int32_t(const sptr<IUeceActivationCallback>&));
    MOCK_METHOD0(UnregisterUeceActivationCallback, int32_t(void));
    MOCK_METHOD3(SetDirEncryptionPolicy, int32_t(uint32_t, const std::string &, uint32_t));
    MOCK_METHOD4(CreateUserDir, int32_t(const std::string &, mode_t, uid_t, gid_t));
    MOCK_METHOD1(DeleteUserDir, int32_t(const std::string &));
    MOCK_METHOD0(StatisticSysDirSpace, int32_t(void));

    MOCK_METHOD2(GetDataSizeByPath, int32_t(const std::string &, int64_t &));
    MOCK_METHOD2(GetRmgResourceSize, int32_t(const std::string &, uint64_t &));
};
}  // namespace StorageDaemon
}  // namespace OHOS
#endif /* MOCK_STORAGE_DAEMON_STUB_H */
