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
