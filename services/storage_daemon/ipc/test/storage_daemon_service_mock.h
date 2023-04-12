/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef MOCK_STORAGE_DAEMON_SERVICE_H
#define MOCK_STORAGE_DAEMON_SERVICE_H

#include "gmock/gmock.h"
#include "ipc/storage_daemon_stub.h"
#include "storage_service_errno.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonServiceMock :  public IRemoteStub<IStorageDaemon> {
public:
    int code_ = 0;
    StorageDaemonServiceMock() : code_(0) {}
    virtual ~StorageDaemonServiceMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return E_OK;
    }

    virtual int32_t Shutdown() override
    {
        return E_OK;
    }

    virtual int32_t Mount(std::string volId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t UMount(std::string volId) override
    {
        return E_OK;
    }

    virtual int32_t Check(std::string volId) override
    {
        return E_OK;
    }

    virtual int32_t Format(std::string volId, std::string fsType) override
    {
        return E_OK;
    }

    virtual int32_t Partition(std::string diskId, int32_t type) override
    {
        return E_OK;
    }

    virtual int32_t SetVolumeDescription(std::string volId, std::string description) override
    {
        return E_OK;
    }

    virtual int32_t StartUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t StopUser(int32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t PrepareUserDirs(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t DestroyUserDirs(int32_t userId, uint32_t flags) override
    {
        return E_OK;
    }

    virtual int32_t InitGlobalKey(void) override
    {
        return E_OK;
    }

    virtual int32_t InitGlobalUserKeys(void) override
    {
        return E_OK;
    }

    virtual int32_t GenerateUserKeys(uint32_t userId, uint32_t flags) override
    {
        return E_OK;
    }
    virtual int32_t DeleteUserKeys(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t UpdateUserAuth(uint32_t userId, const std::vector<uint8_t> &token,
                                   const std::vector<uint8_t> &oldSecret,
                                   const std::vector<uint8_t> &newSecret) override
    {
        return E_OK;
    }

    virtual int32_t ActiveUserKey(uint32_t userId,
                                  const std::vector<uint8_t> &token,
                                  const std::vector<uint8_t> &secret) override
    {
        return E_OK;
    }

    virtual int32_t InactiveUserKey(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t UpdateKeyContext(uint32_t userId) override
    {
        return E_OK;
    }

    virtual int32_t CreateShareFile(std::string uri, uint32_t tokenId, uint32_t flag) override
    {
        return E_OK;
    }

    virtual int32_t DeleteShareFile(uint32_t tokenId, std::vector<std::string>sharePathList) override
    {
        return E_OK;
    }
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // MOCK_STORAGE_DAEMON_SERVICE_H