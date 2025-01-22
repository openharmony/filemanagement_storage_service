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

#include "fscrypt_key_v2_mock.h"
#include "storage_service_errno.h"

using namespace std;
using namespace OHOS::StorageDaemon;

int32_t FscryptKeyV2::ActiveKey(uint32_t flag, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return E_KEY_EMPTY_ERROR;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->ActiveKey(flag, mnt);
}

int32_t FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return E_KEY_EMPTY_ERROR;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->InactiveKey(flag, mnt);
}

int32_t FscryptKeyV2::LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return E_KEY_EMPTY_ERROR;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->LockUserScreen(flag, sdpClass, mnt);
}

int32_t FscryptKeyV2::LockUece(bool &isFbeSupport)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return E_KEY_EMPTY_ERROR;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->LockUece(isFbeSupport);
}

int32_t FscryptKeyV2::UnlockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return E_KEY_EMPTY_ERROR;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->UnlockUserScreen(flag, sdpClass, mnt);
}

int32_t FscryptKeyV2::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->GenerateAppkey(userId, appUid, keyId);
}

int32_t FscryptKeyV2::DeleteAppkey(const std::string KeyId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DeleteAppkey(KeyId);
}

int32_t FscryptKeyV2::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->AddClassE(isNeedEncryptClassE, isSupport, status);
}

int32_t FscryptKeyV2::DeleteClassEPinCode(uint32_t user)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DeleteClassEPinCode(user);
}

int32_t FscryptKeyV2::ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->ChangePinCodeClassE(isFbeSupport, userId);
}

int32_t FscryptKeyV2::DecryptClassE(const UserAuth &auth, bool &isSupport,
                                    bool &eBufferStatue, uint32_t user, bool needSyncCandidate)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DecryptClassE(auth, isSupport, eBufferStatue, user, needSyncCandidate);
}

int32_t FscryptKeyV2::EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return -1;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->EncryptClassE(auth, isSupport, user, status);
}