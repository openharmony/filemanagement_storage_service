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
const std::string DEFAULT_ORGPKGNAME = "storageService";
const std::string ADD_NEW_USER_BEHAVIOR = "ADD_NEW_USER_BEHAVIOR";
const std::string FILE_STORAGE_MANAGER_FAULT_BEHAVIOR  = "FILE_STORAGE_MANAGER_FAULT";
constexpr char STORAGESERVICE_DOAMIN[] = "FILEMANAGEMENT";
const int32_t DEFAULT_USERID = 100;
enum class BizScene : int32_t {
    STORAGE_START = 0,
    USER_MOUNT_MANAGER,
    USER_UMOUNT_MANAGER,
    USER_KEY_ENCRYPTION,
    SPACE_STATISTICS,
    EXTERNAL_VOLUME_MANAGER,
    STORAGE_USAGE_MANAGER,
    DISTRIBUTED_FILE,
    USER_DIR_MANAGER,
    PROCESS_MANAGER,
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

    BIZ_STAGE_PREPARE_ADD_USER = 11,
    BIZ_STAGE_START_USER,
    BIZ_STAGE_STOP_USER,
    BIZ_STAGE_REMOVE_USER,

    BIZ_STAGE_GENERATE_USER_KEYS = 20,
    BIZ_STAGE_ACTIVE_USER_KEY,
    BIZ_STAGE_UPDATE_USER_AUTH,
    BIZ_STAGE_INACTIVE_USER_KEY,
    BIZ_STAGE_DELETE_USER_KEYS,
    BIZ_STAGE_CREATE_RECOVERY_KEY,
    BIZ_STAGE_LOCK_USER_SCREEN,
    BIZ_STAGE_UNLOCK_USER_SCREEN,
    BIZ_STAGE_GET_FILE_ENCRYPT_STATUS,
    BIZ_STAGE_UPDATE_KEY_CONTEXT,
    BIZ_STAGE_INIT_GLOBAL_KEY,

    BIZ_STAGE_GET_TOTAL_SIZE = 31,
    BIZ_STAGE_GET_FREE_SIZE,
    BIZ_STAGE_GET_SYSTEM_SIZE,
    BIZ_STAGE_GET_BUNDLE_STATS,
    BIZ_STAGE_GET_USER_STORAGE_STATS,

    BIZ_STAGE_MOUNT = 41,
    BIZ_STAGE_UNMOUNT,
    BIZ_STAGE_PARTITION,
    BIZ_STAGE_FORMAT,
    BIZ_STAGE_SET_VOLUME_DESCRIPTION,
    BIZ_STAGE_GET_ALL_VOLUMES,

    BIZ_STAGE_THRESHOLD_TEN_PERCENT = 51,
    BIZ_STAGE_THRESHOLD_FIVE_PERCENT,
    BIZ_STAGE_THRESHOLD_MINIMAL,

    BIZ_STAGE_USER_MOUNT = 61,
    BIZ_STAGE_USER_UMOUNT = 62,
    BIZ_STAGE_USER_DIR = 63,
    BIZ_STAGE_FIND_PROCESS = 64,
};

struct RadarParameter {
    std::string orgPkg;
    int32_t userId;
    std::string funcName;
    enum BizScene bizScene;
    enum BizStage bizStage;
    std::string keyElxLevel;
    int32_t errorCode;
    std::string extraData;
};

class StorageRadar {
public:
    static StorageRadar &GetInstance()
    {
        static StorageRadar instance;
        return instance;
    }

public:
    bool RecordFindProcess(const std::string &extraData, int32_t errcode);
    void RecordMountFail(const std::string &extraData, int32_t errcode);
    void RecordUMountFail(const std::string &extraData, int32_t errcode);
    void RecordPrepareDirFail(const std::string &extraData, int32_t errcode);
    bool RecordFuctionResult(const RadarParameter &parameterRes);
    static void ReportActiveUserKey(const std::string &funcName, uint32_t userId, int ret,
        const std::string &keyElxLevel);
    static void ReportGetStorageStatus(const std::string &funcName, uint32_t userId, int ret,
        const std::string &orgPkg);
    static void ReportVolumeOperation(const std::string &funcName, int ret);
    static void ReportUserKeyResult(const std::string &funcName, uint32_t userId, int ret,
        const std::string &keyElxLevel, const std::string &extraData);
    static void ReportUserManager(const std::string &funcName, uint32_t userId, int ret, enum BizStage stage);
    static void ReportUpdateUserAuth(const std::string &funcName, uint32_t userId, int ret, const std::string &keyLevel,
        const std::string &extraData);
    static void ReportFbexResult(const std::string &funcName, uint32_t userId, int ret, const std::string &keyLevel,
        const std::string &extraData);
    static void ReportIamResult(const std::string &funcName, uint32_t userId, int ret);
    static void ReportHuksResult(const std::string &funcName, int ret);
    static void ReportStorageUsage(enum BizStage stage, const std::string &extraData);

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