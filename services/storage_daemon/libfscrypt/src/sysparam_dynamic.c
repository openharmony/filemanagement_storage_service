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
#include "fscrypt_sysparam.h"

#include "fscrypt_log.h"
#include "parameter.h"

int GetFscryptParameter(const char *key, const char *def, char *value, unsigned int *len)
{
    if (!key || !value || !len) {
        FSCRYPT_LOGE("GetFscryptParameter: there is null param");
        return -EINVAL;
    }
    if (GetParameter(key, "", value, *len) <= 0) {
        FSCRYPT_LOGE("GetFscryptParameter: get parameter error");
        return -EFAULT;
    }
    return 0;
}

int SetFscryptParameter(const char *policy, const char *value)
{
    if (!policy || !value) {
        FSCRYPT_LOGE("GetFscryptParameter: there is null param");
        return -EINVAL;
    }
    return SetParameter(policy, value);
}