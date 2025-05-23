/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "fscrypt_key_v1_ext_mock.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;

int32_t FscryptKeyV1Ext::ActiveKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->ActiveKeyExt(flag, iv, elType, authToken);
}

int32_t FscryptKeyV1Ext::ActiveDoubleKeyExt(uint32_t flag, KeyBlob &iv, uint32_t &elType, const KeyBlob &authToken)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->ActiveDoubleKeyExt(flag, iv, elType, authToken);
}

int32_t FscryptKeyV1Ext::InactiveKeyExt(uint32_t flag)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->InactiveKeyExt(flag);
}

int32_t FscryptKeyV1Ext::LockUserScreenExt(uint32_t flag, uint32_t &elType)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->LockUserScreenExt(flag, elType);
}

int32_t FscryptKeyV1Ext::UnlockUserScreenExt(uint32_t flag, uint8_t *iv, uint32_t size, const KeyBlob &authToken)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->UnlockUserScreenExt(flag, iv, size, authToken);
}

int32_t FscryptKeyV1Ext::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->AddClassE(isNeedEncryptClassE, isSupport, status);
}

int32_t FscryptKeyV1Ext::DeleteClassEPinCode(uint32_t userId)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->DeleteClassEPinCode(userId);
}

int32_t FscryptKeyV1Ext::ChangePinCodeClassE(uint32_t userId, bool &isFbeSupport)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->ChangePinCodeClassE(userId, isFbeSupport);
}

int32_t FscryptKeyV1Ext::ReadClassE(uint32_t status, KeyBlob &classEBuffer, const KeyBlob &authToken,
                                    bool &isFbeSupport)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->ReadClassE(status, classEBuffer, authToken);
}

int32_t FscryptKeyV1Ext::WriteClassE(uint32_t status, uint8_t *classEBuffer, uint32_t length)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->WriteClassE(status, classEBuffer, length);
}

int32_t FscryptKeyV1Ext::GenerateAppkey(uint32_t userId, uint32_t appUid,
                                        std::unique_ptr<uint8_t[]> &keyId,
                                        uint32_t size)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->GenerateAppkey(userId, appUid, keyId, size);
}

int32_t FscryptKeyV1Ext::LockUeceExt(bool &isFbeSupport)
{
    return IFscryptKeyV1Ext::fscryptKeyV1ExtMock->LockUeceExt(isFbeSupport);
}


uint32_t FscryptKeyV1Ext::GetUserIdFromDir()
{
    return 0;
}

uint32_t FscryptKeyV1Ext::GetTypeFromDir()
{
    return 0;
}

uint32_t FscryptKeyV1Ext::GetMappedUserId(uint32_t userId, uint32_t type)
{
    return 0;
}

uint32_t FscryptKeyV1Ext::GetMappedDeUserId(uint32_t userId)
{
    return 0;
}
} // namespace StorageDaemon
} // namespace OHOS