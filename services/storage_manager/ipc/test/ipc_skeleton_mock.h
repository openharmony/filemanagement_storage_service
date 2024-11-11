/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef MOCK_STORAGE_MANAGER_IPC_SKELETON_H
#define MOCK_STORAGE_MANAGER_IPC_SKELETON_H

#include <gmock/gmock.h>

#include "ipc_skeleton.h"

namespace OHOS {
namespace StorageManager {
class IIPCSkeletonMoc {
public:
    virtual ~IIPCSkeletonMoc() = default;
public:
    virtual pid_t GetCallingUid() = 0;
    virtual uint32_t GetCallingTokenID() = 0;
public:
    static inline std::shared_ptr<IIPCSkeletonMoc> ipcSkeletonMoc = nullptr;
};

class IPCSkeletonMoc : public IIPCSkeletonMoc {
public:
    MOCK_METHOD0(GetCallingUid, pid_t());
    MOCK_METHOD0(GetCallingTokenID, uint32_t());
};
}
}
#endif