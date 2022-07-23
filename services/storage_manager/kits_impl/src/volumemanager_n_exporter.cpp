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
            volumeInfoObject.AddProp("diskId", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetDiskId()).val_);
            volumeInfoObject.AddProp("description",
                NVal::CreateUTF8String(env, (*volumeInfo)[i].GetDescription()).val_);
            volumeInfoObject.AddProp("removable", NVal::CreateBool(env, (bool)true).val_);
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
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
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}


napi_value GetVolumeByUuid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 1-2");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid uuid");
        return nullptr;
    }

    auto volumeInfo = std::make_shared<VolumeExternal>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, volumeInfo](napi_env env) -> UniError {
        *volumeInfo = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetVolumeByUuid(uuidString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal volumeObject = NVal::CreateObject(env);
        volumeObject.AddProp("id", NVal::CreateUTF8String(env, volumeInfo->GetId()).val_);
        volumeObject.AddProp("uuid", NVal::CreateUTF8String(env, volumeInfo->GetUuid()).val_);
        volumeObject.AddProp("diskId", NVal::CreateUTF8String(env, volumeInfo->GetDiskId()).val_);
        volumeObject.AddProp("description",
            NVal::CreateUTF8String(env, volumeInfo->GetDescription()).val_);
        volumeObject.AddProp("removable", NVal::CreateBool(env, (bool)true).val_);
        volumeObject.AddProp("state", NVal::CreateInt32(env, volumeInfo->GetState()).val_);
        volumeObject.AddProp("path", NVal::CreateUTF8String(env, volumeInfo->GetPath()).val_);

        return volumeObject;
    };

    std::string procedureName = "GetVolumeByUuid";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}


napi_value GetVolumeById(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched 1-2");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid volumeId");
        return nullptr;
    }

    auto volumeInfo = std::make_shared<VolumeExternal>();
    std::string volumeIdString(volumeId.get());
    auto cbExec = [volumeIdString, volumeInfo](napi_env env) -> UniError {
        *volumeInfo = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetVolumeById(volumeIdString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal volumeObject = NVal::CreateObject(env);
        volumeObject.AddProp("id", NVal::CreateUTF8String(env, volumeInfo->GetId()).val_);
        volumeObject.AddProp("uuid", NVal::CreateUTF8String(env, volumeInfo->GetUuid()).val_);
        volumeObject.AddProp("diskId", NVal::CreateUTF8String(env, volumeInfo->GetDiskId()).val_);
        volumeObject.AddProp("description",
            NVal::CreateUTF8String(env, volumeInfo->GetDescription()).val_);
        volumeObject.AddProp("removable", NVal::CreateBool(env, (bool)true).val_);
        volumeObject.AddProp("state", NVal::CreateInt32(env, volumeInfo->GetState()).val_);
        volumeObject.AddProp("path", NVal::CreateUTF8String(env, volumeInfo->GetPath()).val_);

        return volumeObject;
    };

    std::string procedureName = "GetVolumeById";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value SetVolumeDescription(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> uuid;
    std::unique_ptr<char []> description;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid uuid");
        return nullptr;
    }

    tie(succ, description, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid description");
        return nullptr;
    }

    auto result = std::make_shared<bool>();
    std::string uuidString(uuid.get());
    std::string descStr(description.get());
    auto cbExec = [uuidString, descStr, result](napi_env env) -> UniError {
        *result = DelayedSingleton<StorageManagerConnect>::GetInstance()->SetVolumeDescription(uuidString, descStr);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateBool(env, *result) };
    };

    std::string procedureName = "SetVolumeDescription";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Format(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> volumeId;
    std::unique_ptr<char []> fsType;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid volumeId");
        return nullptr;
    }

    tie(succ, fsType, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid fsType");
        return nullptr;
    }

    auto result = std::make_shared<bool>();
    std::string volumeIdString(volumeId.get());
    std::string fsTypeString(fsType.get());
    auto cbExec = [volumeIdString, fsTypeString, result](napi_env env) -> UniError {
        *result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Format(volumeIdString, fsTypeString);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateBool(env, *result) };
    };

    std::string procedureName = "Format";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Partition(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::TWO, (int)NARG_CNT::THREE)) {
        UniError(EINVAL).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> diskId;
    int32_t type;
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid diskId");
        return nullptr;
    }

    std::tie(succ, type) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        UniError(EINVAL).ThrowErr(env, "Invalid type");
        return nullptr;
    }

    auto result = std::make_shared<bool>();
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, type, result](napi_env env) -> UniError {
        *result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Partition(diskIdString, type);
        return UniError(ERRNO_NOERR);
    };
    auto cbComplete = [result](napi_env env, UniError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateBool(env, *result) };
    };

    std::string procedureName = "Partition";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleVolumeManager
} // namespace StorageManager
} // namespace OHOS
