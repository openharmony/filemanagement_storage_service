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
    int FileMove(const std::string &oldPath, const std::string &newPath);
    int ReName(const std::string &oldPath, const std::string &newPath);
    int GetThumbnail(const std::string &path, char *buf);
    int FileRead(const std::string &path, char *buf, size_t size, off_t offset);
    int FileWrite(const std::string &path, const char *buf, size_t size, off_t offset);
    int FilePull(const std::string &src, const std::string &dst);
    int FilePush(const std::string &src, const std::string &dst);
    int FilePushAsync(const std::string src, const std::string dst);
    int FileRemove(const std::string &path);
    int FileRename(const std::string &oldPath, const std::string &newPath);
    void AddUploadRecord(const std::string path, const std::string value);
    void RemoveUploadRecord(const std::string path);
    void SetUploadRecord(const std::string path, const std::string value);
    bool IsUploadRecordEmpty();
    std::tuple<std::string, std::string> FindUploadRecord(const std::string path);
    int DirRemoveDirectly(const std::string &path);
    const MtpFsTypeDir *OpenDirFetchContent(std::string path);
    const MtpFsTypeDir *ReadDirFetchContent(std::string path);
    bool IsDirFetched(std::string path);
    Capabilities GetCapabilities() const;
    char *GetDeviceFriendlyName();
    void FreeObjectHandles(MtpFsTypeDir *dir);
    int GetDirChildren(std::string path, MtpFsTypeDir *dir, uint32_t *out);
    void HandleRemoveEvent(uint32_t handleId);
    static int AddRemovingFile(const std::string &path);
    static int EraseRemovingFile(const std::string &path);
    static bool IsFileRemoving(const std::string &path);
    void SetPtpMode(const char *mode);
    bool IsOpenHarmonyMtpDevice();

private:
    bool EnumStorages();
    static Capabilities GetCapabilities(const MtpFsDevice &device);
    bool ConvertErrorCode(LIBMTP_error_number_t err);
    const void HandleDir(LIBMTP_file_t *content, MtpFsTypeDir *dir);
    void HandleDevNum(const std::string &devFile, int &devNo, int rawDevicesCnt, LIBMTP_raw_device_t *rawDevices);
    void ReadEvent();
    static void MtpEventCallback(int ret, LIBMTP_event_t event, uint32_t param, void *data);
    static int MtpProgressCallback(uint64_t const sent, uint64_t const total, void const *const data);
    void FetchDirContent(MtpFsTypeDir *dir);
    std::map<uint32_t, std::string> FindDifferenceFds(uint32_t *newFd, int32_t newNum, uint32_t *oldFd, int32_t oldNum);
    void HandleDiffFdMap(std::map<uint32_t, std::string> &diffFdMap, MtpFsTypeDir *dir);
    void CheckDirChildren(MtpFsTypeDir *dir);
    bool UpdateFileNameByFd(const MtpFsTypeDir &fileDir, uint32_t fileFd, LIBMTP_file_t *file);
    int PerformUpload(const std::string &src, const std::string &dst, const MtpFsTypeDir *dirParent,
                      const MtpFsTypeFile *fileToRemove, const std::string &dstBaseName);
    void SetFetched(MtpFsTypeDir *dir);
    void DumpLibMtpErrorStack();
    int GetMainMtpErrorCode();
    static void SetTransferValue(bool value);

private:
    LIBMTP_mtpdevice_t *device_;
    Capabilities capabilities_;
    std::mutex deviceMutex_;
    MtpFsTypeDir rootDir_;
    bool isPtp_;
    bool moveEnabled_;
    static uint32_t rootNode_;
    std::mutex uploadRecordMutex_;
    std::map<std::string, std::string> uploadRecordMap_;
    static std::condition_variable eventCon_;
    static std::mutex eventMutex_;
    std::string rootDirName_;
    std::atomic<bool> eventFlag_;
    static std::mutex setMutex_;
    static std::set<std::string> removingFileSet_;
};

#endif // MTPFS_MTP_DEVICE_H
