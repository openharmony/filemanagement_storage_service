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

#ifndef DATASHARE_HELPER_MOCK_H
#define DATASHARE_HELPER_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "datashare_helper.h"

namespace OHOS {
namespace DataShare {
class DataShareHelperMock : public DataShareHelper {
public:
    MOCK_METHOD2(Creator, std::shared_ptr<DataShareHelper>(const sptr<IRemoteObject> &, const std::string &));
    MOCK_METHOD4(Query, std::shared_ptr<DataShareResultSet>(Uri &, const DataSharePredicates &,
        std::vector<std::string> &, DatashareBusinessError *));
    MOCK_METHOD2(RegisterObserver, void(const Uri &, const sptr<AAFwk::IDataAbilityObserver> &));
    MOCK_METHOD2(UnregisterObserver, void(const Uri &, const sptr<AAFwk::IDataAbilityObserver> &));
    MOCK_METHOD0(Release, bool());
    static inline std::shared_ptr<DataShareHelperMock> proxy_ = nullptr;
};
} // namespace OHOS
} // namespace DataShare
#endif // DATASHARE_HELPER_MOCK_H