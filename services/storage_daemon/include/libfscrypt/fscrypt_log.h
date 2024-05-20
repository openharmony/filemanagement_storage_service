/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef FSCRYPT_LOG
#define FSCRYPT_LOG

#include <errno.h>

#include "hilog/log.h"

#ifndef STORAGE_LOG_TAG
#define STORAGE_LOG_TAG "StorageManager"
#endif
#define FSCRYPT_LOG_FILE "fscrypt.log"
#define FSCRYPT_LABEL "FSCRYPT"
#define FSCRYPT_LOGI(fmt, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, LOG_DOMAIN, STORAGE_LOG_TAG, \
    "[%{public}s:%{public}d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define FSCRYPT_LOGE(fmt, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, STORAGE_LOG_TAG, \
    "[%{public}s:%{public}d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))
#define FSCRYPT_LOGV(fmt, ...) \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, STORAGE_LOG_TAG, \
    "[%{public}s:%{public}d] " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__))

#define FSCRYPT_ERROR_CHECK(ret, statement, format, ...) \
    do {                                                  \
        if (!(ret)) {                                     \
            FSCRYPT_LOGE(format, ##__VA_ARGS__);             \
            statement;                                    \
        }                                                 \
    } while (0)

#define FSCRYPT_CHECK(ret, statement) \
    do {                                \
        if (!(ret)) {                  \
            statement;                 \
        }                         \
    } while (0)

#define FSCRYPT_CHECK_RETURN_VALUE(ret, result) \
    do {                                \
        if (!(ret)) {                            \
            return result;                       \
        }                                  \
    } while (0)

#endif