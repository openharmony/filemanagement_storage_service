/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef STORAGE_DAEMON_UTILS_REDACTION_UTILS_H
#define STORAGE_DAEMON_UTILS_REDACTION_UTILS_H

#include <string>
#include <sys/types.h>

const std::string REDACTION_MOUNT_POINT_PREFIX = "/storage/media/";
const std::string REDACTION_MOUNT_POINT_DIR = "/local/redaction";

namespace OHOS {
namespace StorageDaemon {

class RedactionUtils{
public:
    static int32_t MountRedactionFs(int32_t userId);
    static void UMountRedactionFs(int32_t userId);
    static bool CheckRedactionFsMounted(int32_t userId);
    static bool SupportedRedactionFs();
private:
    static std::string GetRedactionMountPoint(int32_t userId);
};
}
}

#endif // STORAGE_DAEMON_UTILS_REDACTION_UTILS_H
