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

#ifndef OHOS_STORAGE_DAEMON_VFAT_OPERATOR_H
#define OHOS_STORAGE_DAEMON_VFAT_OPERATOR_H

#include "disk_manager/volume/ivolume_operator.h"

namespace OHOS {
namespace StorageDaemon {

class VfatOperator : public IVolumeOperator {
public:
    VfatOperator() = default;
    ~VfatOperator() override = default;

    int32_t DoMount(const std::string& devPath,
                    const std::string& mountPath,
                    unsigned long mountFlags) override;
    int32_t Format(const std::string& devPath) override;
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_VFAT_OPERATOR_H
