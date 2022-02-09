/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "volumemanager_n_exporter.h"
#include "storage_manager_connect.h"

#include <tuple>

#include "common/napi/n_class.h"
#include "common/napi/n_func_arg.h"
#include "common/napi/n_val.h"
#include "common/uni_error.h"
#include "common/napi/n_async/n_async_work_callback.h"
#include "common/napi/n_async/n_async_work_promise.h"

#include "storage_service_log.h"

#include <singleton.h>

using namespace OHOS::StorageManager;
using namespace OHOS::DistributedFS;

namespace OHOS {
namespace StorageManager {
namespace ModuleVolumeManager {
napi_value GetAllVolumes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 0-1");
        return nullptr;
    }
    auto volumeInfo = std::make_shared<std::vector<VolumeExternal>>();
    auto cbExec = [volumeInfo](napi_env env) -> UniError {
         *volumeInfo = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetAllVolumes();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value volumeInfoArray = nullptr;
        napi_status status = napi_create_array(env, &volumeInfoArray);
        if (status != napi_ok) {
            return { env, UniError(status).GetNapiErr(env) };
        }
        for (size_t i = 0; i < (*volumeInfo).size(); i++) {
            NVal volumeInfoObject = NVal::CreateObject(env);
            volumeInfoObject.AddProp("id", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetId()).val_);
            volumeInfoObject.AddProp("uuid", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetUuid()).val_);
            volumeInfoObject.AddProp("description",
                NVal::CreateUTF8String(env, (*volumeInfo)[i].GetDescription()).val_);
            volumeInfoObject.AddProp("removeAble", NVal::CreateBool(env, (bool)true).val_);
            volumeInfoObject.AddProp("state", NVal::CreateInt32(env, (*volumeInfo)[i].GetState()).val_);
            volumeInfoObject.AddProp("path", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetPath()).val_);
            status = napi_set_element(env, volumeInfoArray, i, volumeInfoObject.val_);
            if (status != napi_ok) {
                return { env, UniError(status).GetNapiErr(env) };
            }
        }
        return { NVal(env, volumeInfoArray) };
    };
    std::string procedureName = "GetAllVolumes";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}


napi_value Mount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid volumeId");
        return nullptr;
    }

    auto result = std::make_shared<bool>();
    std::string volumeIdString(volumeId.get());
    auto cbExec = [volumeIdString, result](napi_env env) -> UniError {
        *result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Mount(volumeIdString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateBool(env, *result) };
    };

    std::string procedureName = "Mount";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value Unmount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid volumeId");
        return nullptr;
    }

    auto result = std::make_shared<bool>();
    std::string volumeIdString(volumeId.get());
    auto cbExec = [volumeIdString, result](napi_env env) -> UniError {
        *result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Unmount(volumeIdString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateBool(env, *result) };
    };

    std::string procedureName = "Unmount";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}
} // namespace ModuleVolumeManager
} // namespace StorageManager
} // namespace OHOS