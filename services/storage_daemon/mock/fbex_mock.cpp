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

#include "fbex_mock.h"

using namespace std;

using namespace OHOS::StorageDaemon;

bool FBEX::IsFBEXSupported()
{
    return IFbexMoc::fbexMoc->IsFBEXSupported();
}

int FBEX::InstallEL5KeyToKernel(uint32_t userIdSingle, uint32_t userIdDouble, uint8_t flag,
                                bool &isSupport, bool &isNeedEncryptClassE)
{
    return IFbexMoc::fbexMoc->InstallEL5KeyToKernel(userIdSingle, userIdDouble, flag, isSupport, isNeedEncryptClassE);
}

int FBEX::InstallKeyToKernel(uint32_t userId, uint32_t type, KeyBlob &iv, uint8_t flag, const KeyBlob &authToken)
{
    return IFbexMoc::fbexMoc->InstallKeyToKernel(userId, type, iv, flag, authToken);
}

int FBEX::UninstallOrLockUserKeyToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, bool destroy)
{
    return IFbexMoc::fbexMoc->UninstallOrLockUserKeyToKernel(userId, type, iv, size, destroy);
}

int FBEX::InstallDoubleDeKeyToKernel(UserIdToFbeStr &userIdToFbe, KeyBlob &iv, uint8_t flag, const KeyBlob &authToken)
{
    return IFbexMoc::fbexMoc->InstallDoubleDeKeyToKernel(userIdToFbe, iv, flag, authToken);
}

int FBEX::DeleteClassEPinCode(uint32_t userIdSingle, uint32_t userIdDouble)
{
    return IFbexMoc::fbexMoc->DeleteClassEPinCode(userIdSingle, userIdDouble);
}

int FBEX::ChangePinCodeClassE(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport)
{
    return IFbexMoc::fbexMoc->ChangePinCodeClassE(userIdSingle, userIdDouble, isFbeSupport);
}

// for el3 & el4
int FBEX::LockScreenToKernel(uint32_t userId)
{
    return IFbexMoc::fbexMoc->LockScreenToKernel(userId);
}

int FBEX::GenerateAppkey(UserIdToFbeStr &userIdToFbe, uint32_t appUid, std::unique_ptr<uint8_t[]> &appKey,
                         uint32_t size)
{
    return IFbexMoc::fbexMoc->GenerateAppkey(userIdToFbe, appUid, appKey, size);
}

// for el5
int FBEX::LockUece(uint32_t userIdSingle, uint32_t userIdDouble, bool &isFbeSupport)
{
    return IFbexMoc::fbexMoc->LockUece(userIdSingle, userIdDouble, isFbeSupport);
}

int FBEX::UnlockScreenToKernel(uint32_t userId, uint32_t type, uint8_t *iv, uint32_t size, const KeyBlob &authToken)
{
    return IFbexMoc::fbexMoc->UnlockScreenToKernel(userId, type, iv, size, authToken);
}

int FBEX::ReadESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status,
    KeyBlob &eBuffer, const KeyBlob &authToken, bool &isFbeSupport)
{
    return IFbexMoc::fbexMoc->ReadESecretToKernel(userIdToFbe, status, eBuffer, authToken, isFbeSupport);
}

int FBEX::WriteESecretToKernel(UserIdToFbeStr &userIdToFbe, uint32_t status, uint8_t *eBuffer, uint32_t length)
{
    return IFbexMoc::fbexMoc->WriteESecretToKernel(userIdToFbe, status, eBuffer, length);
}

bool FBEX::IsMspReady()
{
    return IFbexMoc::fbexMoc->IsMspReady();
}

int FBEX::GetStatus()
{
    return IFbexMoc::fbexMoc->GetStatus();
}