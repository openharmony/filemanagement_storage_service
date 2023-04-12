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
#ifndef MOCK_STORAGE_DAEMON_STUB_H
#define MOCK_STORAGE_DAEMON_STUB_H

#include "gmock/gmock.h"

#include "ipc/storage_daemon_stub.h"

namespace OHOS {
namespace StorageDaemon {
class StorageDaemonStubMock : public StorageDaemonStub {
public:
    MOCK_METHOD0(Shutdown, int32_t(void));

    MOCK_METHOD2(Mount, int32_t(std::string, uint32_t));
    MOCK_METHOD1(UMount, int32_t(std::string));
    MOCK_METHOD1(Check, int32_t(std::string));
    MOCK_METHOD2(Format, int32_t(std::string, std::string));
    MOCK_METHOD2(Partition, int32_t(std::string, int32_t));
    MOCK_METHOD2(SetVolumeDescription, int32_t(std::string, std::string));

    MOCK_METHOD1(StartUser, int32_t(int32_t));
    MOCK_METHOD1(StopUser, int32_t(int32_t));
    MOCK_METHOD2(PrepareUserDirs, int32_t(int32_t, uint32_t));
    MOCK_METHOD2(DestroyUserDirs, int32_t(int32_t, uint32_t));

    MOCK_METHOD0(InitGlobalKey, int32_t(void));
    MOCK_METHOD0(InitGlobalUserKeys, int32_t(void));
    MOCK_METHOD2(GenerateUserKeys, int32_t(uint32_t, uint32_t));
    MOCK_METHOD1(DeleteUserKeys, int32_t(uint32_t));
    MOCK_METHOD4(UpdateUserAuth,  int32_t(uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &,
        const std::vector<uint8_t> &));
    MOCK_METHOD3(ActiveUserKey,  int32_t (uint32_t, const std::vector<uint8_t> &, const std::vector<uint8_t> &));
    MOCK_METHOD1(InactiveUserKey, int32_t (uint32_t));
    MOCK_METHOD1(UpdateKeyContext, int32_t (uint32_t));
    MOCK_METHOD3(CreateShareFile, int32_t (std::string, uint32_t, uint32_t));
    MOCK_METHOD2(DeleteShareFile, int32_t (uint32_t, std::vector<std::string>));
};
}  // namespace StorageDaemon
}  // namespace OHOS
#endif /* MOCK_STORAGE_DAEMON_STUB_H */