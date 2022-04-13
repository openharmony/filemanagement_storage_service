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

#include "storage_statistics_n_exporter.h"

#include <tuple>
#include <singleton.h>

#include "common/napi/n_class.h"
#include "common/napi/n_func_arg.h"
#include "common/napi/n_val.h"
#include "common/uni_error.h"
#include "common/napi/n_async/n_async_work_callback.h"
#include "common/napi/n_async/n_async_work_promise.h"

#include "storage_statistics_napi.h"
#include "storage_manager_connect.h"
#include "storage_service_log.h"

using namespace OHOS::DistributedFS;

namespace OHOS {
namespace StorageManager {
napi_value GetTotalSizeOfVolume(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid uuid");
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, resultSize](napi_env env) -> UniError {
        *resultSize = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetTotalSizeOfVolume(uuidString);
        return UniError(ERRNO_NOERR);
    };

    auto cbComplete = [resultSize](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateInt64(env, *resultSize);
    };

    std::string procedureName = "GetTotalSizeOfVolume";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}


napi_value GetFreeSizeOfVolume(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid uuid");
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, resultSize](napi_env env) -> UniError {
        *resultSize = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetFreeSizeOfVolume(uuidString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    std::string procedureName = "getFreeSizeOfVolume";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> name;
    tie(succ, name, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid name");
        return nullptr;
    }
    auto bundleStats = std::make_shared<std::vector<int64_t>>();
    std::string nameString(name.get());
    auto cbExec = [nameString, bundleStats](napi_env env) -> UniError {
        *bundleStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetBundleStats(nameString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [bundleStats](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal bundleObject = NVal::CreateObject(env);
        if ((*bundleStats).size() != 3) { // 3 is the fixed size of bundle stats.
            UniError(EINVAL).ThrowErr(env, "vector size error");
            return bundleObject;
        }
        bundleObject.AddProp("appSize", NVal::CreateInt64(env, (*bundleStats)[0]).val_); // 0 is the index of app size
        bundleObject.AddProp("cacheSize", NVal::CreateInt64(env,
            (*bundleStats)[1]).val_); // 1 is the index of cache size
        bundleObject.AddProp("dataSize", NVal::CreateInt64(env, (*bundleStats)[2]).val_); // 2 is the index of data size
        return bundleObject;
    };
    std::string procedureName = "GetBundleStats";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 0-1");
        return nullptr;
    }

    auto bundleStats = std::make_shared<std::vector<int64_t>>();
    auto cbExec = [bundleStats](napi_env env) -> UniError {
        *bundleStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetCurrentBundleStats();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [bundleStats](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal bundleObject = NVal::CreateObject(env);
        if ((*bundleStats).size() != 3) {
            UniError(EINVAL).ThrowErr(env, "vector size error");
            return bundleObject;
        }
        bundleObject.AddProp("appSize", NVal::CreateInt64(env, (*bundleStats)[0]).val_);
        bundleObject.AddProp("cacheSize", NVal::CreateInt64(env,
            (*bundleStats)[1]).val_);
        bundleObject.AddProp("dataSize", NVal::CreateInt64(env, (*bundleStats)[2]).val_);
        return bundleObject;
    };
    std::string procedureName = "GetCurrentBundleStats";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetSystemSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize](napi_env env) -> UniError {
        *resultSize = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetSystemSize();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    std::string procedureName = "GetSystemSize";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetUserStorageStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 1-2");
        return nullptr;
    }

    bool succ = false;
    int64_t userId;
    std::tie(succ, userId) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToInt64();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid userId");
        return nullptr;
    }

    auto totalStats = std::make_shared<std::vector<int64_t>>();
    auto cbExec = [userId, totalStats](napi_env env) -> UniError {
        *totalStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetUserStorageStats(userId);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [totalStats](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal totalObject = NVal::CreateObject(env);
        if ((*totalStats).size() != 6) {
            UniError(EINVAL).ThrowErr(env, "vector size error");
            return totalObject;
        }
        totalObject.AddProp("total", NVal::CreateInt64(env, (*totalStats)[0]).val_);
        totalObject.AddProp("audio", NVal::CreateInt64(env, (*totalStats)[1]).val_);
        totalObject.AddProp("video", NVal::CreateInt64(env, (*totalStats)[2]).val_);
        totalObject.AddProp("image", NVal::CreateInt64(env, (*totalStats)[3]).val_);
        totalObject.AddProp("file", NVal::CreateInt64(env, (*totalStats)[4]).val_);
        totalObject.AddProp("app", NVal::CreateInt64(env, (*totalStats)[5]).val_);
        return totalObject;
    };
    std::string procedureName = "GetUserStorageStats";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetTotalSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize](napi_env env) -> UniError {
        *resultSize = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetTotalSize();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    std::string procedureName = "GetTotalSize";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value GetFreeSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize](napi_env env) -> UniError {
        *resultSize = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetFreeSize();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateInt64(env, *resultSize) };
    };

    std::string procedureName = "GetFreeSize";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (int)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
    return NVal::CreateUndefined(env).val_;
}
} // namespace StorageManager
} // namespace OHOS
