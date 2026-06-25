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

#include "burn_params.h"
#include "n_async/n_async_work_callback.h"
#include "n_async/n_async_work_promise.h"
#include "n_func_arg.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"

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
        int32_t errNum = StorageManagerConnect::GetInstance().GetAllVolumes(*volumeInfo);
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
        if (status != napi_ok) return { env, NError(status).GetNapiErr(env) };
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
            volumeInfoObject.AddProp("extraInfo", NVal::CreateUTF8String(env, (*volumeInfo)[i].GetExtraInfo()).val_);
            volumeInfoObject.AddProp("partitionNum", NVal::CreateUint32(env, (*volumeInfo)[i].GetPartitionNum()).val_);
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
        int32_t result = StorageManagerConnect::GetInstance().Mount(volumeIdString);
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
        int32_t result = StorageManagerConnect::GetInstance().Unmount(volumeIdString);
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
        int32_t errNum = StorageManagerConnect::GetInstance().GetVolumeByUuid(uuidString,
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
        volumeObject.AddProp("extraInfo", NVal::CreateUTF8String(env, volumeInfo->GetExtraInfo()).val_);
        volumeObject.AddProp("partitionNum", NVal::CreateUint32(env, volumeInfo->GetPartitionNum()).val_);
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
        int32_t errNum = StorageManagerConnect::GetInstance().GetVolumeById(volumeIdString,
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
        volumeObject.AddProp("extraInfo", NVal::CreateUTF8String(env, volumeInfo->GetExtraInfo()).val_);
        volumeObject.AddProp("partitionNum", NVal::CreateUint32(env, volumeInfo->GetPartitionNum()).val_);
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
        int32_t result = StorageManagerConnect::GetInstance().SetVolumeDescription(uuidString,
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
        int32_t result = StorageManagerConnect::GetInstance().Format(volumeIdString, fsTypeString);
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
        int32_t result = StorageManagerConnect::GetInstance().Partition(diskIdString, type);
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

napi_value GetAllDisks(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!CheckVolumes(env, info, funcArg)) {
        return nullptr;
    }
    auto diskInfo = std::make_shared<std::vector<Disk>>();
    auto cbExec = [diskInfo]() -> NError {
        int32_t errNum = StorageManagerConnect::GetInstance().GetAllDisks(*diskInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [diskInfo](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        napi_value diskInfoArray = nullptr;
        napi_status status = napi_create_array(env, &diskInfoArray);
        if (status != napi_ok) {
            return { env, NError(status).GetNapiErr(env) };
        }
        for (size_t i = 0; i < (*diskInfo).size(); i++) {
            NVal diskInfoObject = NVal::CreateObject(env);
            diskInfoObject.AddProp("diskId", NVal::CreateUTF8String(env, (*diskInfo)[i].GetDiskId()).val_);
            diskInfoObject.AddProp("sizeBytes", NVal::CreateInt64(env, (*diskInfo)[i].GetSizeBytes()).val_);
            diskInfoObject.AddProp("diskType", NVal::CreateInt32(env, (*diskInfo)[i].GetDiskType()).val_);
            diskInfoObject.AddProp("removable", NVal::CreateBool(env, (*diskInfo)[i].GetRemovable()).val_);
            std::list<std::string> volumeIds = (*diskInfo)[i].GetVolumeIds();
            napi_value volumeIdsArray = nullptr;
            napi_status createStatus = napi_create_array(env, &volumeIdsArray);
            if (createStatus == napi_ok) {
                size_t j = 0;
                for (const auto &id : volumeIds) {
                    napi_set_element(env, volumeIdsArray, j, NVal::CreateUTF8String(env, id).val_);
                    j++;
                }
            }
            diskInfoObject.AddProp("volumeIds", volumeIdsArray);
            diskInfoObject.AddProp("extraInfo", NVal::CreateUTF8String(env, (*diskInfo)[i].GetExtraInfo()).val_);
            status = napi_set_element(env, diskInfoArray, i, diskInfoObject.val_);
            if (status != napi_ok) {
                return { env, NError(status).GetNapiErr(env) };
            }
        }
        return { NVal(env, diskInfoArray) };
    };
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule("GetAllDisks", cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule("GetAllDisks", cbExec, cbComplete).val_;
    }
}

napi_value GetDiskById(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!CheckMount(env, info, funcArg)) {
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char []> diskId;
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto diskInfo = std::make_shared<Disk>();
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, diskInfo]() -> NError {
        int32_t errNum = StorageManagerConnect::GetInstance().GetDiskById(diskIdString, *diskInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [diskInfo](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal diskObject = NVal::CreateObject(env);
        diskObject.AddProp("diskId", NVal::CreateUTF8String(env, diskInfo->GetDiskId()).val_);
        diskObject.AddProp("sizeBytes", NVal::CreateInt64(env, diskInfo->GetSizeBytes()).val_);
        diskObject.AddProp("diskType", NVal::CreateInt32(env, diskInfo->GetDiskType()).val_);
        diskObject.AddProp("removable", NVal::CreateBool(env, diskInfo->GetRemovable()).val_);
        std::list<std::string> volumeIds = diskInfo->GetVolumeIds();
        napi_value volumeIdsArray = nullptr;
        napi_status createStatus = napi_create_array(env, &volumeIdsArray);
        if (createStatus == napi_ok) {
            size_t j = 0;
            for (const auto &id : volumeIds) {
                napi_set_element(env, volumeIdsArray, j, NVal::CreateUTF8String(env, id).val_);
                j++;
            }
        }
        diskObject.AddProp("volumeIds", volumeIdsArray);
        diskObject.AddProp("extraInfo", NVal::CreateUTF8String(env, diskInfo->GetExtraInfo()).val_);
        return diskObject;
    };
    std::string procedureName = "GetDiskById";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value GetPartitionTable(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!CheckMount(env, info, funcArg)) {
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char []> diskId;
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto partitionTableInfo = std::make_shared<PartitionTableInfo>();
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, partitionTableInfo]() -> NError {
        int32_t errNum = StorageManagerConnect::GetInstance().GetPartitionTable(diskIdString,
            *partitionTableInfo);
        if (errNum != E_OK) {
            return NError(Convert2JsErrNum(errNum));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [partitionTableInfo](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        NVal resultObject = NVal::CreateObject(env);
        resultObject.AddProp("diskId", NVal::CreateUTF8String(env, partitionTableInfo->GetDiskId()).val_);
        resultObject.AddProp("tableType", NVal::CreateUTF8String(env, partitionTableInfo->GetTableType()).val_);
        resultObject.AddProp("partitionCount", NVal::CreateUint32(env, partitionTableInfo->GetPartitionCount()).val_);
        int64_t totalSector = static_cast<int64_t>(partitionTableInfo->GetTotalSector());
        resultObject.AddProp("totalSector", NVal::CreateInt64(env, totalSector).val_);
        resultObject.AddProp("sectorSize", NVal::CreateUint32(env, partitionTableInfo->GetSectorSize()).val_);
        resultObject.AddProp("alignSector", NVal::CreateUint32(env, partitionTableInfo->GetAlignSector()).val_);
        napi_value partitionsArray = nullptr;
        napi_status status = napi_create_array(env, &partitionsArray);
        if (status != napi_ok) {
            return { env, NError(status).GetNapiErr(env) };
        }
        auto partitions = partitionTableInfo->GetPartitions();
        for (size_t i = 0; i < partitions.size(); i++) {
            NVal partitionObject = NVal::CreateObject(env);
            partitionObject.AddProp("partitionNum", NVal::CreateUint32(env, partitions[i].GetPartitionNum()).val_);
            partitionObject.AddProp("diskId", NVal::CreateUTF8String(env, partitions[i].GetDiskId()).val_);
            int64_t startSector = static_cast<int64_t>(partitions[i].GetStartSector());
            partitionObject.AddProp("startSector", NVal::CreateInt64(env, startSector).val_);
            int64_t endSector = static_cast<int64_t>(partitions[i].GetEndSector());
            partitionObject.AddProp("endSector", NVal::CreateInt64(env, endSector).val_);
            int64_t sizeBytes = static_cast<int64_t>(partitions[i].GetSizeBytes());
            partitionObject.AddProp("sizeBytes", NVal::CreateInt64(env, sizeBytes).val_);
            partitionObject.AddProp("fsType", NVal::CreateUTF8String(env, partitions[i].GetFsType()).val_);
            status = napi_set_element(env, partitionsArray, i, partitionObject.val_);
            if (status != napi_ok) {
                return { env, NError(status).GetNapiErr(env) };
            }
        }
        resultObject.AddProp("partitions", partitionsArray);
        return resultObject;
    };
    std::string procedureName = "GetPartitionTable";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value CreatePartition(napi_env env, napi_callback_info info)
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
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    napi_value partitionParamsObj = funcArg[(int)NARG_POS::SECOND];
    NVal paramsNVal(env, partitionParamsObj);
    PartitionParams partitionParams;
    NVal partitionNumVal = paramsNVal.GetProp("partitionNum");
    if (partitionNumVal.val_ != nullptr) {
        int32_t partitionNum;
        std::tie(succ, partitionNum) = partitionNumVal.ToInt32();
        if (succ) {
            partitionParams.SetPartitionNum(partitionNum);
        }
    }
    NVal startSectorVal = paramsNVal.GetProp("startSector");
    if (startSectorVal.val_ != nullptr) {
        int64_t startSector;
        std::tie(succ, startSector) = startSectorVal.ToInt64();
        if (succ) {
            partitionParams.SetStartSector(static_cast<uint64_t>(startSector));
        }
    }
    NVal endSectorVal = paramsNVal.GetProp("endSector");
    if (endSectorVal.val_ != nullptr) {
        int64_t endSector;
        std::tie(succ, endSector) = endSectorVal.ToInt64();
        if (succ) {
            partitionParams.SetEndSector(static_cast<uint64_t>(endSector));
        }
    }
    NVal typeCodeVal = paramsNVal.GetProp("typeCode");
    if (typeCodeVal.val_ != nullptr) {
        std::unique_ptr<char []> typeCode;
        tie(succ, typeCode, std::ignore) = typeCodeVal.ToUTF8String();
        if (succ) {
            std::string typeCodeStr(typeCode.get());
            partitionParams.SetTypeCode(typeCodeStr);
        }
    }
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, partitionParams]() -> NError {
        int32_t result = StorageManagerConnect::GetInstance().CreatePartition(diskIdString,
            partitionParams);
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
    std::string procedureName = "CreatePartition";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value DeletePartition(napi_env env, napi_callback_info info)
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
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    uint32_t partitionNum;
    std::tie(succ, partitionNum) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUint32();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, partitionNum]() -> NError {
        int32_t result = StorageManagerConnect::GetInstance().DeletePartition(diskIdString,
            partitionNum);
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
    std::string procedureName = "DeletePartition";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value FormatPartition(napi_env env, napi_callback_info info)
{
    if (!IsSystemApp()) {
        NError(E_PERMISSION_SYS).ThrowErr(env);
        return nullptr;
    }
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs((int)NARG_CNT::THREE, (int)NARG_CNT::FOUR)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    bool succ = false;
    std::unique_ptr<char []> diskId;
    tie(succ, diskId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    uint32_t partitionNum;
    std::tie(succ, partitionNum) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUint32();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    napi_value formatParamsObj = funcArg[(int)NARG_POS::THIRD];
    NVal paramsNVal(env, formatParamsObj);

    FormatParams formatParams;
    NVal fsTypeVal = paramsNVal.GetProp("fsType");
    if (fsTypeVal.val_ != nullptr) {
        std::unique_ptr<char []> fsType;
        tie(succ, fsType, std::ignore) = fsTypeVal.ToUTF8String();
        if (succ) {
            formatParams.SetFsType(std::string(fsType.get()));
        }
    }
    NVal quickFormatVal = paramsNVal.GetProp("quickFormat");
    if (quickFormatVal.val_ != nullptr) {
        bool quickFormat;
        std::tie(succ, quickFormat) = quickFormatVal.ToBool();
        if (succ) {
            formatParams.SetQuickFormat(quickFormat);
        }
    }
    NVal volumeNameVal = paramsNVal.GetProp("volumeName");
    if (volumeNameVal.val_ != nullptr) {
        std::unique_ptr<char []> volumeName;
        tie(succ, volumeName, std::ignore) = volumeNameVal.ToUTF8String();
        if (succ) {
            formatParams.SetVolumeName(std::string(volumeName.get()));
        }
    }
    std::string diskIdString(diskId.get());
    auto cbExec = [diskIdString, partitionNum, formatParams]() -> NError {
        int32_t result = StorageManagerConnect::GetInstance().FormatPartition(diskIdString,
            partitionNum, formatParams);
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
    std::string procedureName = "FormatPartition";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb, FEATURE_STR + __FUNCTION__)
            .Schedule(procedureName, cbExec, cbComplete).val_;
    }
}
} // namespace ModuleVolumeManager
} // namespace StorageManager
} // namespace OHOS
