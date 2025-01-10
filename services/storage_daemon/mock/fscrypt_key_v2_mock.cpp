
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

#include "fscrypt_key_v2_mock.h"

using namespace std;
using namespace OHOS::StorageDaemon;

bool FscryptKeyV2::ActiveKey(uint32_t flag, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->ActiveKey(flag, mnt);
}

bool FscryptKeyV2::InactiveKey(uint32_t flag, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->InactiveKey(flag, mnt);
}

bool FscryptKeyV2::LockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->LockUserScreen(flag, sdpClass, mnt);
}

bool FscryptKeyV2::LockUece(bool &isFbeSupport)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->LockUece(isFbeSupport);
}

bool FscryptKeyV2::UnlockUserScreen(uint32_t flag, uint32_t sdpClass, const std::string &mnt)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->UnlockUserScreen(flag, sdpClass, mnt);
}

bool FscryptKeyV2::GenerateAppkey(uint32_t userId, uint32_t appUid, std::string &keyId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->GenerateAppkey(userId, appUid, keyId);
}

bool FscryptKeyV2::DeleteAppkey(const std::string KeyId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DeleteAppkey(KeyId);
}

bool FscryptKeyV2::AddClassE(bool &isNeedEncryptClassE, bool &isSupport, uint32_t status)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->AddClassE(isNeedEncryptClassE, isSupport, status);
}

bool FscryptKeyV2::DeleteClassEPinCode(uint32_t user)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DeleteClassEPinCode(user);
}

bool FscryptKeyV2::ChangePinCodeClassE(bool &isFbeSupport, uint32_t userId)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->ChangePinCodeClassE(isFbeSupport, userId);
}

bool FscryptKeyV2::DecryptClassE(const UserAuth &auth, bool &isSupport,
                                 bool &eBufferStatue, uint32_t user, bool needSyncCandidate)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->DecryptClassE(auth, isSupport, eBufferStatue, user, needSyncCandidate);
}

bool FscryptKeyV2::EncryptClassE(const UserAuth &auth, bool &isSupport, uint32_t user, uint32_t status)
{
    if (IFscryptKeyV2Moc::fscryptKeyV2Moc == nullptr) {
        return false;
    }
    return IFscryptKeyV2Moc::fscryptKeyV2Moc->EncryptClassE(auth, isSupport, user, status);
}