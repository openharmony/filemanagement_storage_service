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

#include <map>
#include <string>
#include <sys/types.h>
#include "storage_service_constant.h"
namespace OHOS {
namespace StorageService {
const int START_USER_ID = 0;
const int MAX_USER_ID = 10738;
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
const char *USER_ID = "userId";
const char *BUNDLE_NAME = "bundleName";
const char *CLEAN_LEVEL = "clean_level";
const char *CLEAN_LEVEL_LOW = "clean_level_low";
const char *CLEAN_LEVEL_MEDIUM = "clean_level_medium";
const char *CLEAN_LEVEL_HIGH = "clean_level_high";
const char *CLEAN_LEVEL_RICH = "clean_level_rich";
const int32_t CLEAN_LOW_TIME = 1 * 60;
const int32_t CLEAN_MEDIUM_TIME = 24 * 60 * 60;
const int32_t CLEAN_HIGH_TIME = 7 * 24 * 60 * 60;
const int32_t CLEAN_RICH_TIME = 7 * 24 * 60 * 60;
} // namespace StorageService

namespace StorageDaemon {
const uint32_t GLOBAL_USER_ID = 0;
const uint32_t USER_ID_SIZE_VALUE = 16;
const char FILE_SEPARATOR_CHAR = '/';
const char *ANCO_DIR = "/data/virt_service/rgm_hmos/anco_hmos_data/";
const char *ANCO_MEDIA_PATH = "/data/virt_service/rgm_hmos/anco_hmos_data/media/0";
const char *SERVICE_DIR_PATH = "/data/service/";
const std::string EL1 = "el1";
const std::string EL2 = "el2";
const std::string EL3 = "el3";
const std::string EL4 = "el4";
const std::string EL5 = "el5";

const std::map<std::string, KeyType> EL_DIR_MAP = {
    {EL1, EL1_KEY},
    {EL2, EL2_KEY},
    {EL3, EL3_KEY},
    {EL4, EL4_KEY},
    {EL5, EL5_KEY},
};
} // namespace StorageDaemon
} // namespace OHOS