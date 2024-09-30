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

#ifndef OHOS_SET_FLAG_UTILS_H
#define OHOS_SET_FLAG_UTILS_H

#include <string>

namespace OHOS {
namespace StorageService {

class SetFlagUtils {
public:
    static void ParseDirAllPath();
private:
    static void ParseDirPath(const std::string &path);
    static void SetDirDelFlags(const std::string &dirpath);
    static void SetFileDelFlags(const std::string &dirpath);
};

} // namespace StorageService
} // namespace OHOS
#endif // OHOS_SET_FLAG_UTILS_H