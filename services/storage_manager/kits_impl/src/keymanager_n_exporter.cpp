/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "keymanager_n_exporter.h"
 
#include <singleton.h>
 
#include "n_class.h"
#include "n_error.h"
#include "n_func_arg.h"
#include "n_val.h"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "keymanager_napi.h"
 
using namespace OHOS::StorageManager;
using namespace OHOS::FileManagement::LibN;
 
namespace OHOS {
namespace StorageManager {
 
napi_value DeactivateUserKey(napi_env env, napi_callback_info info)
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
    bool succ = false;
    uint32_t userId;
    std::tie(succ, userId) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUint32();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto err = DelayedSingleton<StorageManagerConnect>::GetInstance()->LockUserScreen(userId);
    if (err != E_OK) {
        NError(Convert2JsErrNum(err)).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}
} // namespace StorageManager
} // namespace OHOS