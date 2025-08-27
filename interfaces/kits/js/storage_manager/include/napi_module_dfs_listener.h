/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef NAPI_MODULE_DFS_LISTENER_H
#define NAPI_MODULE_DFS_LISTENER_H
#ifdef HMDFS_FILE_MANAGER

#include <memory>
#include <string>

#include <node_api.h>

#include "distributed_file_daemon_manager.h"
#include "file_dfs_listener_stub.h"
#include "filemgmt_libn.h"

namespace OHOS {
namespace StorageManager {

class NapiFileDfsListener : public FileManagement::ModuleFileIO::FileDfsListenerStub {
public:
    NapiFileDfsListener(napi_env env, napi_value jsCallback) : env_(env), jsCallback_(jsCallback)
    {
        napi_create_reference(env, jsCallback_, 1, &callbackRef_);
    }

    ~NapiFileDfsListener()
    {
        napi_delete_reference(env_, callbackRef_);
    }

    // 禁止拷贝构造和赋值
    NapiFileDfsListener(const NapiFileDfsListener &) = delete;
    NapiFileDfsListener &operator=(const NapiFileDfsListener &) = delete;
    void OnStatus(const std::string &networkId, int32_t status, const std::string &path, int32_t type) override;
    struct StatusEventInfo {
        std::string networkId;
        std::string path;
        int32_t type;
        int32_t status;
    };

private:
    napi_env env_;
    napi_value jsCallback_;
    napi_ref callbackRef_;
};

} // namespace StorageManager
} // namespace OHOS
#endif
#endif // NAPI_MODULE_DFS_LISTENER_H