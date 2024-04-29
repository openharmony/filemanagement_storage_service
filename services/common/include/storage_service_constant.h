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
#ifndef STORAGE_SERVICE_CONSTANTS_H
#define STORAGE_SERVICE_CONSTANTS_H

#include <map>
#include <sys/types.h>

namespace OHOS {
namespace StorageService {
const int START_USER_ID = 100;
const int MAX_USER_ID = 1099;
const int ZERO_USER_ID = 0;
const int UID_FILE_MANAGER = 1006;
const uid_t USER_ID_BASE = 200000;
}

namespace StorageDaemon {
constexpr uint32_t GLOBAL_USER_ID = 0;
constexpr uint32_t ANCO_USER_ID = 100;
constexpr char FILE_SEPARATOR_CHAR = '/';
static const std::string FILE_CONTENT_SEPARATOR = ";";
static const std::string WILDCARD_DEFAULT_INCLUDE = "*";
static const std::string BACKUP_PATH_PREFIX = "/data/service/el2/";
static const std::string BACKUP_PATH_SURFFIX = "/backup/backup_sa/";
static const std::string BACKUP_INCEXC_SYMBOL = "incExc_";
static const std::string BACKUP_STAT_SYMBOL = "stat_";
static const std::string BACKUP_INCLUDE = "INCLUDES";
static const std::string BACKUP_EXCLUDE = "EXCLUDES";
static const std::string DEFAULT_PATH_WITH_WILDCARD = "haps/*";
static const std::string BASE_EL1 = "/data/storage/el1/base/";
static const std::string BASE_EL2 = "/data/storage/el2/base/";
static const std::string PHY_APP = "/data/app/";
static const std::string BASE = "/base/";
static const std::string DEFAULT_INCLUDE_PATH_IN_HAP_FILES = "files";
static const std::string DEFAULT_INCLUDE_PATH_IN_HAP_DATABASE = "database";
static const std::string DEFAULT_INCLUDE_PATH_IN_HAP_PREFERENCE = "preferences";
static const std::string URI_PREFIX = "file://";
static const std::string NORMAL_SAND_PREFIX = "/data/storage";
static const std::string FILE_SAND_PREFIX = "/storage/Users";
static const std::string MEDIA_CLOUD_SAND_PREFIX = "/storage/cloud";
static const std::string MEDIA_SAND_PREFIX = "/storage/media";
static const std::string FILE_AUTHORITY = "docs";

// backup stat file version
static const std::string VER_10_LINE1 = "version=1.0&attrNum=8";
static const std::string VER_10_LINE2 = "path;mode;dir;size;mtime;hash;isIncremental;encodeFlag";

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
}
}

#endif // STORAGE_SERVICE_CONSTANTS_H