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

#ifndef STORAGE_DAEMON_CLIENT_H
#define STORAGE_DAEMON_CLIENT_H

#include <iostream>

#include "ipc/storage_daemon.h"
#include "istorage_daemon.h"
namespace OHOS {
namespace StorageDaemon {
class StorageDaemonClient {
public:
    static int32_t PrepareUserDirs(int32_t userId, uint32_t flags);
    static int32_t DestroyUserDirs(int32_t userId, uint32_t flags);
    static int32_t StartUser(int32_t userId);
    static int32_t StopUser(int32_t userId);
    static int32_t PrepareUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags);
    static int32_t DestroyUserSpace(uint32_t userId, const std::string &volumId, uint32_t flags);
    static int32_t InitGlobalKey(void);
    static int32_t InitGlobalUserKeys(void);
    static int32_t DeleteUserKeys(uint32_t userId);
    static int32_t EraseAllUserEncryptedKeys();
    static int32_t UpdateUserAuth(uint32_t userId, uint64_t secureUid,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &oldSecret,
                                  const std::vector<uint8_t> &newSecret);
    static int32_t UpdateUseAuthWithRecoveryKey(const std::vector<uint8_t> &authToken,
                                                const std::vector<uint8_t> &newSecret,
                                                uint64_t secureUid,
                                                uint32_t userId,
                                                std::vector<std::vector<uint8_t>> &plainText);
    static int32_t ActiveUserKey(uint32_t userId,
                                 const std::vector<uint8_t> &token,
                                 const std::vector<uint8_t> &secret);
    static int32_t InactiveUserKey(uint32_t userId);
    static int32_t FscryptEnable(const std::string &fscryptOptions);
    static int32_t UpdateKeyContext(uint32_t userId, bool needRemoveTmpKey = false);
    static int32_t LockUserScreen(uint32_t userId);
    static int32_t UnlockUserScreen(uint32_t userId,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret);
    static int32_t GetLockScreenStatus(uint32_t userId, bool &lockScreenStatus);
    static int32_t GenerateAppkey(uint32_t userId, uint32_t hashId, std::string &keyId);
    static int32_t DeleteAppkey(uint32_t userId, const std::string keyId);
    static int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    static int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId);
    static int32_t GetFileEncryptStatus(uint32_t userId, bool &isEncrypted, bool needCheckDirMount = false);
    static int32_t GetUserNeedActiveStatus(uint32_t userId, bool &needActive);
    static int32_t CreateRecoverKey(uint32_t userId,
                                    uint32_t userType,
                                    const std::vector<uint8_t> &token,
                                    const std::vector<uint8_t> &secret);
    static int32_t SetRecoverKey(const std::vector<uint8_t> &key);
    // file mgr fuse
    static int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd);
    static int32_t UMountFileMgrFuse(int32_t userId, const std::string &path);
    // file lock
    static int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy);
    static int32_t SetDirEncryptionPolicy(uint32_t userId, const std::string &dirPath, uint32_t level);
    static int32_t ListUserdataDirInfo(std::vector<UserdataDirInfo> &scanDirs);

private:
    static sptr<IStorageDaemon> GetStorageDaemonProxy(void);
    static int32_t CheckServiceStatus(uint32_t serviceFlags);
};
} // StorageDaemon
} // OHOS

#endif
