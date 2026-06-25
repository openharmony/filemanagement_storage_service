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

#ifndef OHOS_STORAGE_DAEMON_UDF_OPERATOR_H
#define OHOS_STORAGE_DAEMON_UDF_OPERATOR_H

#include "disk_manager/volume/ivolume_operator.h"
#include "disk_manager/disk/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {

class UdfOperator : public IVolumeOperator {
public:
    UdfOperator() = default;
    ~UdfOperator() override = default;
    int32_t ReadMetadata(const std::string& devPath,
                         std::string& uuid,
                         std::string& type,
                         std::string& label) override;
    int32_t DoMount(const std::string& devPath,
                    const std::string& mountPath,
                    unsigned long mountFlags,
                    const std::string& mountData) override;
    int32_t CreateIsoImage(const std::string& devPath,
                           const std::string& filePath,
                           const std::string& mountPath) override;
    int32_t DoCDBurn(const std::string &devPath,
                     const BurnOptions &burnOptions,
                     bool isDiskEmpty,
                     const std::string &incBurnAddr);
    int32_t DoDVDBurn(const std::string &devPath, const BurnOptions &burnOptions, bool isDiskEmpty);
    int32_t Burn(const std::string &devPath, const BurnOptions &burnOptions) override;

private:
    int32_t PrepareIsoImage(const std::string &devPath,
                            const BurnOptions &burnOptions,
                            bool isDiskEmpty,
                            const std::string &incBurnAddr);
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_UDF_OPERATOR_H