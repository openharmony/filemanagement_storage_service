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

#include "storage_statistics_n_exporter.h"

#include "n_async/n_async_work_callback.h"
#include "n_async/n_async_work_promise.h"
#include "n_func_arg.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

using namespace OHOS::FileManagement::LibN;

namespace OHOS {
namespace StorageManager {
const std::string EMPTY_STRING = "";
const std::string FEATURE_STR = "StorageStatistics.";
constexpr int32_t INVALID_INDEX = -1;
constexpr int32_t INVALID_STATFLAG = -1;

napi_value GetTotalSizeOfVolume(napi_env env, napi_callback_info info)
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
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, resultSize]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetTotalSizeOfVolume(uuidString,
            *resultSize);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}


napi_value GetFreeSizeOfVolume(napi_env env, napi_callback_info info)
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
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, resultSize]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetFreeSizeOfVolume(uuidString,
            *resultSize);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

std::tuple<std::string, int32_t, uint32_t> ExtractNameAndIndex(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::FOUR)) {
        NError(E_PARAMS).ThrowErr(env);
        return std::make_tuple(EMPTY_STRING, INVALID_INDEX, INVALID_STATFLAG);
    }
    bool succ = false;
    std::unique_ptr<char []> name;
    tie(succ, name, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return std::make_tuple(EMPTY_STRING, INVALID_INDEX, INVALID_STATFLAG);
    }
    std::string nameString(name.get());
    int32_t index = 0;
    uint32_t statFlag = 0;
    if (funcArg.GetArgc() == (uint)NARG_CNT::FOUR) {
        std::tie(succ, index) = NVal(env, funcArg[(int)NARG_POS::THIRD]).ToInt32();
        std::tie(succ, statFlag) = NVal(env, funcArg[(int)NARG_POS::FOURTH]).ToUint32();
    } else if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        if (NVal(env, funcArg[(int)NARG_POS::SECOND]).TypeIs(napi_number)) {
            std::tie(succ, index) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToInt32();
            std::tie(succ, statFlag) = NVal(env, funcArg[(int)NARG_POS::THIRD]).ToUint32();
        } else {
            std::tie(succ, index) = NVal(env, funcArg[(int)NARG_POS::THIRD]).ToInt32();
        }
    } else if (NVal(env, funcArg[(int)NARG_POS::SECOND]).TypeIs(napi_number)) {
        std::tie(succ, index) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToInt32();
    }
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return std::make_tuple(EMPTY_STRING, INVALID_INDEX, INVALID_STATFLAG);
    }
    return std::make_tuple(nameString, index, statFlag);
}

napi_value GetBundleStats(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    auto result = ExtractNameAndIndex(env, info);
    if (std::get<0>(result).empty()) {
        LOGE("Extract pkgName and index from arguments failed");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::string nameString = std::get<0>(result);
    int32_t index = std::get<1>(result);
    int32_t statFlag = static_cast<int32_t>(std::get<2>(result));
    auto bundleStats = std::make_shared<BundleStats>();
    auto cbExec = [nameString, bundleStats, index, statFlag]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetBundleStats(nameString,
            *bundleStats, index, statFlag);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [bundleStats](napi_env env, NError err) -> NVal {
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
    NFuncArg funcArg(env, info);
    funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::THREE);
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE || NVal(env, funcArg[(int)NARG_POS::SECOND]).TypeIs(napi_number)) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetCurrentBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto bundleStats = std::make_shared<BundleStats>();
    uint32_t statFlag = 0;
        if (funcArg.GetArgc() >= 1) {
        NVal flag(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
        if (flag.TypeIs(napi_number)) {
            bool succ = false;
            std::tie(succ, statFlag) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUint32();
            if (!succ) {
                NError(E_PARAMS).ThrowErr(env);
                return nullptr;
            }
        }
    }

    auto cbExec = [bundleStats, statFlag]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetCurrentBundleStats(*bundleStats,
            statFlag);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [bundleStats](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetSystemSize(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetSystemSize(*resultSize);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetUserStorageStats(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env);
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
                NError(E_PARAMS).ThrowErr(env);
                return nullptr;
            }
            fac = true;
        }
    }

    auto storageStats = std::make_shared<StorageStats>();
    auto cbExec = [fac, userId, storageStats]() -> NError {
        int32_t errNum;
        if (!fac) {
            errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetUserStorageStats(*storageStats);
        } else {
            errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetUserStorageStats(userId, *storageStats);
        }
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [storageStats](napi_env env, NError err) -> NVal {
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
            return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
                .Schedule(procedureName, cbExec, cbComplete).val_;
        }
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetTotalSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetTotalSize(*resultSize);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetFreeSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    auto cbExec = [resultSize]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetFreeSize(*resultSize);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [resultSize](napi_env env, NError err) -> NVal {
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
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetTotalSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    
    int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetTotalSize(*resultSize);
    if (errNum != E_OK) {
        NError(Convert2JsErrNum(errNum)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateInt64(env, *resultSize).val_;
}

napi_value GetFreeSizeSync(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto resultSize = std::make_shared<int64_t>();
    
    int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetFreeSize(*resultSize);
    if (errNum != E_OK) {
        NError(Convert2JsErrNum(errNum)).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateInt64(env, *resultSize).val_;
}

static bool ParseExtBundleStats(napi_env env, NVal statsObj, ExtBundleStats &stats)
{
    bool succ = false;
    NVal nameVal = statsObj.GetProp("businessName");
    std::unique_ptr<char[]> nameBuf;
    std::tie(succ, nameBuf, std::ignore) = nameVal.ToUTF8String();
    if (!succ || !nameBuf || !nameBuf[0]) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return false;
    }
    stats.businessName_ = std::string(nameBuf.get());
    NVal sizeVal = statsObj.GetProp("size");
    int64_t tempSize = 0;
    std::tie(succ, tempSize) = sizeVal.ToInt64();
    if (!succ) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return false;
    }
    if (tempSize < 0) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return false;
    }
    stats.businessSize_ = static_cast<uint64_t>(tempSize);
    NVal flagVal = statsObj.GetProp("flag");
    if (!flagVal.TypeIs(napi_boolean)) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return false;
    }
    std::tie(succ, stats.showFlag_) = flagVal.ToBool();
    if (!succ) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return false;
    }
    return true;
}

