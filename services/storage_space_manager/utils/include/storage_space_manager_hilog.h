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

#ifndef OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_HILOG_H
#define OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_HILOG_H

#include "hilog/log.h"
#include <string>

#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0xD004302
#endif
#ifndef STORAGE_SPACE_MANAGER_LOG_TAG
#define STORAGE_SPACE_MANAGER_LOG_TAG "StorageSpaceManager"
#endif

inline std::string GetFileNameFromFullPath(const char *str)
{
    std::string fullPath(str);
    size_t pos = fullPath.find_last_of("/");
    return (pos == std::string::npos) ? std::string() : fullPath.substr(pos + 1);
}

#define LOGD(fmt, ...)                                                                                                 \
    ((void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOG_DOMAIN, STORAGE_SPACE_MANAGER_LOG_TAG,                                  \
                      "[%{public}s:%{public}d->%{public}s] " fmt, GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__))

#define LOGI(fmt, ...)                                                                                                 \
    ((void)HILOG_IMPL(LOG_CORE, LOG_INFO, LOG_DOMAIN, STORAGE_SPACE_MANAGER_LOG_TAG,                                   \
                      "[%{public}s:%{public}d->%{public}s] " fmt, GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__))

#define LOGW(fmt, ...)                                                                                                 \
    ((void)HILOG_IMPL(LOG_CORE, LOG_WARN, LOG_DOMAIN, STORAGE_SPACE_MANAGER_LOG_TAG,                                   \
                      "[%{public}s:%{public}d->%{public}s] " fmt, GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__))

#define LOGE(fmt, ...)                                                                                                 \
    ((void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOG_DOMAIN, STORAGE_SPACE_MANAGER_LOG_TAG,                                  \
                      "[%{public}s:%{public}d->%{public}s] " fmt, GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__))

#define LOGF(fmt, ...)                                                                                                 \
    ((void)HILOG_IMPL(LOG_CORE, LOG_FATAL, LOG_DOMAIN, STORAGE_SPACE_MANAGER_LOG_TAG,                                  \
                      "[%{public}s:%{public}d->%{public}s] " fmt, GetFileNameFromFullPath(__FILE__).c_str(), __LINE__, \
                      __FUNCTION__, ##__VA_ARGS__))

#endif // OHOS_FILEMANAGEMENT_STORAGE_SPACE_MANAGER_HILOG_H