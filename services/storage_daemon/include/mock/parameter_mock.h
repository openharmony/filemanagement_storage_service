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

#ifndef MOCK_OHOS_MOCK_PARAMETER_H
#define MOCK_OHOS_MOCK_PARAMETER_H

#include <gmock/gmock.h>
#include <memory>

namespace OHOS {
namespace StorageDaemon {
class IParamMoc {
public:
    virtual ~IParamMoc() = default;
    virtual int GetParameter(const char *key, const char *def, char *value, uint32_t len) = 0;
    static inline std::shared_ptr<IParamMoc> paramMoc_;
};

class ParamMoc : public IParamMoc {
public:
    MOCK_METHOD4(GetParameter, int(const char *key, const char *def, char *value, uint32_t len));
};
}
}
#endif