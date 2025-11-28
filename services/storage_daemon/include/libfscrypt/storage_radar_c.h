/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef STORAGE_RADAR_C_H
#define STORAGE_RADAR_C_H

#include "hisysevent_c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGESERVICE_DOAMIN "FILEMANAGEMENT"
#define FILE_STORAGE_MANAGER_FAULT_BEHAVIOR "FILE_STORAGE_MANAGER_FAULT"
#define BIZ_STAGE_SET_POLICY 72
#define REASON_BUFFER 256
#define ASSIGNER_SIZE 10

#define RADAR_REPORT(path, reason, errorCode) ReportSetPolicyResult(path, reason, errorCode, __FUNCTION__, __LINE__)

typedef enum {
    STORAGE_START = 0,
    USER_MOUNT_MANAGER,
    USER_KEY_ENCRYPTION,
    SPACE_STATISTICS,
    EXTERNAL_VOLUME_MANAGER,
    STORAGE_USAGE_MANAGER,
    MTP_DEVICE_MANAGER,
} BizScene;

typedef enum {
    STAGE_IDLE = 0,
    STAGE_SUCC = 1,
    STAGE_FAIL = 2,
    STAGE_STOP = 3,
} StageRes;

typedef enum  {
    BIZ_STATE_START = 1,
    BIZ_STATE_END = 2,
} BizState;

void ReportSetPolicyResult(const char *path, const char *reason, int32_t errorCode, const char *func, int32_t line);
void HiSysEventParamsFree(HiSysEventParam params[], size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif // STORAGE_RADAR_C_H
