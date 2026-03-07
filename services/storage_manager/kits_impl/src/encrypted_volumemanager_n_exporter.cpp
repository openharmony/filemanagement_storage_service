/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <optional>

using namespace OHOS::StorageManager;
using namespace OHOS::FileManagement::LibN;

namespace OHOS {
namespace StorageManager {
namespace ModuleEncryptedVolumeManager {
const std::string FEATURE_STR = "EncryptedVolumeManager.";
constexpr size_t MAX_STRING_LENGTH = 256;

std::optional<std::string> ParseStringArgSafe(napi_env env, NFuncArg& funcArg, NARG_POS pos)
{
    bool succ = false;
    std::unique_ptr<char[]> buf;
    tie(succ, buf, std::ignore) = NVal(env, funcArg[(int)pos]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return std::nullopt;
    }
    
    std::string result = buf.get();
    if (result.empty() || result.size() >= MAX_STRING_LENGTH) {
        NError(E_PARAMS).ThrowErr(env);
        return std::nullopt;
    }
    
    return result;
}

napi_value Encrypt(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    if (!volIdStr.has_value() || !pwdStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, pwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            Encrypt(*volIdStr, *pwdStr);
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
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    if (!volIdStr.has_value()) {
        return nullptr;
    }
    auto progress = std::make_shared<int32_t>();
    auto cbExec = [volIdStr, progress]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            GetCryptProgressById(*volIdStr, *progress);
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

napi_value GetCryptUuidById(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    if (!volIdStr.has_value()) {
        return nullptr;
    }
    auto uuid = std::make_shared<std::string>();
    auto cbExec = [volIdStr, uuid]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            GetCryptUuidById(*volIdStr, *uuid);
        if (result != E_OK) {
            return NError(Convert2JsErrNum(result));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [uuid](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUTF8String(env, *uuid) };
    };
    std::string procedureName = "getCryptUuidById";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value BindRecoverKeyToPasswd(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::THREE, (int)NARG_CNT::FOUR)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    auto reKeyStr = ParseStringArgSafe(env, funcArg, NARG_POS::THIRD);
    if (!volIdStr.has_value() || !pwdStr.has_value() || !reKeyStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, pwdStr, reKeyStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            BindRecoverKeyToPasswd(*volIdStr, *pwdStr, *reKeyStr);
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
    std::string procedureName = "bindRecoverKeyToPasswd";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value UpdateCryptPasswd(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::THREE, (int)NARG_CNT::FOUR)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    auto nPwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::THIRD);
    if (!volIdStr.has_value() || !pwdStr.has_value() || !nPwdStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, pwdStr, nPwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            UpdateCryptPasswd(*volIdStr, *pwdStr, *nPwdStr);
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
    std::string procedureName = "updateCryptPasswd";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value ResetCryptPasswd(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::THREE, (int)NARG_CNT::FOUR)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto reKeyStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    auto nPwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::THIRD);
    if (!volIdStr.has_value() || !reKeyStr.has_value() || !nPwdStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, reKeyStr, nPwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            ResetCryptPasswd(*volIdStr, *reKeyStr, *nPwdStr);
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
    std::string procedureName = "resetCryptPasswd";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value VerifyCryptPasswd(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    if (!volIdStr.has_value() || !pwdStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, pwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            VerifyCryptPasswd(*volIdStr, *pwdStr);
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
    std::string procedureName = "verifyCryptPasswd";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Unlock(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    if (!volIdStr.has_value() || !pwdStr.has_value()) {
        return nullptr;
    }

    auto cbExec = [volIdStr, pwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            Unlock(*volIdStr, *pwdStr);
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
    std::string procedureName = "unlock";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Decrypt(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volIdStr = ParseStringArgSafe(env, funcArg, NARG_POS::FIRST);
    auto pwdStr = ParseStringArgSafe(env, funcArg, NARG_POS::SECOND);
    if (!volIdStr.has_value() || !pwdStr.has_value()) {
        return nullptr;
    }
    auto cbExec = [volIdStr, pwdStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->
            Decrypt(*volIdStr, *pwdStr);
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
    std::string procedureName = "decrypt";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

} // namespace ModuleEncryptedVolumeManager
} // namespace StorageManager
} // namespace OHOS