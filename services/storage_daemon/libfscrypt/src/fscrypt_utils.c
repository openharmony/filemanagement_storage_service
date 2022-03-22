/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include "fscrypt_utils.h"

#include "fscrypt_log.h"
#include "fscrypt_control.h"

int FscryptPolicyEnable(const char *dir)
{
    FSCRYPT_LOGI("FscryptPolicyEnable start, dir = %s", dir);
    if (!dir) {
        return -EINVAL;
    }

    return SetGlobalEl1DirPolicy(dir);
}

int SetFscryptSysparam(const char *policy)
{
    FSCRYPT_LOGI("SetFscryptSysparam start1, policy = %s", policy);
    if (!policy) {
        return -EINVAL;
    }

    return FscryptSetSysparam(policy);
}