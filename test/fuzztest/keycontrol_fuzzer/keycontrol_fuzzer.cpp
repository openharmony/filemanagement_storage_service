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
#include <cstddef>
#include <cstdint>

namespace OHOS {
bool SysparamDynamicFuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return false;
    }
    bool result = false;
    struct fscrypt_add_key_arg fscryptaddkeyarg;
    struct fscrypt_add_key_arg *fscryptaddkeyarg2 = &fscryptaddkeyarg;
    struct fscrypt_remove_key_arg fscryptremovekeyarg;
    struct fscrypt_remove_key_arg *fscryptremovekeyarg2 = &fscryptremovekeyarg;
    struct fscrypt_get_key_status_arg fscryptgetkeystatusarg;
    struct fscrypt_get_key_status_arg *fscryptgetkeystatusarg2 = &fscryptgetkeystatusarg;
    struct fscrypt_get_policy_ex_arg fscryptgetpolicyexarg;
    struct fscrypt_get_policy_ex_arg *fscryptgetpolicyexarg2 = &fscryptgetpolicyexarg;
    struct fscrypt_policy fscryptpolicy;
    struct fscrypt_policy *fscryptpolicy2 = &fscryptpolicy;
    struct fscrypt_key fsKey1;
    struct fscrypt_key *fsKey = &fsKey1;
    union FscryptPolicy policy1;
    union FscryptPolicy *policy = &policy1;
    char character = *(reinterpret_cast<const char *>(data));
    char *character2 = &character;
    int state = *(reinterpret_cast<const int *>(data));
    KeyCtrlGetKeyringId(state, state);
    KeyCtrlAddKey(character2, character2, state);
    KeyCtrlAddKeyEx(character2, character2, fsKey, state);
    KeyCtrlSearch(state, character2, character2, state);
    KeyCtrlUnlink(state, state);
    KeyCtrlInstallKey(character2, fscryptaddkeyarg2);
    KeyCtrlRemoveKey(character2, fscryptremovekeyarg2);
    KeyCtrlGetKeyStatus(character2, fscryptgetkeystatusarg2);
    KeyCtrlGetPolicyEx(character2, fscryptgetpolicyexarg2);
    KeyCtrlSetPolicy(character2, policy);
    KeyCtrlGetPolicy(character2, fscryptpolicy2);
    KeyCtrlGetFscryptVersion(character2);
    KeyCtrlLoadVersion(character2);
    KeyCtrlHasFscryptSyspara();
    result = true;
    return result;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SysparamDynamicFuzzTest(data, size);
    return 0;
}
