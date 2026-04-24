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
#ifndef EXTERNAL_VOLUME_INFO_MOCK_H
#define EXTERNAL_VOLUME_INFO_MOCK_H

#include "gmock/gmock.h"
#include "volume/external_volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class ExternalVolumeInfoMock : public ExternalVolumeInfo {
public:
    ExternalVolumeInfoMock() {}
    virtual ~ExternalVolumeInfoMock() {}

    MOCK_METHOD1(DoCreate, int32_t(dev_t));
    MOCK_METHOD0(DoDestroy, int32_t());
    MOCK_METHOD1(DoMount, int32_t(uint32_t));
    MOCK_METHOD1(DoUMount, int32_t(bool));
    MOCK_METHOD0(DoCheck, int32_t());
    MOCK_METHOD1(DoFormat, int32_t(std::string));
    MOCK_METHOD1(DoSetVolDesc, int32_t(std::string));
    MOCK_METHOD0(DoFix4Ntfs, int32_t());
    MOCK_METHOD0(DoFix4Exfat, int32_t());
    MOCK_METHOD0(DoTryToFix, int32_t());
    MOCK_METHOD0(DoUMountWithForceUsbFuse, int32_t());
    MOCK_METHOD3(DoGetOddCapacity, int32_t(const std::string&, int64_t&, int64_t&));

    MOCK_METHOD0(DoTryToCheck, int32_t());
    MOCK_METHOD0(GetFsType, std::string());
    MOCK_METHOD1(GetFsTypeByDev, std::string(dev_t));
    MOCK_METHOD0(DoUMountUsbFuse, int32_t());
    MOCK_METHOD2(DoEncrypt, int32_t(const std::string&, const std::string&));
    MOCK_METHOD2(DoGetCryptProgressById, int32_t(const std::string&, int32_t&));
    MOCK_METHOD2(DoGetCryptUuidById, int32_t(const std::string&, std::string&));
    MOCK_METHOD3(DoBindRecoverKeyToPasswd, int32_t(const std::string&, const std::string&, const std::string&));
    MOCK_METHOD3(DoUpdateCryptPasswd, int32_t(const std::string&, const std::string&, const std::string&));
    MOCK_METHOD3(DoResetCryptPasswd, int32_t(const std::string&, const std::string&, const std::string&));
    MOCK_METHOD2(DoVerifyCryptPasswd, int32_t(const std::string&, const std::string&));
    MOCK_METHOD2(DoUnlock, int32_t(const std::string&, const std::string&));
    MOCK_METHOD2(DoDecrypt, int32_t(const std::string&, const std::string&));
    MOCK_METHOD1(DoDestroyCrypt, int32_t(const std::string&));
    MOCK_METHOD1(DoEject, int32_t(const std::string&));
    MOCK_METHOD2(DoGetOpticalDriveOpsProgress, int32_t(const std::string&, uint32_t&));
};
} // namespace StorageDaemon
} // namespace OHOS
#endif // EXTERNAL_VOLUME_INFO_MOCK_H