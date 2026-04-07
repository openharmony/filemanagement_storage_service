/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_MOUNT_MANAGER_MOCK_H
#define STORAGE_DAEMON_MOUNT_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "user/mount_manager.h"

namespace OHOS {
namespace StorageDaemon {
class IMountManagerMoc {
public:
    virtual ~IMountManagerMoc() = default;
public:
    virtual bool CheckMountFileByUser(int32_t userId) = 0;
    virtual int32_t PrepareAppdataDir(int32_t userId) = 0;
    virtual int32_t UMountDisShareFile(int32_t userId, const std::string &networkId) = 0;
    virtual int32_t MountDisShareFile(int32_t userId,
        const std::map<std::string, std::string> &shareFiles) = 0;
    virtual int32_t MountCryptoPathAgain(uint32_t userId) = 0;
    virtual int32_t MountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) = 0;
    virtual int32_t UMountDfsDocs(int32_t userId, const std::string &relativePath,
        const std::string &networkId, const std::string &deviceId) = 0;
    virtual int32_t MountMediaFuse(int32_t userId, int32_t &devFd) = 0;
    virtual int32_t UMountMediaFuse(int32_t userId) = 0;
    virtual int32_t MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd) = 0;
    virtual int32_t UMountFileMgrFuse(int32_t userId, const std::string &path) = 0;
    virtual int32_t IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
        std::vector<std::string> &outputList, bool &isOccupy) = 0;
    virtual int32_t ClearSecondMountPoint(uint32_t userId, const std::string &bundleName) = 0;
public:
    static inline std::shared_ptr<IMountManagerMoc> mountManagerMoc = nullptr;
};

class MountManagerMoc : public IMountManagerMoc {
public:
    MOCK_METHOD(bool, CheckMountFileByUser, (int32_t));
    MOCK_METHOD(int32_t, PrepareAppdataDir, (int32_t));
    MOCK_METHOD(int32_t, UMountDisShareFile, (int32_t, const std::string &));
    MOCK_METHOD(int32_t, MountDisShareFile, (int32_t, (const std::map<std::string, std::string>&)));
    MOCK_METHOD(int32_t, MountCryptoPathAgain, (uint32_t));
    MOCK_METHOD(int32_t, MountDfsDocs, (int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD(int32_t, UMountDfsDocs, (int32_t, const std::string &, const std::string &, const std::string &));
    MOCK_METHOD(int32_t, MountMediaFuse, (int32_t, int32_t &));
    MOCK_METHOD(int32_t, UMountMediaFuse, (int32_t));
    MOCK_METHOD(int32_t, MountFileMgrFuse, (int32_t, const std::string &, int32_t &));
    MOCK_METHOD(int32_t, UMountFileMgrFuse, (int32_t, const std::string &));
    MOCK_METHOD(int32_t, IsFileOccupied, (const std::string &, const std::vector<std::string> &,
        std::vector<std::string> &, bool &));
    MOCK_METHOD(int32_t, ClearSecondMountPoint, (uint32_t, const std::string &));
};
}
}
#endif