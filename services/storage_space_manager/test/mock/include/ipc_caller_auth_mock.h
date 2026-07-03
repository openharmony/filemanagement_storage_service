/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_MOCK_H
#define OHOS_STORAGE_SPACE_MANAGER_IPC_CALLER_AUTH_MOCK_H

#include <cstdint>
#include <string>

extern uint32_t g_mockCallingTokenId;
extern uint64_t g_mockCallingFullTokenId;
extern int32_t g_mockCallingUid;
extern int32_t g_mockVerifyAccessTokenResult;
extern uint32_t g_mockTokenTypeFlag;
extern bool g_mockIsSystemApp;
extern int32_t g_mockGetNativeTokenInfoResult;
extern std::string g_mockNativeProcessName;
extern int32_t g_mockGetHapTokenInfoResult;
extern std::string g_mockHapBundleName;

#endif
