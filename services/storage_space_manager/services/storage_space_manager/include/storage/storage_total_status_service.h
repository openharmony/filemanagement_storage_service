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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H

#include <nocopyable.h>
#include <singleton.h>

namespace OHOS {
namespace StorageSpaceManager {
static const char *PATH_DATA = "/data";
static const char *PATH_ROOT = "/";

enum class StorageStatType : int32_t {
    TOTAL,
    FREE,
    USED
};

class StorageTotalStatusService : public NoCopyable {
public:
    static StorageTotalStatusService &GetInstance()
    {
        static StorageTotalStatusService instance;
        return instance;
    }

    int32_t GetSystemSize(int64_t &systemSize);
    int32_t GetTotalSize(int64_t &totalSize);
    int32_t GetFreeSize(int64_t &freeSize);
    int32_t GetTotalInodes(int64_t &totalInodes);
    int32_t GetFreeInodes(int64_t &freeInodes);
    int64_t GetRoundSize(int64_t size);

private:
    StorageTotalStatusService() = default;
    ~StorageTotalStatusService() = default;

    int32_t GetSizeOfPath(const char *path, int32_t type, int64_t &size);
    int32_t GetInodeOfPath(const char *path, int32_t type, int64_t &inodeCnt);
};

} // namespace StorageSpaceManager
} // namespace OHOS

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_STORAGE_TOTAL_STATUS_SERVICE_H
