/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <map>
#include "volume/volume_info.h"

namespace OHOS {
namespace StorageDaemon {
class ExternalVolumeInfo : public VolumeInfo {
public:
    ExternalVolumeInfo() = default;
    virtual ~ExternalVolumeInfo() = default;

    virtual int32_t DoTryToFix() override;
    virtual int32_t DoTryToCheck() override;
    virtual std::string GetFsType() override;
    std::string GetFsUuid();
    std::string GetFsLabel();
    bool GetDamagedFlag();
    std::string GetMountPath();
    int32_t IsUsbInUse(int fd);
    int32_t GetLatestProgressFromFile(const char* filePath, uint32_t &progress);
protected:
    virtual int32_t DoCreate(dev_t dev) override;
    virtual int32_t DoDestroy() override;
    virtual int32_t DoMount(uint32_t mountFlags) override;
    virtual int32_t DoUMount(bool force) override;
    virtual int32_t DoUMountUsbFuse() override;
    virtual int32_t DoCheck() override;
    virtual int32_t DoFormat(std::string type) override;
    virtual int32_t DoSetVolDesc(std::string description) override;
    virtual std::string GetFsTypeByDev(dev_t dev) override;
    virtual int32_t DoGetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize) override;

    //disk crypt api
    virtual int32_t DoEncrypt(const std::string &volumeId, const std::string &pazzword) override;
    virtual int32_t DoGetCryptProgressById(const std::string &volumeId, int32_t &progress) override;
    virtual int32_t DoGetCryptUuidById(const std::string &volumeId, std::string &uuid) override;
    virtual int32_t DoBindRecoverKeyToPasswd(const std::string &volumeId,
                                             const std::string &pazzword,
                                             const std::string &recoverKey) override;
    virtual int32_t DoUpdateCryptPasswd(const std::string &volumeId,
                                        const std::string &pazzword,
                                        const std::string &newPazzword) override;
    virtual int32_t DoResetCryptPasswd(const std::string &volumeId,
                                       const std::string &recoverKey,
                                       const std::string &newPazzword) override;
    virtual int32_t DoVerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword) override;
    virtual int32_t DoUnlock(const std::string &volumeId, const std::string &pazzword) override;
    virtual int32_t DoDecrypt(const std::string &volumeId, const std::string &pazzword) override;
    virtual int32_t DoDestroyCrypt(const std::string &volumeId) override;
    virtual int32_t DoEject(const std::string &volId) override;
    virtual int32_t DoGetOpticalDriveOpsProgress(const std::string &volId, uint32_t &progress) override;
private:
    std::string devPath_;
    std::string devBackupPath_;
    std::string fsLabel_;
    std::string fsUuid_;
    std::string fsType_;
    std::string mountPath_;
    std::string mountUsbFusePath_;
    std::string mountBackupPath_;
    dev_t device_;

    const std::string devPathDir_ = "/dev/block/%s";
    const std::string mountPathDir_ = "/mnt/data/external/%s";
    const std::string mountFusePathDir_ = "/mnt/data/external_fuse/%s";
    std::vector<std::string> supportMountType_ = { "ntfs", "exfat", "vfat", "hmfs", "f2fs", "udf",
        "iso9660", "crypt_LUKS"};
    std::map<std::string, std::string> supportFormatType_ = {{"exfat", "mkfs.exfat"}, {"vfat", "newfs_msdos"}};

    int32_t ReadMetadata();
    int32_t DoMount4Ext(uint32_t mountFlags);
    int32_t DoMount4Hmfs(uint32_t mountFlags);
    int32_t DoMount4Ntfs(uint32_t mountFlags);
    int32_t DoMount4Exfat(uint32_t mountFlags);
    int32_t DoMount4Udf(uint32_t mountFlags);
    int32_t DoMount4Iso9660(uint32_t mountFlags);
    int32_t DoFix4Ntfs();
    int32_t DoFix4Exfat();
    int32_t DoMount4OtherType(uint32_t mountFlags);
    int32_t DoMount4Vfat(uint32_t mountFlags);
    int32_t DoCheck4Ntfs();
    int32_t DoCheck4Exfat();
    int32_t CreateMountPath();
    int32_t CreateFuseMountPath();
    std::string GetVolDescByNtfsLabel(std::vector<std::string> &cmd);
    std::string SplitOutputIntoLines(std::vector<std::string> &output);
    int32_t ExecuteAsyncMount(uint32_t mountFlags);
    int32_t DoUMountWithForceUsbFuse();
    int32_t ValidatePazzword(const std::string &pazzword);
    std::string GetDevPathByVolumeId(const std::string& volumeId);
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_EXTERNAL_VOLUME_INFO_H
