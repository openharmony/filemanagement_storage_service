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

#include "mock/disk_manager_client_mock.h"

namespace OHOS {
namespace DiskManager {

int32_t DiskManagerClient::GetFreeSizeOfVolume(const std::string &volumeUuid, int64_t &freeSize)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetFreeSizeOfVolume(volumeUuid, freeSize);
}

int32_t DiskManagerClient::GetTotalSizeOfVolume(const std::string &volumeUuid, int64_t &totalSize)
{
    return IDiskManagerClientMock::diskManagerClientMock->GetTotalSizeOfVolume(volumeUuid, totalSize);
}

} // DiskManager
} // OHOS
