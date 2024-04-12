/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_CRYPTO_ANCO_KEY_MANAGER_H
#define STORAGE_DAEMON_CRYPTO_ANCO_KEY_MANAGER_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "storage_service_constant.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {
class AncoKeyManager {
public:
    static AncoKeyManager *GetInstance(void)
    {
        static AncoKeyManager instance;
        return &instance;
    }
    int32_t SetAncoDirectoryElPolicy(const std::string &path, const std::string &policyType, unsigned int user);

private:
    int32_t ReadFileAndCreateDir(const std::string &path, const std::string &type, std::vector<FileList> &vec);
    int32_t CreatePolicyDir(const AncoDirInfo &ancoDirInfo, const std::string &type, std::vector<FileList> &vec);
    int32_t CheckMemberValid(const AncoDirInfo &ancoDirInfo);
    void SetUserPermissionMap();
    static std::map<std::string, std::string> ownerMap_;
    std::mutex Mutex_;
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // STORAGE_DAEMON_CRYPTO_ANCO_KEY_MANAGER_H
