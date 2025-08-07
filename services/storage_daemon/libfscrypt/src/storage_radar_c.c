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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fscrypt_log.h"
#include "storage_radar_c.h"

const char* ERR_STR = "strdup err";

void ReportSetPolicyResult(const char *path, const char *reason, int32_t errorCode, const char *func, int32_t line)
{
    if (func == NULL || path == NULL || reason == NULL) {
        LOGE("func or extra is NUll");
        return;
    }

    BizScene scene = USER_KEY_ENCRYPTION;
    StageRes res = STAGE_SUCC;
    BizState state = BIZ_STATE_START;
    char *policy = strdup("SetPolicy");
    char *reasonStr = strdup(reason);
    char *el1 = strdup("EL1");
    char *pathStr = strdup(path);

    HiSysEventParam eventParams[ASSIGNER_SIZE] = {
        {.name = "ORG_PKG", .t = HISYSEVENT_STRING,
         .v.s = (policy == NULL ? (char*)ERR_STR : policy), .arraySize = 0},
        {.name = "USER_ID", .t = HISYSEVENT_INT32, .v.i32 = 0, .arraySize = 0},
        {.name = "FUNC", .t = HISYSEVENT_STRING,
         .v.s = (reasonStr == NULL ? (char*)ERR_STR : reasonStr), .arraySize = 0},
        {.name = "BIZ_SCENE", .t = HISYSEVENT_INT32, .v.i32 = scene, .arraySize = 0},
        {.name = "BIZ_STAGE", .t = HISYSEVENT_INT32, .v.i32 = BIZ_STAGE_SET_POLICY, .arraySize = 0},
        {.name = "KEY_ELX_LEVEL", .t = HISYSEVENT_STRING,
         .v.s = (el1 == NULL ? (char*)ERR_STR : el1), .arraySize = 0},
        {.name = "FILE_STATUS", .t = HISYSEVENT_STRING,
         .v.s = (pathStr == NULL ? (char*)ERR_STR : pathStr), .arraySize = 0},
        {.name = "STAGE_RES", .t = HISYSEVENT_INT32, .v.i32 = res, .arraySize = 0},
        {.name = "BIZ_STATE", .t = HISYSEVENT_INT32, .v.i32 = state, .arraySize = 0},
        {.name = "ERROR_CODE", .t = HISYSEVENT_INT32, .v.i32 = errorCode, .arraySize = 0}
    };
    HiSysEventEventType eventType = HISYSEVENT_FAULT;
    int32_t ret = HiSysEvent_Write(func, line, STORAGESERVICE_DOAMIN,
                                   FILE_STORAGE_MANAGER_FAULT_BEHAVIOR,
                                   eventType, eventParams, ASSIGNER_SIZE);
    if (ret != 0) {
        LOGE("StorageRadar ERROR, ret :%{public}d", ret);
    }
    HiSysEventParamsFree(eventParams, ASSIGNER_SIZE);
}

void HiSysEventParamsFree(HiSysEventParam params[], size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        if (params[i].t == HISYSEVENT_STRING && params[i].v.s != NULL && params[i].v.s != ERR_STR) {
            free(params[i].v.s);
        }
        params[i].v.s = NULL;
    }
}
