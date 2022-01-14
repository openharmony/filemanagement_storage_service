/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H
#define OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H

#include <vector>
#include <nocopyable.h>
#include <singleton.h>
#include <iostream>

namespace OHOS {
namespace StorageManager {
class StorageTotalStatusService : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageTotalStatusService);
private:
    const std::vector<std::string> mountDir = {"/debug_ramdisk", "/patch_hw",
        "/metadata", "/", "/cust", "/hw_product", "/odm", "/preas", "/vendor",
        "/vendor/modem/modem_driver", "/data"};
public:
    int64_t GetFreeSizeOfVolume(std::string volumeUuid);
    int64_t GetTotalSizeOfVolume(std::string volumeUuid);
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H

