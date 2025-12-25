/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#ifndef VOLUME_INFO_MOCK_H
#define VOLUME_INFO_MOCK_H

#include "gmock/gmock.h"
#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class VolumeInfoMock : public VolumeInfo {
public:
    static inline std::shared_ptr<VolumeInfoMock> volumeInfoMock_ = nullptr;
public:
    VolumeInfoMock() {}
    virtual ~VolumeInfoMock() {}

    MOCK_METHOD1(DoCreate, int32_t(dev_t));
    MOCK_METHOD0(DoDestroy, int32_t());
    MOCK_METHOD1(DoMount, int32_t(uint32_t));
    MOCK_METHOD1(DoUMount, int32_t(bool));
    MOCK_METHOD0(DoUMountUsbFuse, int32_t());
    MOCK_METHOD0(DoCheck, int32_t());
    MOCK_METHOD1(DoFormat, int32_t(std::string));
    MOCK_METHOD1(DoSetVolDesc, int32_t(std::string));
    MOCK_METHOD0(DoTryToCheck, int32_t());
    MOCK_METHOD0(DoTryToFix, int32_t());
    MOCK_METHOD1(GetFsTypeByDev, std::string(dev_t));
    MOCK_METHOD1(IsUsbFuseByType, bool(std::string));
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // VOLUME_INFO_MOCK_H