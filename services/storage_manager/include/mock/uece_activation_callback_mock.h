/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_UECE_ACTIVATION_CALLBACK_MOCK_H
#define OHOS_STORAGE_MANAGER_UECE_ACTIVATION_CALLBACK_MOCK_H

#include <gmock/gmock.h>

#include "uece_activation_callback_stub.h"

namespace OHOS {
namespace StorageManager {
class UeceActivationCallbackMock : public UeceActivationCallbackStub {
public:
    MOCK_METHOD4(OnEl5Activation, ErrCode(int32_t, int32_t, bool, int32_t &));
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_UECE_ACTIVATION_CALLBACK_MOCK_H