/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef NETLINK_LISTENER_REAL_MOCK_H
#define NETLINK_LISTENER_REAL_MOCK_H

#include "gmock/gmock.h"

namespace OHOS {
namespace StorageDaemon {
class INetlinkListenerMock {
    public:
        virtual ~INetlinkListenerMock() = default;
        static inline std::shared_ptr<INetlinkListenerMock> netlinkListenerMoc = nullptr;
        virtual int32_t StartListener() = 0;
        virtual int32_t StopListener() = 0;
};

class NetlinkListenerRealMoc : public INetlinkListenerMock {
public:
    MOCK_METHOD0(StartListener, int32_t());
    MOCK_METHOD0(StopListener, int32_t());
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // NETLINK_LISTENER_REAL_MOCK_H