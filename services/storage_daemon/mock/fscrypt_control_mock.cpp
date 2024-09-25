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

#include "fscrypt_control_mock.h"

#include "fscrypt_control.h"

using namespace std;

using namespace OHOS::StorageDaemon;

uint8_t GetFscryptVersionFromPolicy(void)
{
    if (IFscryptControlMoc::fscryptControlMoc == nullptr) {
        return FSCRYPT_V2;
    }
    return IFscryptControlMoc::fscryptControlMoc->GetFscryptVersionFromPolicy();
}

bool KeyCtrlHasFscryptSyspara(void)
{
    if (IFscryptControlMoc::fscryptControlMoc == nullptr) {
        return false;
    }
    return IFscryptControlMoc::fscryptControlMoc->KeyCtrlHasFscryptSyspara();
}

int LoadAndSetPolicy(const char *keyDir, const char *dir)
{
    if (IFscryptControlMoc::fscryptControlMoc == nullptr) {
        return false;
    }
    return IFscryptControlMoc::fscryptControlMoc->LoadAndSetPolicy(keyDir, dir);
}

int LoadAndSetEceAndSecePolicy(const char *keyDir, const char *dir, int type)
{
    if (IFscryptControlMoc::fscryptControlMoc == nullptr) {
        return false;
    }
    return IFscryptControlMoc::fscryptControlMoc->LoadAndSetEceAndSecePolicy(keyDir, dir, type);
}