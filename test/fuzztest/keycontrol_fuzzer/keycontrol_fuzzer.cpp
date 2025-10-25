/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "keycontrol_fuzzer.h"
#include "key_control.h"
#include <securec.h>
#include <cstddef>
#include <cstdint>
#define MAX_NUM 100

namespace OHOS {
bool SysparamDynamicFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int))) {
        return false;
    }
    struct fscrypt_policy fscryptpolicy;
    struct fscrypt_policy *fscryptpolicy2 = &fscryptpolicy;
    union FscryptPolicy policy1;
    union FscryptPolicy *policy = &policy1;
    char character[MAX_NUM] = { 0x00 };
    if (EOK != memcpy_s(character, sizeof(character)-1, data, size)) {
        return false;
    }

#ifdef SUPPORT_FSCRYPT_V2
    struct fscrypt_add_key_arg fscryptaddkeyarg;
    struct fscrypt_add_key_arg *fscryptaddkeyarg2 = &fscryptaddkeyarg;
    struct fscrypt_remove_key_arg fscryptremovekeyarg;
    struct fscrypt_remove_key_arg *fscryptremovekeyarg2 = &fscryptremovekeyarg;
    struct fscrypt_get_key_status_arg fscryptgetkeystatusarg;
    struct fscrypt_get_key_status_arg *fscryptgetkeystatusarg2 = &fscryptgetkeystatusarg;
    struct fscrypt_get_policy_ex_arg fscryptgetpolicyexarg;
    struct fscrypt_get_policy_ex_arg *fscryptgetpolicyexarg2 = &fscryptgetpolicyexarg;
    KeyCtrlInstallKey(character, fscryptaddkeyarg2);
    KeyCtrlRemoveKey(character, fscryptremovekeyarg2);
    KeyCtrlGetKeyStatus(character, fscryptgetkeystatusarg2);
    KeyCtrlGetPolicyEx(character, fscryptgetpolicyexarg2);
#endif
    KeyCtrlSetPolicy(character, policy);
    KeyCtrlGetPolicy(character, fscryptpolicy2);
    KeyCtrlGetFscryptVersion(character);
    KeyCtrlLoadVersion(character);
    KeyCtrlHasFscryptSyspara();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SysparamDynamicFuzzTest(data, size);
    return 0;
}
