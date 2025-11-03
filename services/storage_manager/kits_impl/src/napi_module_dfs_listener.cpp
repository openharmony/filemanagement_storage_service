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

#include "napi_module_dfs_listener.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "distributed_file_daemon_manager.h"
#include "filemgmt_libhilog.h"

namespace OHOS {
namespace StorageManager {
static napi_status SetStringProperty(napi_env env, napi_value object,
    const char* propertyName, const std::string& value)
{
    napi_value stringValue = nullptr;
    napi_status napiStatus = napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &stringValue);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to create string for property: %{public}s", propertyName);
        return napiStatus;
    }

    napiStatus = napi_set_named_property(env, object, propertyName, stringValue);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set string property: %{public}s", propertyName);
        return napiStatus;
    }

    return napi_ok;
}

static napi_status SetInt32Property(napi_env env, napi_value object,
    const char* propertyName, int32_t value)
{
    napi_value intValue = nullptr;
    napi_status napiStatus = napi_create_int32(env, value, &intValue);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to create int32 for property: %{public}s", propertyName);
        return napiStatus;
    }

    napiStatus = napi_set_named_property(env, object, propertyName, intValue);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set int32 property: %{public}s", propertyName);
        return napiStatus;
    }

    return napi_ok;
}

static napi_status CreateEventDataObject(napi_env env, napi_value &target,
    const NapiFileDfsListener::StatusEventInfo &eventInfo)
{
    napi_status napiStatus = napi_create_object(env, &target);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to create object for event data, status: %{public}d", napiStatus);
        return napiStatus;
    }

    napiStatus = SetStringProperty(env, target, "networkId", eventInfo.networkId);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set networkId property, status: %{public}d", napiStatus);
        return napiStatus;
    }

    napiStatus = SetStringProperty(env, target, "path", eventInfo.path);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set path property, status: %{public}d", napiStatus);
        return napiStatus;
    }

    napiStatus = SetInt32Property(env, target, "type", eventInfo.type);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set type property, status: %{public}d, type: %{public}d", napiStatus, eventInfo.type);
        return napiStatus;
    }

    napiStatus = SetInt32Property(env, target, "status", eventInfo.status);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to set status property, status: %{public}d, statusValue: %{public}d", napiStatus,
               eventInfo.status);
        return napiStatus;
    }

    return napi_ok;
}

static napi_status CallJsCallback(napi_env env, napi_ref callbackRef_, napi_value eventData)
{
    napi_value jsCallback = nullptr;
    napi_status napiStatus = napi_get_reference_value(env, callbackRef_, &jsCallback);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to get callback reference, status: %{public}d", napiStatus);
        return napiStatus;
    }
    
    if (jsCallback == nullptr) {
        HILOGE("Callback is null");
        return napi_generic_failure;
    }

    napi_value args[1] = {eventData};
    napiStatus = napi_call_function(env, nullptr, jsCallback, 1, args, nullptr);
    return napiStatus;
}

static void HandleStatusEvent(napi_env env, napi_ref callbackRef_,
    const NapiFileDfsListener::StatusEventInfo &eventInfo)
{
    napi_handle_scope scope = nullptr;
    napi_status napiStatus = napi_open_handle_scope(env, &scope);
    if (napiStatus != napi_ok || scope == nullptr) {
        HILOGE("Failed to open handle scope, status: %{public}d", napiStatus);
        return;
    }

    napi_value target = nullptr;
    napiStatus = CreateEventDataObject(env, target, eventInfo);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to create event data object, status: %{public}d", napiStatus);
        napi_close_handle_scope(env, scope);
        return;
    }

    napiStatus = CallJsCallback(env, callbackRef_, target);
    if (napiStatus != napi_ok) {
        HILOGE("Failed to call JavaScript callback, status: %{public}d", napiStatus);
    }
    
    napi_close_handle_scope(env, scope);
}

void NapiFileDfsListener::OnStatus(const std::string &networkId, int32_t status, const std::string &path, int32_t type)
{
    HILOGI("enter OnStatus");
    if (networkId.empty() || path.empty()) {
        HILOGE("networkId or path is empty");
        return;
    }

    NapiFileDfsListener::StatusEventInfo eventInfo;
    eventInfo.networkId = networkId;
    eventInfo.status = status;
    eventInfo.path = path;
    eventInfo.type = type;
    napi_status ret = napi_send_event(
        env_,
        [this, eventInfo]() mutable {
            HandleStatusEvent(env_, callbackRef_, eventInfo);
        },
        napi_eprio_immediate);
    if (ret != napi_ok) {
        HILOGE("Failed to send event to JavaScript thread, status: %{public}d", ret);
    }
}
} // namespace StorageManager
} // namespace OHOS