/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef STORAGE_RADAR_H
#define STORAGE_RADAR_H

#include <string>

namespace OHOS {
namespace StorageService {
const std::string ORGPKGNAME = "storageService";
const std::string ADD_NEW_USER_BEHAVIOR = "ADD_NEW_USER_BEHAVIOR";
const std::string ACTIVE_CURRENT_USER_BEHAVIOR = "ACTIVE_CURRENT_USER_BEHAVIOR";
const std::string UMOUNT_FAIL_BEHAVIOR = "UMOUNT_FAIL_BEHAVIOR";
constexpr char STORAGESERVICE_DOAMIN[] = "STORAGE_SERVICE";
enum class BizScene : int32_t {
    STORAGE_START = 1,
};

enum class StageRes : int32_t {
    STAGE_IDLE = 0,
    STAGE_SUCC = 1,
    STAGE_FAIL = 2,
    STAGE_STOP = 3,
};

enum class BizState : int32_t {
    BIZ_STATE_START = 1,
    BIZ_STATE_END = 2,
};

enum class BizStage : int32_t {
    BIZ_STAGE_SA_START = 1,
    BIZ_STAGE_CONNECT = 2,
    BIZ_STAGE_ENABLE = 3,
    BIZ_STAGE_SA_STOP = 4,
};

class StorageRadar {
public:
    static StorageRadar &GetInstance()
    {
        static StorageRadar instance;
        return instance;
    }

public:
    bool RecordPrepareUserDirsResult(int32_t errcode);
    bool RecordActiveUserKeyResult(int32_t errcode);
    bool RecordKillProcessResult(std::string processName, int32_t errcode);

private:
    StorageRadar() = default;
    ~StorageRadar() = default;
    StorageRadar(const StorageRadar &) = delete;
    StorageRadar &operator=(const StorageRadar &) = delete;
    StorageRadar(StorageRadar &&) = delete;
    StorageRadar &operator=(StorageRadar &&) = delete;
};
} // namespace StorageService
} // namespace OHOS
#endif // STORAGE_RADAR_H