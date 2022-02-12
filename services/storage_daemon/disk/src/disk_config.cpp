/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "disk/disk_config.h"

#include <fnmatch.h>

#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
DiskConfig::DiskConfig(const std::string &sysPattern, const std::string &label, int flag)
{
    sysPattern_ = sysPattern;
    label_ = label;
    flag_ = flag;
}

DiskConfig::~DiskConfig()
{
}

bool DiskConfig::IsMatch(std::string &sysPattern)
{
    LOGI("config sysPattern %{public}s", sysPattern_.c_str());
    LOGI("device sysPattern %{public}s", sysPattern.c_str());

    return !fnmatch(sysPattern_.c_str(), sysPattern.c_str(), 0);
}

int DiskConfig::GetFlag() const
{
    return flag_;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
