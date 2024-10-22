/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_ElX_FILEKEYMANAGER_KIT_MOCK_H
#define STORAGE_DAEMON_ElX_FILEKEYMANAGER_KIT_MOCK_H

#include <gmock/gmock.h>
#include <memory>

namespace OHOS {
namespace StorageDaemon {
class IEl5FilekeyManagerKitMoc {
public:
    virtual ~IEl5FilekeyManagerKitMoc() = default;
public:
    virtual int32_t GetUserAllAppKey(int32_t userId, std::vector<std::pair<int32_t, std::string>> &keyInfos) = 0;
    virtual int32_t GetUserAppKey(int32_t userId, std::vector<std::pair<int32_t, std::string>> &keyInfos) = 0;
    virtual int32_t ChangeUserAppkeysLoadInfo(int32_t userId, std::vector<std::pair<std::string, bool>> &loadInfos) = 0;
public:
    static inline std::shared_ptr<IEl5FilekeyManagerKitMoc> el5FilekeyManagerKitMoc = nullptr;
};

class El5FilekeyManagerKitMoc : public IEl5FilekeyManagerKitMoc {
public:
    MOCK_METHOD2(GetUserAllAppKey, int32_t(int32_t userId, std::vector<std::pair<int32_t, std::string>> &keyInfos));
    MOCK_METHOD2(GetUserAppKey, int32_t(int32_t userId, std::vector<std::pair<int32_t, std::string>> &keyInfos));
    MOCK_METHOD2(ChangeUserAppkeysLoadInfo, int32_t(int32_t userId,
        std::vector<std::pair<std::string, bool>> &loadInfos));
};
}
}
#endif