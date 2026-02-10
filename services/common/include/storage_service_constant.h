/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef STORAGE_SERVICE_CONSTANTS_H
#define STORAGE_SERVICE_CONSTANTS_H

#include <map>
#include <string>
#include <sys/types.h>

namespace OHOS {
namespace StorageService {
extern const int START_USER_ID;
extern const int MAX_USER_ID; // user id range is (0, 10737]
extern const uint32_t TOP_USER_ID;
extern const int START_APP_CLONE_USER_ID;
extern const int MAX_APP_CLONE_USER_ID;
extern const int ZERO_USER;
extern const int UID_FILE_MANAGER;
extern const int32_t USER_ID_BASE;
extern const int32_t APP_UID;
extern const int32_t ZERO_USER_MIN_UID;
extern const int32_t ZERO_USER_MAX_UID;
extern const int32_t DEFAULT_USER_ID;
extern const int MAX_APP_INDEX;
extern const int PRIORITY_LEVEL;
extern const int ANCO_SA_UID;
extern const int64_t TWO_G_BYTE;
extern const int64_t ONE_G_BYTE;
extern const char *USER_ID;
extern const char *BUNDLE_NAME;
extern const char *CLEAN_LEVEL;
extern const char *CLEAN_LEVEL_LOW;
extern const char *CLEAN_LEVEL_MEDIUM;
extern const char *CLEAN_LEVEL_HIGH;
extern const char *CLEAN_LEVEL_RICH;
extern const int32_t CLEAN_LOW_TIME; // min
extern const int32_t CLEAN_MEDIUM_TIME; // day
extern const int32_t CLEAN_HIGH_TIME; // week
extern const int32_t CLEAN_RICH_TIME; // week
extern const int32_t ROOT_UID;
extern const int32_t SYSTEM_UID;
extern const int32_t MEMMGR_UID;
}

namespace StorageDaemon {
extern const uint32_t GLOBAL_USER_ID;
extern const uint32_t USER_ID_SIZE_VALUE;
extern const char FILE_SEPARATOR_CHAR;
extern const char *ANCO_DIR;
extern const char *ANCO_MEDIA_PATH;
extern const char *SERVICE_DIR_PATH;
extern const std::string EL1;
extern const std::string EL2;
extern const std::string EL3;
extern const std::string EL4;
extern const std::string EL5;

enum KeyType {
    EL0_KEY = 0,
    EL1_KEY = 1,
    EL2_KEY = 2,
    EL3_KEY = 3,
    EL4_KEY = 4,
    EL5_KEY = 5,
};

enum QuotaIdType {
    USRID,
    GRPID,
    PRJID
};

enum IStorageDaemonEnum {
    CRYPTO_FLAG_EL1 = 1,
    CRYPTO_FLAG_EL2 = 2,
    CRYPTO_FLAG_EL3 = 4,
    CRYPTO_FLAG_EL4 = 8,
    CRYPTO_FLAG_EL5 = 16,
};
extern const std::map<std::string, KeyType> EL_DIR_MAP;
}
}

#endif // STORAGE_SERVICE_CONSTANTS_H