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

#ifndef OHOS_STORAGE_DAEMON_ISO_OPERATOR_H
#define OHOS_STORAGE_DAEMON_ISO_OPERATOR_H

#include "disk_manager/volume/ivolume_operator.h"
#include "disk_manager/disk/disk_utils.h"

namespace OHOS {
namespace StorageDaemon {

class IsoOperator : public IVolumeOperator {
public:
    IsoOperator() = default;
    ~IsoOperator() override = default;

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
    int32_t DoVerifyBurnData(const std::string &devPath, const BurnOptions &burnOptions, bool isDiskEmpty);

    int32_t PrepareVerifyMountPath();

    int32_t ExtractIsoFiles(const std::string& isoPath,
                            const std::string& sourceDir);

private:
    int32_t PrepareIsoImage(const std::string &devPath,
                            const BurnOptions &burnOptions,
                            bool isDiskEmpty,
                            const std::string &incBurnAddr);

    int32_t GenerateChecksums(const std::string& dirPath,
                              const std::string& checksumFilePath);

    std::map<std::string, std::string> ParseChecksumFile(
        const std::string& checksumContent, const std::string& basePath);

    void LogChecksumMap(const std::string& mapName,
                        const std::map<std::string, std::string>& map);

    int32_t CompareChecksums(
        const std::map<std::string, std::string>& sourceMap,
        const std::map<std::string, std::string>& discMap);

    int32_t ExecuteIsoInfoList(const std::string& isoPath,
                               std::vector<std::string>& mergedLines);
    int32_t ProcessMergedLine(const std::string& isoPath,
                              const std::string& sourceDir,
                              const std::string& line,
                              std::string& currentPath);
    int32_t PrepareSourceDirectory(const BurnOptions& burnOptions, std::string& sourceDir);
    int32_t GenerateAndCompareChecksums(const std::string& sourceDir,
                                        const std::string& sourceChecksumPath,
                                        const std::string& discChecksumPath);
};

} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_ISO_OPERATOR_H