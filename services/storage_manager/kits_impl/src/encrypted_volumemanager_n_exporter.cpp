/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "encrypted_volumemanager_n_exporter.h"

#include "n_async/n_async_work_callback.h"
#include "n_async/n_async_work_promise.h"
#include "n_func_arg.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"

using namespace OHOS::StorageManager;
using namespace OHOS::FileManagement::LibN;

namespace OHOS {
namespace StorageManager {
namespace ModuleEncryptedVolumeManager {
const std::string FEATURE_STR = "EncryptedVolumeManager.";

napi_value Encrypt(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    std::unique_ptr<char []> pazzword;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, pazzword, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string volumeIdString(volumeId.get());
    std::string pazzwordString(pazzword.get());
    auto cbExec = [volumeIdString, pazzwordString]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            Encrypt(volumeIdString, pazzwordString);
        if (result != E_OK) {
            return NError(Convert2JsErrNum(result));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "encrypt";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetCryptProgressById(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string volumeIdString(volumeId.get());
    auto progress = std::make_shared<int32_t>();
    auto cbExec = [volumeIdString, progress]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            GetCryptProgressById(volumeIdString, *progress);
        if (result != E_OK) {
            return NError(Convert2JsErrNum(result));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [progress](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt32(env, *progress) };
    };

    std::string procedureName = "getCryptProgressById";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleEncryptedVolumeManager
} // namespace StorageManager
} // namespace OHOS
