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

#include <singleton.h>
#include <tuple>

#include "common/napi/n_async/n_async_work_callback.h"
#include "common/napi/n_async/n_async_work_promise.h"
#include "common/napi/n_class.h"
#include "common/napi/n_func_arg.h"
#include "common/napi/n_val.h"
#include "common/uni_error.h"
#include "storage_manager_connect.h"
#include "storage_service_log.h"
#include "storage_statistics_napi.h"

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
    auto bundleStats = std::make_shared<BundleStats>();
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
        bundleObject.AddProp("appSize", NVal::CreateInt64(env, (bundleStats->appSize_)).val_);
        bundleObject.AddProp("cacheSize", NVal::CreateInt64(env,
            (bundleStats->cacheSize_)).val_);
        bundleObject.AddProp("dataSize", NVal::CreateInt64(env, (bundleStats->dataSize_)).val_);
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
}

napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 0-1");
        return nullptr;
    }

    auto bundleStats = std::make_shared<BundleStats>();
    auto cbExec = [bundleStats](napi_env env) -> UniError {
        *bundleStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetCurrentBundleStats();
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [bundleStats](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal bundleObject = NVal::CreateObject(env);
        bundleObject.AddProp("appSize", NVal::CreateInt64(env, (bundleStats->appSize_)).val_);
        bundleObject.AddProp("cacheSize", NVal::CreateInt64(env,
            (bundleStats->cacheSize_)).val_);
        bundleObject.AddProp("dataSize", NVal::CreateInt64(env, (bundleStats->dataSize_)).val_);
        return bundleObject;
    };
    std::string procedureName = "GetCurrentBundleStats";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetUserStorageStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 0-2");
        return nullptr;
    }
    bool fac = false;
    int64_t userId = -1;
    if (funcArg.GetArgc() >= 1) {
        NVal ui(env, NVal(env, funcArg[(int)NARG_POS::FIRST]).val_);
        if (ui.TypeIs(napi_number)) {
            bool succ = false;
            std::tie(succ, userId) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToInt64();
            if (!succ) {
                UniError(EINVAL).ThrowErr(env, "Invalid userId");
                return nullptr;
            }
            fac = true;
        }
    }

    auto storageStats = std::make_shared<StorageStats>();
    auto cbExec = [fac, userId, storageStats](napi_env env) -> UniError {
        if (!fac) {
            *storageStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetUserStorageStats();
        } else {
            *storageStats = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetUserStorageStats(userId);
        }
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [storageStats](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal totalObject = NVal::CreateObject(env);
        totalObject.AddProp("total", NVal::CreateInt64(env, (storageStats->total_)).val_);
        totalObject.AddProp("audio", NVal::CreateInt64(env, (storageStats->audio_)).val_);
        totalObject.AddProp("video", NVal::CreateInt64(env, (storageStats->video_)).val_);
        totalObject.AddProp("image", NVal::CreateInt64(env, (storageStats->image_)).val_);
        totalObject.AddProp("file", NVal::CreateInt64(env, (storageStats->file_)).val_);
        totalObject.AddProp("app", NVal::CreateInt64(env, (storageStats->app_)).val_);
        return totalObject;
    };
    std::string procedureName = "GetUserStorageStats";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO || (funcArg.GetArgc() == (uint)NARG_CNT::ONE && fac)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        if (!fac) {
            NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
            return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
        }
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace StorageManager
} // namespace OHOS
