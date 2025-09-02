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

#include "napi_module_dfs_service.h"
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>
#include <mutex>

#include "n_async_work_promise.h"
#include "n_class.h"
#include "n_func_arg.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_module_dfs_listener.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"

using namespace OHOS::StorageManager;
using namespace OHOS::FileManagement::LibN;

namespace OHOS {
namespace StorageManager {

static constexpr int32_t NAPI_NUMBER_SIZE = 5;
static constexpr size_t DEVICE_ONLINE_ARG_COUNT = 3;
static constexpr size_t DEVICE_OFFLINE_ARG_COUNT = 2;
static constexpr int32_t FIRST_ARGUMENT_INDEX = 0;
static constexpr int32_t SECOND_ARGUMENT_INDEX = 1;
static constexpr int32_t THIRD_ARGUMENT_INDEX = 2;
static constexpr int32_t SINGLE_ARGUMENT_COUNT = 1;
static constexpr int32_t NO_ARGUMENT_COUNT = 0;

static std::map<std::string, sptr<OHOS::Storage::DistributedFile::IFileDfsListener>> dfsListeners;
static std::mutex g_dfsListenersMutex;
static napi_status DealNapiStrValue(napi_env env, const napi_value napiValue, std::string &result)
{
    HILOGD("Start DealNapiStrValue");
    size_t bufferSize = 0;
    napi_status status = napi_get_value_string_utf8(env, napiValue, nullptr, 0, &bufferSize);
    if (status != napi_ok) {
        HILOGE("Cannot get buffer size, status: %{public}d", status);
        return status;
    }
    
    if (bufferSize == 0) {
        result = "";
        return napi_ok;
    }
    
    std::string buffer(bufferSize + 1, '\0');
    status = napi_get_value_string_utf8(env, napiValue, buffer.data(), bufferSize + 1, &bufferSize);
    if (status != napi_ok) {
        HILOGE("Cannot get buffer value, status: %{public}d", status);
        return status;
    }
    
    result = buffer.substr(0, bufferSize);
    return status;
}

static napi_value CreateDfsDeviceInfosAsNapiArray(napi_env env,
                                                  std::vector<Storage::DistributedFile::DfsDeviceInfo> dfsDeviceInfos)
{
    napi_value dfsDeviceObjArray = nullptr;
    napi_status status = napi_create_array(env, &dfsDeviceObjArray);
    if (status != napi_ok || dfsDeviceObjArray == nullptr) {
        HILOGE("Failed to create array, status: %{public}d", status);
        return nullptr;
    }

    const size_t dfsDeviceInfoCount = dfsDeviceInfos.size();
    for (size_t i = 0; i < dfsDeviceInfoCount; i++) {
        napi_value dfsDeviceInfo = nullptr;
        status = napi_create_object(env, &dfsDeviceInfo);
        if (status != napi_ok || dfsDeviceInfo == nullptr) {
            HILOGE("Failed to create object, status: %{public}d", status);
            return nullptr;
        }
        
        napi_value networkId = nullptr;
        napi_value path = nullptr;

        std::string deviceNetworkId = dfsDeviceInfos[i].networkId_;
        std::string devicePath = dfsDeviceInfos[i].path_;
        status = napi_create_string_utf8(env, deviceNetworkId.c_str(), NAPI_AUTO_LENGTH, &networkId);
        if (status != napi_ok || networkId == nullptr) {
            HILOGE("Failed to create networkId string, status: %{public}d", status);
            return nullptr;
        }
        
        status = napi_create_string_utf8(env, devicePath.c_str(), NAPI_AUTO_LENGTH, &path);
        if (status != napi_ok || path == nullptr) {
            HILOGE("Failed to create path string, status: %{public}d", status);
            return nullptr;
        }
        
        status = napi_set_named_property(env, dfsDeviceInfo, "networkId", networkId);
        if (status != napi_ok) {
            HILOGE("Failed to set networkId property, status: %{public}d", status);
            return nullptr;
        }
        
        status = napi_set_named_property(env, dfsDeviceInfo, "path", path);
        if (status != napi_ok) {
            HILOGE("Failed to set path property, status: %{public}d", status);
            return nullptr;
        }
        
        status = napi_set_element(env, dfsDeviceObjArray, i, dfsDeviceInfo);
        if (status != napi_ok) {
            HILOGE("Failed to set array element, status: %{public}d", status);
            return nullptr;
        }
    }
    return dfsDeviceObjArray;
}

napi_value StorageManager::DfsService::IsSameAccountDevice(napi_env env, napi_callback_info info)
{
    HILOGD("enter IsSameAccountDevice");

    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(SINGLE_ARGUMENT_COUNT, SINGLE_ARGUMENT_COUNT)) {
        HILOGE("Number of arguments mismatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char[]> networkid;
    tie(succ, networkid, std::ignore) = NVal(env, funcArg[FIRST_ARGUMENT_INDEX]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::string networkId(networkid.get());
    bool isSameAccount = false;
    int32_t ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().IsSameAccountDevice(
        networkId, isSameAccount);
    if (ret != E_OK) {
        HILOGE("FileManagerDFS IsSameAccountDevice failed, ret = %{public}d.", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    napi_value resIsSameAccount = nullptr;
    napi_get_boolean(env, isSameAccount, &resIsSameAccount);
    return resIsSameAccount;
}

napi_value StorageManager::DfsService::GetDfsSwitchStatus(napi_env env, napi_callback_info info)
{
    HILOGD("enter GetDfsSwitchStatus");

    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(SINGLE_ARGUMENT_COUNT, SINGLE_ARGUMENT_COUNT)) {
        HILOGE("Number of arguments mismatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> networkIdStr;
    std::tie(succ, networkIdStr, std::ignore) = NVal(env, funcArg[FIRST_ARGUMENT_INDEX]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    
    std::string networkId(networkIdStr.get());
    int32_t switchStatus = 0;
    int32_t ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().GetDfsSwitchStatus(
        networkId, switchStatus);
    if (ret != E_OK) {
        HILOGE("GetDfsSwitchStatus failed, ret = %{public}d.", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    
    napi_value resSwitchStatus = nullptr;
    napi_create_int32(env, switchStatus, &resSwitchStatus);
    return resSwitchStatus;
}

napi_value StorageManager::DfsService::UpdateDfsSwitchStatus(napi_env env, napi_callback_info info)
{
    HILOGI("enter UpdateDfsSwitchStatus");

        NFuncArg funcArg(env, info);
        if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::ONE)) {
            HILOGE("Number of arguments mismatched");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        auto [bRet, argStatus] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt32(NAPI_NUMBER_SIZE);
        if (!bRet) {
            HILOGE("Get query count of recent dir failed");
            NError(E_PARAMS).ThrowErr(env);
            return nullptr;
        }
        int32_t status = argStatus;
        auto cbExec = [status]() -> NError {
            int32_t errNum =
                Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().UpdateDfsSwitchStatus(status);
            if (errNum != E_OK) {
                return NError(Convert2JsErrNum(errNum));
            }
            return NError(ERRNO_NOERR);
        };
        auto cbComplete = [](napi_env env, NError err) -> NVal {
            if (err) {
                return {env, err.GetNapiErr(env)};
            }
            return NVal::CreateUndefined(env);
        };
        
        NVal thisVar(env, funcArg.GetThisVar());
        return NAsyncWorkPromise(env, thisVar).Schedule("UpdateDfsSwitchStatus", cbExec, cbComplete).val_;
}

napi_value StorageManager::DfsService::ConnectDfs(napi_env env, napi_callback_info info)
{
    HILOGD("enter ConnectDfs");

    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(SINGLE_ARGUMENT_COUNT, SINGLE_ARGUMENT_COUNT)) {
        HILOGE("Number of arguments mismatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> networkIdStr;
    std::tie(succ, networkIdStr, std::ignore) = NVal(env, funcArg[FIRST_ARGUMENT_INDEX]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::string networkId(networkIdStr.get());

    auto cbExec = [networkId]() -> NError {
        int32_t errNum = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().OpenP2PConnectionEx(
            networkId, nullptr);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule("ConnectDfs", cbExec, cbComplete).val_;
}

napi_value StorageManager::DfsService::DisconnectDfs(napi_env env, napi_callback_info info)
{
    HILOGD("enter DisconnectDfs");

    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(SINGLE_ARGUMENT_COUNT, SINGLE_ARGUMENT_COUNT)) {
        HILOGE("Number of arguments mismatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char[]> networkIdStr;
    std::tie(succ, networkIdStr, std::ignore) = NVal(env, funcArg[FIRST_ARGUMENT_INDEX]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::string networkId(networkIdStr.get());

    auto cbExec = [networkId]() -> NError {
        int32_t errNum =
            Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().CloseP2PConnectionEx(networkId);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule("DisconnectDfs", cbExec, cbComplete).val_;
}

napi_value StorageManager::DfsService::GetConnectedDeviceList(napi_env env, napi_callback_info info)
{
    HILOGD("enter GetConnectedDeviceList");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NO_ARGUMENT_COUNT, NO_ARGUMENT_COUNT)) {
        HILOGE("Number of arguments mismatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    
    auto dfsDeviceInfos = std::make_shared<std::vector<Storage::DistributedFile::DfsDeviceInfo>>();
    int32_t ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().GetConnectedDeviceList(
        *dfsDeviceInfos);
    if (ret != E_OK) {
        HILOGE("FileManagerDFS GetConnectedDeviceList failed, ret = %{public}d.", ret);
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    napi_value reDfsDeviceInfos = CreateDfsDeviceInfosAsNapiArray(env, *dfsDeviceInfos);
    HILOGD("size is %{public}zu", dfsDeviceInfos->size());

    return reDfsDeviceInfos;
}

napi_value StorageManager::DfsService::DeviceOnline(napi_env env, napi_callback_info info)
{
    napi_value args[DEVICE_ONLINE_ARG_COUNT];
    size_t argc = DEVICE_ONLINE_ARG_COUNT;
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != DEVICE_ONLINE_ARG_COUNT) {
        HILOGE("Cannot get num of func args for %{public}d", status);
        return nullptr;
    }

    std::string instanceId;
    status = DealNapiStrValue(env, args[SECOND_ARGUMENT_INDEX], instanceId);
    if (status != napi_ok) {
        HILOGE("Cannot get instanceId for %{public}d", status);
        return nullptr;
    }
    HILOGD("DeviceOnline instanceId:%{public}s", instanceId.c_str());

    OHOS::StorageManager::NapiFileDfsListener *listenerObj =
        new OHOS::StorageManager::NapiFileDfsListener(env, args[THIRD_ARGUMENT_INDEX]);

    sptr<OHOS::StorageManager::NapiFileDfsListener> listener = listenerObj;

    {
        std::lock_guard<std::mutex> lock(g_dfsListenersMutex);
        dfsListeners[instanceId] = listener;
    }

    int32_t ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().RegisterFileDfsListener(
        instanceId, listener);
    if (ret != 0) {
        HILOGE("RegisterFileDfsListener failed, ret is %{public}d", ret);
        std::lock_guard<std::mutex> lock(g_dfsListenersMutex);
        dfsListeners.erase(instanceId);
        return nullptr;
    }
    return nullptr;
}

napi_value StorageManager::DfsService::DeviceOffline(napi_env env, napi_callback_info info)
{
    napi_value args[DEVICE_OFFLINE_ARG_COUNT];
    size_t argc = DEVICE_OFFLINE_ARG_COUNT;
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc != DEVICE_OFFLINE_ARG_COUNT) {
        HILOGE("Cannot get num of func args for %{public}d", status);
        return nullptr;
    }

    std::string instanceId;
    DealNapiStrValue(env, args[SECOND_ARGUMENT_INDEX], instanceId);
    
    {
        std::lock_guard<std::mutex> lock(g_dfsListenersMutex);
        if (dfsListeners.find(instanceId) == dfsListeners.end()) {
            HILOGE("The current device does not have a listening instance");
            return nullptr;
        }
    }
    
    int32_t ret = Storage::DistributedFile::DistributedFileDaemonManager::GetInstance().UnregisterFileDfsListener(
        instanceId);
    if (ret != 0) {
        HILOGE("UnregisterFileDfsListener failed, ret is %{public}d", ret);
        return nullptr;
    }
    
    {
        std::lock_guard<std::mutex> lock(g_dfsListenersMutex);
        dfsListeners.erase(instanceId);
    }
    return nullptr;
}
} // namespace StorageManager
} // namespace OHOS