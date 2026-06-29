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

#include "mock/storage_manager_client_mock.h"

namespace OHOS {
namespace StorageDaemon {
int32_t StorageManagerClient::GetClient()
{
    return 0;
}

int32_t StorageManagerClient::NotifyCreateBundleDataDirWithEl(uint32_t userId, uint8_t elx)
{
    return 0;
}

int32_t StorageManagerClient::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    if (IStorageManagerClientMock::iStorageManagerClientMock_ == nullptr) {
        return -1;
    }
    return IStorageManagerClientMock::iStorageManagerClientMock_->QueryActiveOsAccountIds(ids);
}

int32_t StorageManagerClient::IsOsAccountExists(unsigned int userId, bool &isOsAccountExists)
{
    if (IStorageManagerClientMock::iStorageManagerClientMock_ == nullptr) {
        return -1;
    }
    return IStorageManagerClientMock::iStorageManagerClientMock_->IsOsAccountExists(userId, isOsAccountExists);
}
} // StorageDaemon
} // OHOS
