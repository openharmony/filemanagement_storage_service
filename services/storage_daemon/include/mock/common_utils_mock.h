/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_COMMON_UTILS_MOCK_H
#define STORAGE_DAEMON_COMMON_UTILS_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include <string>

namespace OHOS {
namespace StorageDaemon {
class ICommonUtils {
public:
    virtual ~ICommonUtils() = default;
public:
    virtual bool LoadStringFromFile(const std::string&, std::string&) = 0;
public:
    inline static std::shared_ptr<ICommonUtils> utils = nullptr;
};

class CommonUtilsMock : public ICommonUtils {
public:
    MOCK_METHOD(bool, LoadStringFromFile, (const std::string&, std::string&));
};
}
}
#endif