napi_value SetExtBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    uint32_t userId = 0;
    bool succ = false;
    std::tie(succ, userId) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUint32();
    if (!succ) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    NVal statsObj(env, funcArg[static_cast<int>(NARG_POS::SECOND)]);
    if (!statsObj.TypeIs(napi_object)) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    auto stats = std::make_shared<ExtBundleStats>();
    if (!ParseExtBundleStats(env, statsObj, *stats)) {
        return nullptr;
    }
    auto cbExec = [userId, stats]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->SetExtBundleStats(userId, *stats);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateBool(env, true);
    };
    NVal thisVar(env, funcArg.GetThisVar());
    std::string procedureName = "SetExtBundleStats";
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, std::move(cbExec), std::move(cbComplete)).val_;
}

napi_value GetExtBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::TWO))) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    uint32_t userId = 0;
    bool succ = false;
    std::tie(succ, userId) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUint32();
    if (!succ) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    std::unique_ptr<char[]> nameBuf;
    std::tie(succ, nameBuf, std::ignore) = NVal(env, funcArg[NARG_POS::SECOND]).ToUTF8String();
    if (!succ || !nameBuf || !nameBuf[0]) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    auto extStats = std::make_shared<ExtBundleStats>();
    std::string businessName = std::string(nameBuf.get());
    extStats->businessName_ = businessName;
    auto cbExec = [userId, extStats]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetExtBundleStats(userId, *extStats);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [extStats](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("businessName", NVal::CreateUTF8String(env, extStats->businessName_).val_);
        obj.AddProp("size", NVal::CreateInt64(env, static_cast<int64_t>(extStats->businessSize_)).val_);
        obj.AddProp("flag", NVal::CreateBool(env, extStats->showFlag_).val_);
        return obj;
    };
    std::string procedureName = "GetExtBundleStats";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, std::move(cbExec), std::move(cbComplete)).val_;
}

napi_value GetAllExtBundleStats(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<int>(NARG_CNT::ONE))) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    uint32_t userId = 0;
    std::tie(succ, userId) = NVal(env, funcArg[static_cast<int>(NARG_POS::FIRST)]).ToUint32();
    if (!succ) {
        NError(E_JS_PARAMS_INVALID).ThrowErr(env);
        return nullptr;
    }
    auto statsVec = std::make_shared<std::vector<ExtBundleStats>>();
    auto cbExec = [userId, statsVec]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetAllExtBundleStats(userId,
            *statsVec);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [statsVec](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value resultArray = nullptr;
        napi_create_array(env, &resultArray);
        uint32_t index = 0;
        for (const auto &extStats : *statsVec) {
            NVal obj = NVal::CreateObject(env);
            obj.AddProp("businessName", NVal::CreateUTF8String(env, extStats.businessName_).val_);
            obj.AddProp("size", NVal::CreateInt64(env, static_cast<int64_t>(extStats.businessSize_)).val_);
            obj.AddProp("flag", NVal::CreateBool(env, extStats.showFlag_).val_);
            napi_set_element(env, resultArray, index++, obj.val_);
        }
        return NVal(env, resultArray);
    };
    std::string procedureName = "GetAllExtBundleStats";
    NVal thisVar(env, funcArg.GetThisVar());
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, std::move(cbExec), std::move(cbComplete)).val_;
}

} // namespace StorageManager
} // namespace OHOS
