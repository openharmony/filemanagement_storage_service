/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include <sys/types.h>

namespace OHOS {
namespace StorageService {
const int START_USER_ID = 0;
const int MAX_USER_ID = 10738; // user id range is (0, 10737]
const uint32_t TOP_USER_ID = 10738;
const int START_APP_CLONE_USER_ID = 219;
const int MAX_APP_CLONE_USER_ID = 239;
const int ZERO_USER = 0;
const int UID_FILE_MANAGER = 1006;
const int32_t USER_ID_BASE = 200000;
const int32_t APP_UID = 20000000;
const int32_t ZERO_USER_MIN_UID = 20000;
const int32_t ZERO_USER_MAX_UID = 65535;
const int32_t DEFAULT_USER_ID = 100;
const int MAX_APP_INDEX = 5;
const int PRIORITY_LEVEL = -20;
const int ANCO_SA_UID = 7558;
const int64_t TWO_G_BYTE = 2LL * 1000 * 1000 * 1000;
const int64_t ONE_G_BYTE = 1LL * 1000 * 1000 * 1000;
const int32_t ROWCOUNT_INIT = 0;
const int32_t COLINDEX_INIT = -1;
const std::string CREATE_BUNDLE_EXT_STATS_TABLE_SQL = "CREATE TABLE IF NOT EXISTS bundle_ext_stats_table \
( \
    businessName               TEXT NOT NULL, \
    businessSize               LONG NOT NULL, \
    userId                     INTEGER NOT NULL DEFAULT 0, \
    bundleName                 TEXT NOT NULL, \
    lastModifyTime             INTEGER NOT NULL DEFAULT 0, \
    showFlag                   INTEGER NOT NULL, \
    PRIMARY KEY (businessName, userId) \
);";
const std::string CREATE_CLEAN_NOTIFY_TABLE_SQL = "CREATE TABLE IF NOT EXISTS clean_notify_table  \
( \
    cleanLevelName             TEXT NOT NULL, \
    lastCleanNotifyTime        INTEGER NOT NULL DEFAULT 0 \
);";
const std::string BUSINESS_NAME = "businessName";
const std::string BUSINESS_SIZE = "businessSize";
const std::string USER_ID = "userId";
const std::string BUNDLE_NAME = "bundleName";
const std::string LAST_MODIFY_TIME = "lastModifyTime";
const std::string SHOW_FLAG = "showFlag";
const std::string BUNDLE_EXT_STATS_TABLE = "bundle_ext_stats_table";
const std::string WHERE_CLAUSE = "businessName = ? and userId = ?";
const std::string SELECT_BUNDLE_EXT_SQL = "SELECT * FROM bundle_ext_stats_table "
    "WHERE businessName = ? AND userId = ? LIMIT 1";
const std::string SELECT_ALL_BUNDLE_EXT_SQL = "SELECT * FROM bundle_ext_stats_table "
    "WHERE userId = ?";

const std::string CLEAN_LEVEL = "clean_level";
const std::string LEVEL_NAME = "cleanLevelName";
const std::string LAST_CLEAN_NOTIFY_TIME = "lastCleanNotifyTime";
const std::string CLEAN_NOTIFY_TABLE = "clean_notify_table ";
const std::string WHERE_CLAUSE_LEVEL = "cleanLevelName = ?";
const std::string SELECT_CLEAN_NOTIFY_SQL =
    "SELECT * FROM `clean_notify_table` WHERE `cleanLevelName` = ?";
static const std::string CLEAN_LEVEL_LOW = "clean_level_low";
static const std::string CLEAN_LEVEL_MEDIUM = "clean_level_medium";
static const std::string CLEAN_LEVEL_HIGH = "clean_level_high";
static const std::string CLEAN_LEVEL_RICH = "clean_level_rich";
constexpr int32_t CLEAN_LOW_TIME = 1 * 60; // min
constexpr int32_t CLEAN_MEDIUM_TIME = 24 * 60 * 60; // day
constexpr int32_t CLEAN_HIGH_TIME = 7 * 24 * 60 * 60; // week
constexpr int32_t CLEAN_RICH_TIME = 7 * 24 * 60 * 60; // week
}

namespace StorageDaemon {
constexpr uint32_t GLOBAL_USER_ID = 0;
constexpr uint32_t USER_ID_SIZE_VALUE = 16;
constexpr char FILE_SEPARATOR_CHAR = '/';
constexpr const char *ANCO_DIR = "/data/virt_service/rgm_hmos/anco_hmos_data/";
constexpr const char *ANCO_MEDIA_PATH = "/data/virt_service/rgm_hmos/anco_hmos_data/media/0";
constexpr const char *SERVICE_DIR_PATH = "/data/service/";

static const std::string EL1 = "el1";
static const std::string EL2 = "el2";
static const std::string EL3 = "el3";
static const std::string EL4 = "el4";
static const std::string EL5 = "el5";

enum KeyType {
    EL1_KEY = 1,
    EL2_KEY = 2,
    EL3_KEY = 3,
    EL4_KEY = 4,
    EL5_KEY = 5,
};

static std::map<std::string, KeyType> EL_DIR_MAP = {
    {EL1, EL1_KEY},
    {EL2, EL2_KEY},
    {EL3, EL3_KEY},
    {EL4, EL4_KEY},
    {EL5, EL5_KEY},
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
}
}

#endif // STORAGE_SERVICE_CONSTANTS_H