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

#ifndef OHOS_STORAGE_DAEMON_EXTERNAL_VOLUME_INFO_H
#define OHOS_STORAGE_DAEMON_EXTERNAL_VOLUME_INFO_H

#include <vector>
#include <sys/types.h>
#include <map>
#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
constexpr int UID_FILE_MANAGER = 1006;
class ExternalVolumeInfo : public VolumeInfo {
public:
    ExternalVolumeInfo() = default;
    virtual ~ExternalVolumeInfo() = default;

    int32_t GetFsType();
    std::string GetFsUuid();
    std::string GetFsLabel();
    std::string GetMountPath();

protected:
    virtual int32_t DoCreate(dev_t dev) override;
    virtual int32_t DoDestroy() override;
    virtual int32_t DoMount(uint32_t mountFlags) override;
    virtual int32_t DoUMount(bool force) override;
    virtual int32_t DoCheck() override;
    virtual int32_t DoFormat(std::string type) override;
    virtual int32_t DoSetVolDesc(std::string description) override;

private:
    std::string devPath_;
    std::string fsLabel_;
    std::string fsUuid_;
    std::string fsType_;
    std::string mountPath_;

    dev_t device_;

    const std::string devPathDir_ = "/dev/block/%s";
    const std::string mountPathDir_ = "/mnt/external/%s";
    std::vector<std::string> supportMountType_ = { "ext2", "ext3", "ext4", "ntfs", "exfat", "vfat" };
    std::map<std::string, std::string> supportFormatType_ = {{"exfat", "mkfs.exfat"}, {"vfat", "newfs_msdos"}};

    int32_t ReadMetadata();
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_EXTERNAL_VOLUME_INFO_H
