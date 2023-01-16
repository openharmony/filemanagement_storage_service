/*
* Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_VOLUME_STORAGE_STATUS_SERVICE_H
#define OHOS_STORAGE_MANAGER_VOLUME_STORAGE_STATUS_SERVICE_H

#include <nocopyable.h>
#include <singleton.h>

namespace OHOS {
namespace StorageManager {
class VolumeStorageStatusService : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(VolumeStorageStatusService);

public:
    int32_t GetFreeSizeOfVolume(std::string volumeUuid, int64_t &freeSize);
    int32_t GetTotalSizeOfVolume(std::string volumeUuid, int64_t &totalSize);
private:
    std::string GetVolumePath(std::string volumeUuid);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_VOLUME_STORAGE_STATUS_SERVICE_H

