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

#ifndef MTPFS_MTP_DEVICE_H
#define MTPFS_MTP_DEVICE_H

#include "mtpfs_type_dir.h"
#include "mtpfs_type_file.h"

class MtpFsDevice {
public:
    class Capabilities {
    public:
        Capabilities() : getPartialObject_(false), sendPartialObject_(false), editObjects_(false) {}

        void SetCanGetPartialObject(bool b)
        {
            getPartialObject_ = b;
        }
        void SetCanSendPartialobject(bool b)
        {
            sendPartialObject_ = b;
        }
        void SetCanEditObjects(bool b)
        {
            editObjects_ = b;
        }

        bool CanGetPartialObject() const
        {
            return getPartialObject_;
        }
        bool CanSendPartialObject() const
        {
            return sendPartialObject_;
        }
        bool CanEditObjects() const
        {
            return editObjects_;
        }

    private:
        bool getPartialObject_;
        bool sendPartialObject_;
        bool editObjects_;
    };

    MtpFsDevice();
    ~MtpFsDevice();

    bool Connect(LIBMTP_raw_device_t *dev);
    bool Connect(int devNo = 0);
    bool Connect(const std::string &devFile);
    void Disconnect();

    void EnableMove(bool e = true)
    {
        moveEnabled_ = e;
    }

    uint64_t StorageTotalSize() const;
    uint64_t StorageFreeSize() const;

    int DirCreateNew(const std::string &path);
    int DirRemove(const std::string &path);
    int DirReName(const std::string &oldPath, const std::string &newPath);
    const MtpFsTypeDir *DirFetchContent(std::string path);

    int ReName(const std::string &oldPath, const std::string &newPath);

    int FileRead(const std::string &path, char *buf, size_t size, off_t offset);
    int FileWrite(const std::string &path, const char *buf, size_t size, off_t offset);
    int FilePull(const std::string &src, const std::string &dst);
    int FilePush(const std::string &src, const std::string &dst);
    int FileRemove(const std::string &path);
    int FileRename(const std::string &oldPath, const std::string &newPath);

    Capabilities GetCapabilities() const;

private:
    void CriticalEnter()
    {
        deviceMutex_.lock();
    }
    void CriticalLeave()
    {
        deviceMutex_.unlock();
    }

    bool EnumStorages();
    static Capabilities GetCapabilities(const MtpFsDevice &device);
    bool ConnectPriv(int devNo, const std::string &devFile);
    bool ConnectPrivJudgeErr(LIBMTP_error_number_t err);
    bool ConnectPrivInner();
    const void HandleDir(LIBMTP_file_t *content, MtpFsTypeDir *dir);
    void HandleDevNum(const std::string &devFile, int &devNo, int rawDevicesCnt, LIBMTP_raw_device_t *rawDevices);
    int ReNameInner(const std::string &oldPath, const std::string &newPath);

private:
    LIBMTP_mtpdevice_t *device_;
    Capabilities capabilities_;
    std::mutex deviceMutex_;
    MtpFsTypeDir rootDir_;
    bool moveEnabled_;
    static uint32_t rootNode_;
};

#endif // MTPFS_MTP_DEVICE_H
