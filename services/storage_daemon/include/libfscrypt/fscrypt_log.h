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

#include "init_log.h"

#define FSCRYPT_LOG_FILE "fscrypt.log"
#define FSCRYPT_LABEL "FSCRYPT"
#define FSCRYPT_LOGI(fmt, ...) INIT_LOGI(fmt, ##__VA_ARGS__)
#define FSCRYPT_LOGE(fmt, ...) INIT_LOGE(fmt, ##__VA_ARGS__)
#define FSCRYPT_LOGV(fmt, ...) INIT_LOGV(fmt, ##__VA_ARGS__)

#endif