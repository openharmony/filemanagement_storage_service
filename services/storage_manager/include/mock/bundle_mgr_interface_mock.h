/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_I_BUNDLEMGR_H
#define OHOS_STORAGE_I_BUNDLEMGR_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bundle_mgr_interface.h"
#include "iremote_broker.h"
#include "iremote_stub.h"

namespace OHOS::StorageManager {
class BundleMgrMock : public IRemoteStub<AppExecFwk::IBundleMgr> {
public:
    int code_;
    BundleMgrMock() : code_(0) {}
    virtual ~BundleMgrMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
};
} // namespace OHOS::StorageManager

#endif // OHOS_STORAGE_I_BUNDLEMGR_H