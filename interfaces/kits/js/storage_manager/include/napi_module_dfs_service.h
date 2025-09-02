/*
* Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef NAPI_MODULE_DFS_SERVICE_H
#define NAPI_MODULE_DFS_SERVICE_H
#ifdef HMDFS_FILE_MANAGER
#include "distributed_file_daemon_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace StorageManager {
class DfsService {
public:
    static napi_value IsSameAccountDevice(napi_env env, napi_callback_info info);
    static napi_value GetDfsSwitchStatus(napi_env env, napi_callback_info info);
    static napi_value UpdateDfsSwitchStatus(napi_env env, napi_callback_info info);
    static napi_value ConnectDfs(napi_env env, napi_callback_info info);
    static napi_value DisconnectDfs(napi_env env, napi_callback_info info);
    static napi_value GetConnectedDeviceList(napi_env env, napi_callback_info info);
    static napi_value DeviceOnline(napi_env env, napi_callback_info info);
    static napi_value DeviceOffline(napi_env env, napi_callback_info info);
};

struct DfsStatusChange {
    std::string networkId;
    std::string path;
    int32_t type = 0;
    int32_t status = 0;
};
} // namespace StorageManager
} // namespace OHOS
#endif
#endif // NAPI_MODULE_DFS_SERVICE_H
