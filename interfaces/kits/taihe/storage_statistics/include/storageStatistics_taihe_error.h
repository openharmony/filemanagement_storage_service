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

#ifndef OHOS_STORAGESTATISTICS_TAIHE_ERROR_H
#define OHOS_STORAGESTATISTICS_TAIHE_ERROR_H

#include <map>
#include <string>
#include "storage_service_errno.h"
#include "taihe/runtime.hpp"


namespace OHOS {

struct StorageTaiheErrorInfo {
    int errorCode;
    std::string errorMsg;
};

const std::map<int, StorageTaiheErrorInfo> STORAGE_TAIHE_ERR_MAP {
    { E_PERMISSION, { 201, "Permission verification failed" } },
    { E_PERMISSION_SYS, { 202, "The caller is not a system application" } },
    { E_PARAMS, { 401, "The input parameter is invalid" } },
    { E_DEVICENOTSUPPORT, { 801, "The device doesn't support this api" } },
    { E_OSNOTSUPPORT, { 901, "The os doesn't support this api" } },
    { E_IPCSS, { 13600001, "IPC error. Possible causes: 1.IPC failed or timed out. 2.Failed to load the service" } },
    { E_SUPPORTEDFS, { 13600002, "Not supported filesystem" } },
    { E_MOUNT_ERR, { 13600003, "Failed to mount" } },
    { E_UNMOUNT, { 13600004, "Failed to unmount" } },
    { E_VOLUMESTATE, { 13600005, "Incorrect volume state" } },
    { E_PREPARE, { 13600006, "Prepare directory or node error" } },
    { E_DELETE, { 13600007, "Delete directory or node error" } },
    { E_NOOBJECT, { 13600008, "No such object" } },
    { E_OUTOFRANGE, { 13600009, "User id out of range" } }
};

class StorageTaiheError {
public:
    static int32_t Convert(const int errCode);
    static std::string ToMessage(const int errCode);
    static void SetStorageTaiheError(const int error_code);
};

} // namespace OHOS

#endif // OHOS_STORAGESTATISTICS_TAIHE_ERROR_H
