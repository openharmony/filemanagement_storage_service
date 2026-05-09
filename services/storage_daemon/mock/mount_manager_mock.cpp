/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mock/mount_manager_mock.h"

using namespace std;

using namespace OHOS::StorageDaemon;

MountManager &MountManager::GetInstance()
{
    static MountManager instance_;
    return instance_;
}

bool MountManager::CheckMountFileByUser(int32_t userId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return false;
    }
    return IMountManagerMoc::mountManagerMoc->CheckMountFileByUser(userId);
}

int32_t MountManager::PrepareAppdataDir(int32_t userId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->PrepareAppdataDir(userId);
}

int32_t MountManager::UMountDisShareFile(int32_t userId, const std::string &networkId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->UMountDisShareFile(userId, networkId);
}

int32_t MountManager::MountDisShareFile(int32_t userId, const std::map<std::string, std::string> &shareFiles)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->MountDisShareFile(userId, shareFiles);
}

int32_t MountManager::MountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->MountDfsDocs(userId, relativePath,
        networkId, deviceId);
}

int32_t MountManager::UMountDfsDocs(int32_t userId, const std::string &relativePath,
    const std::string &networkId, const std::string &deviceId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->UMountDfsDocs(userId, relativePath,
        networkId, deviceId);
}

int32_t MountManager::MountMediaFuse(int32_t userId, int32_t &devFd)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->MountMediaFuse(userId, devFd);
}

int32_t MountManager::UMountMediaFuse(int32_t userId)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->UMountMediaFuse(userId);
}

int32_t MountManager::MountFileMgrFuse(int32_t userId, const std::string &path, int32_t &fuseFd)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->MountFileMgrFuse(userId, path, fuseFd);
}

int32_t MountManager::UMountFileMgrFuse(int32_t userId, const std::string &path)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->UMountFileMgrFuse(userId, path);
}

int32_t MountManager::IsFileOccupied(const std::string &path, const std::vector<std::string> &inputList,
    std::vector<std::string> &outputList, bool &isOccupy)
{
    if (IMountManagerMoc::mountManagerMoc == nullptr) {
        return -1;
    }
    return IMountManagerMoc::mountManagerMoc->IsFileOccupied(path, inputList, outputList, isOccupy);
}