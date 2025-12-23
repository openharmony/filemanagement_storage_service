/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
 
#include "keymanager_napi.h"

#include "keymanager_n_exporter.h"
#include "storage_service_log.h"
 
namespace OHOS {
namespace StorageManager {
/***********************************************
 * Module export and register
 ***********************************************/
 napi_value KeyManagerExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("deactivateUserKey", DeactivateUserKey),
    };
    FILEMGMT_CALL(napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
 
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = KeyManagerExport,
    .nm_modname = "file.keyManager",
    .nm_priv = ((void *)0),
    .reserved = {0}
};
 
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace StorageManager
} // namespace OHOS