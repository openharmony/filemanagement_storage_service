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

#include "volumemanager_n_exporter.h"

#include "n_async/n_async_work_callback.h"
#include "n_async/n_async_work_promise.h"
#include "n_func_arg.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"

using namespace OHOS::StorageManager;
using namespace OHOS::FileManagement::LibN;

namespace OHOS {
namespace StorageManager {
namespace ModuleVolumeManager {
const std::string FEATURE_STR = "VolumeManager.";

bool CheckVolumes(napi_env env, napi_callback_info info, NFuncArg& funcArg)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return false;
    }
    if (!funcArg.InitArgs((int)NARG_CNT::ZERO, (int)NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return false;
    }
    return true;
}

napi_value GetAllVolumes(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!CheckVolumes(env, info, funcArg)) {
        return nullptr;
    }
    auto volumeInfo = std::make_shared<std::vector<VolumeExternal>>();
    auto cbExec = [volumeInfo]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetAllVolumes(*volumeInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value volumeInfoArray = nullptr;
        napi_status status = napi_create_array(env, &volumeInfoArray);
        if (status != napi_ok) {
            return { env, NError(status).GetNapiErr(env) };
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
            volumeInfoObject.AddProp("fsType", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetFsTypeString()).val_);
            volumeInfoObject.AddProp("diskType", NVal::CreateInt32(env, (*volumeInfo)[i].GetFlags()).val_);
            status = napi_set_element(env, volumeInfoArray, i, volumeInfoObject.val_);
            if (status != napi_ok) {
                return { env, NError(status).GetNapiErr(env) };
            }
        }
        return { NVal(env, volumeInfoArray) };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetAllVolumes", cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule("GetAllVolumes", cbExec, cbComplete).val_;
    }
}

bool CheckMount(napi_env env, napi_callback_info info, NFuncArg& funcArg)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return false;
    }
    if (!funcArg.InitArgs((int)NARG_CNT::ONE, (int)NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env);
        return false;
    }
    return true;
}


napi_value Mount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    bool checkresult = CheckMount(env, info, funcArg);
    if (!checkresult) {
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
    auto cbExec = [volumeIdString]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Mount(volumeIdString);
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

    std::string procedureName = "Mount";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Unmount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    bool checkresult = CheckMount(env, info, funcArg);
    if (!checkresult) {
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
    auto cbExec = [volumeIdString]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Unmount(volumeIdString);
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

    std::string procedureName = "Unmount";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}


napi_value GetVolumeByUuid(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    bool checkresult = CheckMount(env, info, funcArg);
    if (!checkresult) {
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char []> uuid;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto volumeInfo = std::make_shared<VolumeExternal>();
    std::string uuidString(uuid.get());
    auto cbExec = [uuidString, volumeInfo]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetVolumeByUuid(uuidString,
            *volumeInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, NError err) -> NVal {
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
        volumeObject.AddProp("fsType", NVal::CreateUTF8String(env, volumeInfo->GetFsTypeString()).val_);
        volumeObject.AddProp("diskType", NVal::CreateInt32(env, volumeInfo->GetFlags()).val_);
        return volumeObject;
    };

    std::string procedureName = "GetVolumeByUuid";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}


napi_value GetVolumeById(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    bool checkresult = CheckMount(env, info, funcArg);
    if (!checkresult) {
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char []> volumeId;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto volumeInfo = std::make_shared<VolumeExternal>();
    std::string volumeIdString(volumeId.get());
    auto cbExec = [volumeIdString, volumeInfo]() -> NError {
        int32_t errNum = DelayedSingleton<StorageManagerConnect>::GetInstance()->GetVolumeById(volumeIdString,
            *volumeInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [volumeInfo](napi_env env, NError err) -> NVal {
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
        volumeObject.AddProp("diskType", NVal::CreateInt32(env, volumeInfo->GetFlags()).val_);
        volumeObject.AddProp("fsType", NVal::CreateUTF8String(env, volumeInfo->GetFsTypeString()).val_);
        
        return volumeObject;
    };

    std::string procedureName = "GetVolumeById";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value SetVolumeDescription(napi_env env, napi_callback_info info)
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
    std::unique_ptr<char []> uuid;
    std::unique_ptr<char []> description;
    tie(succ, uuid, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, description, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string uuidString(uuid.get());
    std::string descStr(description.get());
    auto cbExec = [uuidString, descStr]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->SetVolumeDescription(uuidString,
            descStr);
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

    std::string procedureName = "SetVolumeDescription";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Format(napi_env env, napi_callback_info info)
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
    std::unique_ptr<char []> fsType;
    tie(succ, volumeId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, fsType, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string volumeIdString(volumeId.get());
    std::string fsTypeString(fsType.get());
    auto cbExec = [volumeIdString, fsTypeString]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Format(volumeIdString, fsTypeString);
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

    std::string procedureName = "Format";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value Partition(napi_env env, napi_callback_info info)
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
    std::unique_ptr<char []> diskId;
    int32_t type;
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::tie(succ, type) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToInt32();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, type]() -> NError {
        int32_t result = DelayedSingleton<StorageManagerConnect>::GetInstance()->Partition(diskIdString, type);
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

    std::string procedureName = "Partition";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleVolumeManager
} // namespace StorageManager
} // namespace OHOS
