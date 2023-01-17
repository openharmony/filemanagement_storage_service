/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
static const char *PATH_DEV_BLOCK = "/dev/block";
static const char *PATH_MNT = "/mnt";
static const char *PATH_DATA = "/data";

enum SizeType {
    TOTAL,
    FREE,
    USED
};
class StorageTotalStatusService : public NoCopyable {
    DECLARE_DELAYED_SINGLETON(StorageTotalStatusService);

public:
    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetFreeSize(int64_t &freeSize);
private:
    int32_t GetSizeOfPath(const char *path, int32_t type, int64_t &size);
    const std::vector<std::string> mountDir = {"/debug_ramdisk", "/patch_hw",
        "/metadata", "/", "/cust", "/hw_product", "/odm", "/preas", "/vendor",
        "/vendor/modem/modem_driver", "/data"};
};
} // StorageManager
} // OHOS

#endif // OHOS_STORAGE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H

