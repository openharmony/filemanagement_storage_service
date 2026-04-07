/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_VOLUME_INFO_H
#define OHOS_STORAGE_DAEMON_VOLUME_INFO_H

#include <string>
#include <sys/types.h>

namespace OHOS {
namespace StorageDaemon {
enum VolumeState {
    UNMOUNTED,
    CHECKING,
    MOUNTED,
    EJECTING,
    REMOVED,
    BADREMOVABLE,
    DAMAGED,
    FUSE_REMOVED,
    DAMAGED_MOUNTED,
    ENCRYPTING,
    ENCRYPTED_AND_LOCKED,
    ENCRYPTED_AND_UNLOCKED,
    DECRYPTING,
};

enum VolumeType {
    EXTERNAL,
};

class VolumeInfo {
public:
    VolumeInfo() = default;
    virtual ~VolumeInfo() = default;

    int32_t Create(const std::string volId, const std::string diskId, dev_t device, bool isUserdata);
    int32_t Destroy();
    void DestroyUsbFuse();
    int32_t Mount(uint32_t flags);
    int32_t UMount(bool force = false);
    int32_t UMountUsbFuse();
    int32_t Check();
    int32_t Format(const std::string type);
    int32_t SetVolumeDescription(const std::string description);
    int32_t TryToCheck();
    int32_t TryToFix();
    std::string GetVolumeId();
    int32_t GetVolumeType();
    std::string GetDiskId();
    int32_t GetState();
    void SetState(VolumeState mountState);
    bool GetIsUserdata();
    std::string GetFsTypeBase();
    int32_t GetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize);
    //disk crypt api
    int32_t DestroyCrypt(const std::string &volumeId);
    int32_t Encrypt(const std::string &volumeId, const std::string &pazzword);
    int32_t GetCryptProgressById(const std::string &volumeId, int32_t &progress);
    int32_t GetCryptUuidById(const std::string &volumeId, std::string &uuid);
    int32_t BindRecoverKeyToPasswd(const std::string &volumeId,
                                   const std::string &pazzword,
                                   const std::string &recoverKey);
    int32_t UpdateCryptPasswd(const std::string &volumeId,
                              const std::string &pazzword,
                              const std::string &newPazzword);
    int32_t ResetCryptPasswd(const std::string &volumeId,
                             const std::string &recoverKey,
                             const std::string &newPazzword);
    int32_t VerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword);
    int32_t Unlock(const std::string &volumeId, const std::string &pazzword);
    int32_t Decrypt(const std::string &volumeId, const std::string &pazzword);

protected:
    virtual int32_t DoCreate(dev_t dev) = 0;
    virtual int32_t DoDestroy() = 0;
    virtual int32_t DoMount(uint32_t mountFlags) = 0;
    virtual int32_t DoUMount(bool force) = 0;
    virtual int32_t DoUMountUsbFuse() = 0;
    virtual int32_t DoCheck() = 0;
    virtual int32_t DoFormat(std::string type) = 0;
    virtual int32_t DoSetVolDesc(std::string description) = 0;
    virtual int32_t DoTryToCheck() = 0;
    virtual int32_t DoTryToFix() = 0;
    virtual std::string GetFsTypeByDev(dev_t dev) = 0;
    virtual bool IsUsbFuseByType(std::string fsType);
    virtual std::string GetFsType() = 0;
    virtual int32_t DoGetOddCapacity(const std::string& volumeId, int64_t &totalSize, int64_t &freeSize) = 0;

    //disk crypt api
    virtual int32_t DoEncrypt(const std::string &volumeId, const std::string &pazzword) = 0;
    virtual int32_t DoGetCryptProgressById(const std::string &volumeId, int32_t &progress) = 0;
    virtual int32_t DoGetCryptUuidById(const std::string &volumeId, std::string &uuid) = 0;
    virtual int32_t DoBindRecoverKeyToPasswd(const std::string &volumeId,
                                             const std::string &pazzword,
                                             const std::string &recoverKey) = 0;
    virtual int32_t DoUpdateCryptPasswd(const std::string &volumeId,
                                        const std::string &pazzword,
                                        const std::string &newPazzword) = 0;
    virtual int32_t DoResetCryptPasswd(const std::string &volumeId,
                                       const std::string &recoverKey,
                                       const std::string &newPazzword) = 0;
    virtual int32_t DoVerifyCryptPasswd(const std::string &volumeId, const std::string &pazzword) = 0;
    virtual int32_t DoUnlock(const std::string &volumeId, const std::string &pazzword) = 0;
    virtual int32_t DoDecrypt(const std::string &volumeId, const std::string &pazzword) = 0;
    virtual int32_t DoDestroyCrypt(const std::string &volumeId) = 0;

private:
    std::string id_;
    std::string diskId_;
    VolumeType type_;
    VolumeState mountState_;
    uint32_t mountFlags_;
    int32_t userIdOwner_;
    bool isUserdata_;
    std::string fsTypeBase_;
protected:
    bool isDamaged_ = false;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_VOLUME_INFO_H