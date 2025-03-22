/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include <condition_variable>
#include "mtpfs_type_dir.h"
#include "mtpfs_type_file.h"
#include <map>

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
    bool ConnectByDevNo(int devNo = 0);
    bool ConnectByDevFile(const std::string &devFile);
    void Disconnect();
    void InitDevice();
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
    void AddUploadRecord(const std::string path, bool value);
    void RemoveUploadRecord(const std::string path);
    void SetUploadRecord(const std::string path, bool value);
    std::tuple<std::string, bool> FindUploadRecord(const std::string path);
    void RefreshDirContent(std::string path);

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
    bool ConvertErrorCode(LIBMTP_error_number_t err);
    const void HandleDir(LIBMTP_file_t *content, MtpFsTypeDir *dir);
    void HandleDevNum(const std::string &devFile, int &devNo, int rawDevicesCnt, LIBMTP_raw_device_t *rawDevices);
    int ReNameInner(const std::string &oldPath, const std::string &newPath);
    void ReadEvent();
    static void MtpEventCallback(int ret, LIBMTP_event_t event, uint32_t param, void *data);
    const void HandleDirByFetch(LIBMTP_file_t *content, MtpFsTypeDir *dir);

private:
    LIBMTP_mtpdevice_t *device_;
    Capabilities capabilities_;
    std::mutex deviceMutex_;
    MtpFsTypeDir rootDir_;
    bool moveEnabled_;
    static uint32_t rootNode_;
    bool eventFlag_ = true;
    std::mutex uploadRecordMutex_;
    std::map<std::string, bool> uploadRecordMap_;
    static std::condition_variable eventCon_;
    static std::mutex eventMutex_;
};

#endif // MTPFS_MTP_DEVICE_H
