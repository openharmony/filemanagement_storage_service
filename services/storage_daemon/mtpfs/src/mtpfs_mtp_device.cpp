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

#include "mtpfs_mtp_device.h"

#include <sstream>
#include <unistd.h>

#include "mtpfs_fuse.h"
#include "mtpfs_libmtp.h"
#include "mtpfs_util.h"
#include "storage_service_log.h"

uint32_t MtpFsDevice::rootNode_ = ~0;

MtpFsDevice::MtpFsDevice() : device_(nullptr), capabilities_(), deviceMutex_(), rootDir_(), moveEnabled_(false)
{
    MtpFsUtil::Off();
    LIBMTP_Init();
    
    MtpFsUtil::On();
}

MtpFsDevice::~MtpFsDevice()
{
    Disconnect();
}

bool MtpFsDevice::Connect(LIBMTP_raw_device_t *dev)
{
    if (device_) {
        LOGI("Already connected");
        return true;
    }

    // Do not output LIBMTP debug stuff
    MtpFsUtil::Off();
    device_ = LIBMTP_Open_Raw_Device_Uncached(dev);
    MtpFsUtil::On();

    if (!device_) {
        LIBMTP_Dump_Errorstack(device_);
        return false;
    }

    if (!EnumStorages())
        return false;

    // Retrieve capabilities.
    capabilities_ = MtpFsDevice::GetCapabilities(*this);

    LOGI("Connected");
    return true;
}

bool MtpFsDevice::ConnectPrivJudgeErr(LIBMTP_error_number_t err)
{
    if (err != LIBMTP_ERROR_NONE) {
        switch (err) {
            case LIBMTP_ERROR_NO_DEVICE_ATTACHED:
                LOGE("No raw devices found");
                break;
            case LIBMTP_ERROR_CONNECTING:
                LOGE("There has been an error connecting. Exiting");
                break;
            case LIBMTP_ERROR_MEMORY_ALLOCATION:
                LOGE("Encountered a Memory Allocation Error. Exiting");
                break;
            case LIBMTP_ERROR_GENERAL:
                LOGE("General error occurred. Exiting");
                break;
            case LIBMTP_ERROR_USB_LAYER:
                LOGE("USB Layer error occurred. Exiting");
                break;
            default:
                break;
        }
        return false;
    }
    return true;
}

bool MtpFsDevice::ConnectPrivInner()
{
    if (!device_) {
        LOGE("device_ is nullptr");
        LIBMTP_Dump_Errorstack(device_);
        return false;
    }

    if (!EnumStorages()) {
        LOGE("EnumStorages fail");
        return false;
    }
    // Retrieve capabilities.
    capabilities_ = MtpFsDevice::GetCapabilities(*this);
    LOGI("Connected");
    return true;
}

void MtpFsDevice::HandleDevNum(const std::string &devFile, int &devNo, int rawDevicesCnt,
    LIBMTP_raw_device_t *rawDevices)
{
    uint8_t bnum;
    uint8_t dnum;
    if (SmtpfsUsbDevPath(devFile, &bnum, &dnum)) {
        for (devNo = 0; devNo < rawDevicesCnt; ++devNo) {
            if (bnum == rawDevices[devNo].bus_location && dnum == rawDevices[devNo].devnum) {
                break;
            }
        }
    }
}

bool MtpFsDevice::ConnectPriv(int devNo, const std::string &devFile)
{
    if (device_) {
        LOGI("Already connected");
        return true;
    }

    int rawDevicesCnt;
    LIBMTP_raw_device_t *rawDevices;

    // Do not output LIBMTP debug stuff
    MtpFsUtil::Off();
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawDevices, &rawDevicesCnt);
    MtpFsUtil::On();

    if (!ConnectPrivJudgeErr(err)) {
        return false;
    }

    if (devNo < 0 || devNo >= rawDevicesCnt) {
        LOGE("Can not connect to device no %{public}d", devNo + 1);
        free(static_cast<void *>(rawDevices));
        return false;
    }

    LIBMTP_raw_device_t *rawDevice = &rawDevices[devNo];

    // Do not output LIBMTP debug stuff
    MtpFsUtil::Off();
    device_ = LIBMTP_Open_Raw_Device_Uncached(rawDevice);
    MtpFsUtil::On();
    free(static_cast<void *>(rawDevices));

    if (!ConnectPrivInner()) {
        return false;
    }
    return true;
}

bool MtpFsDevice::Connect(int devNo)
{
    return ConnectPriv(devNo, std::string());
}

bool MtpFsDevice::Connect(const std::string &devFile)
{
    if (device_) {
        LOGE("Already connected");
        return true;
    }

    LIBMTP_raw_device_t *rawDevice = SmtpfsRawDeviceNew(devFile);
    if (!rawDevice) {
        LOGE("Can not open such device: %{public}s", devFile.c_str());
        return false;
    }

    bool rval = Connect(rawDevice);
    SmtpfsRawDeviceFree(rawDevice);
    return rval;
}

void MtpFsDevice::Disconnect()
{
    if (!device_) {
        return;
    }
    LIBMTP_Release_Device(device_);
    device_ = nullptr;
    LOGI("Disconnected");
}

uint64_t MtpFsDevice::StorageTotalSize() const
{
    uint64_t total = 0;
    for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
        total += s->MaxCapacity;
    }
    return total;
}

uint64_t MtpFsDevice::StorageFreeSize() const
{
    uint64_t free = 0;
    for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
        free += s->FreeSpaceInBytes;
    }
    return free;
}

bool MtpFsDevice::EnumStorages()
{
    CriticalEnter();
    LIBMTP_Clear_Errorstack(device_);
    if (LIBMTP_Get_Storage(device_, LIBMTP_STORAGE_SORTBY_NOTSORTED) < 0) {
        LOGE("Could not retrieve device storage. Exiting");
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return false;
    }
    CriticalLeave();
    return true;
}

const void MtpFsDevice::HandleDir(LIBMTP_file_t *content, MtpFsTypeDir *dir)
{
    if (content == nullptr || dir == nullptr) {
        LOGE("content or dir is nullptr");
        return;
    }
    for (LIBMTP_file_t *f = content; f; f = f->next) {
        if (f->filetype == LIBMTP_FILETYPE_FOLDER) {
            dir->AddDir(MtpFsTypeDir(f));
        } else {
            dir->AddFile(MtpFsTypeFile(f));
        }
    }
}

const MtpFsTypeDir *MtpFsDevice::DirFetchContent(std::string path)
{
    if (!rootDir_.IsFetched()) {
        for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
            rootDir_.AddDir(MtpFsTypeDir(rootNode_, 0, s->id, std::string(s->StorageDescription)));
            rootDir_.SetFetched();
        }
    }

    if (rootDir_.DirCount() == 1) {
        path = '/' + rootDir_.Dirs().begin()->Name() + path;
    }
    if (path == "/") {
        return &rootDir_;
    }

    std::string member;
    std::istringstream ss(path);
    MtpFsTypeDir *dir = &rootDir_;
    while (std::getline(ss, member, '/')) {
        if (member.empty()) {
            continue;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        if (!tmp && !dir->IsFetched()) {
            CriticalEnter();
            LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(device_, dir->StorageId(), dir->Id());
            CriticalLeave();
            HandleDir(content, dir);
            LIBMTPFreeFilesAndFolders(&content);
            dir->SetFetched();
            tmp = dir->Dir(member);
        }
        if (!tmp) {
            return nullptr;
        }
        dir = const_cast<MtpFsTypeDir *>(tmp);
    }

    if (dir->IsFetched()) {
        return dir;
    }
    CriticalEnter();
    dir->SetFetched();
    LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(device_, dir->StorageId(), dir->Id());
    CriticalLeave();
    HandleDir(content, dir);
    LIBMTPFreeFilesAndFolders(&content);
    return dir;
}

int MtpFsDevice::DirCreateNew(const std::string &path)
{
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpDirName);
    if (!dirParent || dirParent->Id() == 0) {
        LOGE("Can not remove directory: %{public}s", path.c_str());
        return -EINVAL;
    }
    char *cName = strdup(tmpBaseName.c_str());
    CriticalEnter();
    uint32_t newId = LIBMTP_Create_Folder(device_, cName, dirParent->Id(), dirParent->StorageId());
    CriticalLeave();
    if (newId == 0) {
        LOGE("Could not create directory: %{public}s", path.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
    } else {
        const_cast<MtpFsTypeDir *>(dirParent)
            ->AddDir(MtpFsTypeDir(newId, dirParent->Id(), dirParent->StorageId(), tmpBaseName));
        LOGI("Directory %{public}s created", path.c_str());
    }
    free(static_cast<void *>(cName));
    return newId != 0 ? 0 : -EINVAL;
}

int MtpFsDevice::DirRemove(const std::string &path)
{
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpDirName);
    const MtpFsTypeDir *dirToRemove = dirParent ? dirParent->Dir(tmpBaseName) : nullptr;
    if (!dirParent || !dirToRemove || dirParent->Id() == 0) {
        LOGE("No such directory %{public}s to remove", path.c_str());
        return -ENOENT;
    }
    if (!dirToRemove->IsEmpty()) {
        return -ENOTEMPTY;
    }
    CriticalEnter();
    int rval = LIBMTP_Delete_Object(device_, dirToRemove->Id());
    CriticalLeave();
    if (rval != 0) {
        LOGE("Could not remove the directory: %{public}s", path.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return -EINVAL;
    }
    const_cast<MtpFsTypeDir *>(dirParent)->RemoveDir(*dirToRemove);
    LOGI("Folder %{public}s removed", path.c_str());
    return 0;
}

int MtpFsDevice::DirReName(const std::string &oldPath, const std::string &newPath)
{
    const std::string tmpOldBaseName(SmtpfsBaseName(oldPath));
    const std::string tmpOldDirName(SmtpfsDirName(oldPath));
    const std::string tmpNewBaseName(SmtpfsBaseName(newPath));
    const std::string tmpNewDirName(SmtpfsDirName(newPath));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpOldDirName);
    const MtpFsTypeDir *dirToReName = dirParent ? dirParent->Dir(tmpOldBaseName) : nullptr;
    if (!dirParent || !dirToReName || dirParent->Id() == 0) {
        LOGE("Can not rename %{public}s to %{public}s ", tmpOldBaseName.c_str(), tmpNewBaseName.c_str());
        return -EINVAL;
    }
    if (tmpOldDirName != tmpNewDirName) {
        LOGE("Can not move %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
        return -EINVAL;
    }

    LIBMTP_folder_t *folder = dirToReName->ToLIBMTPFolder();
    CriticalEnter();
    int ret = LIBMTP_Set_Folder_Name(device_, folder, tmpNewBaseName.c_str());
    CriticalLeave();
    free(static_cast<void *>(folder->name));
    free(static_cast<void *>(folder));
    if (ret != 0) {
        LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return -EINVAL;
    }
    const_cast<MtpFsTypeDir *>(dirToReName)->SetName(tmpNewBaseName);
    LOGI("Directory %{public}s renamed to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
    return 0;
}

int MtpFsDevice::ReNameInner(const std::string &oldPath, const std::string &newPath)
{
    const std::string tmpOldBaseName(SmtpfsBaseName(oldPath));
    const std::string tmpOldDirName(SmtpfsDirName(oldPath));
    const std::string tmpNewBaseName(SmtpfsBaseName(newPath));
    const std::string tmpNewDirName(SmtpfsDirName(newPath));
    const MtpFsTypeDir *dirOldParent = DirFetchContent(tmpOldDirName);
    const MtpFsTypeDir *dirNewParent = DirFetchContent(tmpNewDirName);
    const MtpFsTypeDir *dirToReName = dirOldParent ? dirOldParent->Dir(tmpOldBaseName) : nullptr;
    const MtpFsTypeFile *fileToReName = dirOldParent ? dirOldParent->File(tmpOldBaseName) : nullptr;

    LOGD("dirToReName:%{public}s", dirToReName);
    LOGD("fileToReName:%{public}s", fileToReName);

    if (!dirOldParent || !dirNewParent || dirOldParent->Id() == 0) {
        return -EINVAL;
    }
    const MtpFsTypeBasic *objectToReName = dirToReName ? static_cast<const MtpFsTypeBasic *>(dirToReName) :
                                                             static_cast<const MtpFsTypeBasic *>(fileToReName);

    LOGD("objectToReName:%{public}s", objectToReName);
    LOGD("objectToReName->id:%{public}d", objectToReName->Id());

    if (!objectToReName) {
        LOGE("No such file or directory to rename/move!");
        return -ENOENT;
    }

    if (tmpOldDirName != tmpNewDirName) {
        CriticalEnter();
        int rval =
            LIBMTP_Set_Object_u32(device_, objectToReName->Id(), LIBMTP_PROPERTY_ParentObject, dirNewParent->Id());
        CriticalLeave();
        if (rval != 0) {
            LOGE("Could not move %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
            LIBMTP_Dump_Errorstack(device_);
            LIBMTP_Clear_Errorstack(device_);
            return -EINVAL;
        }
        const_cast<MtpFsTypeBasic *>(objectToReName)->SetParent(dirNewParent->Id());
    }
    if (tmpOldBaseName != tmpNewBaseName) {
        CriticalEnter();
        int rval =
            LIBMTP_Set_Object_String(device_, objectToReName->Id(), LIBMTP_PROPERTY_Name, tmpNewBaseName.c_str());
        CriticalLeave();
        if (rval != 0) {
            LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
            LIBMTP_Dump_Errorstack(device_);
            LIBMTP_Clear_Errorstack(device_);
            return -EINVAL;
        }
    }
    return 0;
}

int MtpFsDevice::ReName(const std::string &oldPath, const std::string &newPath)
{
#ifndef SMTPFS_MOVE_BY_SET_OBJECT_PROPERTY
    const std::string tmpOldBaseName(SmtpfsBaseName(oldPath));
    const std::string tmpOldDirName(SmtpfsDirName(oldPath));
    const std::string tmpNewDirName(SmtpfsDirName(newPath));
    if (tmpOldDirName != tmpNewDirName) {
        return -EINVAL;
    }

    const MtpFsTypeDir *dirParent = DirFetchContent(tmpOldDirName);
    if (!dirParent || dirParent->Id() == 0) {
        return -EINVAL;
    }
    const MtpFsTypeDir *dirToReName = dirParent->Dir(tmpOldBaseName);
    if (dirToReName) {
        return DirReName(oldPath, newPath);
    } else {
        return FileRename(oldPath, newPath);
    }
#else
    int32_t ret = ReNameInner(oldPath, newPath);
    if (ret != 0) {
        LOGE("ReNameInner fail");
        return ret;
    }
    return 0;
#endif
}

int MtpFsDevice::FileRead(const std::string &path, char *buf, size_t size, off_t offset)
{
    const std::string pathBaseName(SmtpfsBaseName(path));
    const std::string pathDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(pathDirName);
    const MtpFsTypeFile *fileToFetch = dirParent ? dirParent->File(pathBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", path.c_str());
        return -EINVAL;
    }
    if (!fileToFetch) {
        LOGE("No such file %{public}s", path.c_str());
        return -ENOENT;
    }

    // all systems clear
    unsigned char *tmpBuf;
    unsigned int tmpSize;
    int rval = LIBMTP_GetPartialObject(device_, fileToFetch->Id(), offset, size, &tmpBuf, &tmpSize);
    if (tmpSize > 0) {
        if (memcpy_s(buf, tmpSize, tmpBuf, tmpSize) != EOK) {
            LOGE("memcpy_s tmpBuf fail");
        }
        free(tmpBuf);
    }

    if (rval != 0) {
        return -EIO;
    }
    return tmpSize;
}

int MtpFsDevice::FileWrite(const std::string &path, const char *buf, size_t size, off_t offset)
{
    const std::string pathBaseName(SmtpfsBaseName(path));
    const std::string pathDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(pathDirName);
    const MtpFsTypeFile *fileToFetch = dirParent ? dirParent->File(pathBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", path.c_str());
        return -EINVAL;
    }
    if (!fileToFetch) {
        LOGE("No such file %{public}s", path.c_str());
        return -ENOENT;
    }

    // all systems clear
    int rval = LIBMTP_SendPartialObject(device_, fileToFetch->Id(), offset, (unsigned char *)buf, size);
    if (rval < 0) {
        return -EIO;
    }
    return size;
}

int MtpFsDevice::FilePull(const std::string &src, const std::string &dst)
{
    const std::string srcBaseName(SmtpfsBaseName(src));
    const std::string srcDirName(SmtpfsDirName(src));
    const MtpFsTypeDir *dirParent = DirFetchContent(srcDirName);
    const MtpFsTypeFile *fileToFetch = dirParent ? dirParent->File(srcBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", src.c_str());
        return -EINVAL;
    }
    if (!fileToFetch) {
        LOGE("No such file %{public}s", src.c_str());
        return -ENOENT;
    }
    if (fileToFetch->Size() == 0) {
        int fd = ::creat(dst.c_str(), S_IRUSR | S_IWUSR);
        ::close(fd);
    } else {
        LOGI("Started fetching %{public}s", src.c_str());
        CriticalEnter();
        int rval = LIBMTP_Get_File_To_File(device_, fileToFetch->Id(), dst.c_str(), nullptr, nullptr);
        CriticalLeave();
        if (rval != 0) {
            LOGE("Could not fetch file %{public}s", src.c_str());
            LIBMTP_Dump_Errorstack(device_);
            LIBMTP_Clear_Errorstack(device_);
            return -ENOENT;
        }
    }
    LOGI("File fetched %{public}s", src.c_str());
    return 0;
}

int MtpFsDevice::FilePush(const std::string &src, const std::string &dst)
{
    const std::string dstBaseName(SmtpfsBaseName(dst));
    const std::string dstDirName(SmtpfsDirName(dst));
    const MtpFsTypeDir *dirParent = DirFetchContent(dstDirName);
    const MtpFsTypeFile *fileToRemove = dirParent ? dirParent->File(dstBaseName) : nullptr;
    if (dirParent && fileToRemove) {
        CriticalEnter();
        int rval = LIBMTP_Delete_Object(device_, fileToRemove->Id());
        CriticalLeave();
        if (rval != 0) {
            LOGE("Can not upload %{public}s to %{public}s", src.c_str(), dst.c_str());
            return -EINVAL;
        }
    }

    struct stat fileStat;
    stat(src.c_str(), &fileStat);
    MtpFsTypeFile fileToUpload(0, dirParent->Id(), dirParent->StorageId(), dstBaseName,
        static_cast<uint64_t>(fileStat.st_size), 0);
    LIBMTP_file_t *f = fileToUpload.ToLIBMTPFile();
    if (fileStat.st_size) {
        LOGI("Started uploading %{public}s", dst.c_str());
    }
    CriticalEnter();
    int rval = LIBMTP_Send_File_From_File(device_, src.c_str(), f, nullptr, nullptr);
    CriticalLeave();
    if (rval != 0) {
        LOGE("Could not upload file %{public}s", src.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        rval = -EINVAL;
    } else {
        fileToUpload.SetId(f->item_id);
        fileToUpload.SetParent(f->parent_id);
        fileToUpload.SetStorage(f->storage_id);
        fileToUpload.SetName(std::string(f->filename));
        fileToUpload.SetModificationDate(fileStat.st_mtime);
        if (fileToRemove) {
            const_cast<MtpFsTypeDir *>(dirParent)->ReplaceFile(*fileToRemove, fileToUpload);
        } else {
            const_cast<MtpFsTypeDir *>(dirParent)->AddFile(fileToUpload);
        }
    }
    free(static_cast<void *>(f->filename));
    free(static_cast<void *>(f));
    return rval;
}

int MtpFsDevice::FileRemove(const std::string &path)
{
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpDirName);
    const MtpFsTypeFile *fileToRemove = dirParent ? dirParent->File(tmpBaseName) : nullptr;
    if (!dirParent || !fileToRemove) {
        LOGE("No such file %{public}s to remove", path.c_str());
        return -ENOENT;
    }
    CriticalEnter();
    int rval = LIBMTP_Delete_Object(device_, fileToRemove->Id());
    CriticalLeave();
    if (rval != 0) {
        LOGE("Could not remove the directory %{public}s", path.c_str());
        return -EINVAL;
    }
    const_cast<MtpFsTypeDir *>(dirParent)->RemoveFile(*fileToRemove);
    LOGI("File %{public}s removed", path.c_str());
    return 0;
}

int MtpFsDevice::FileRename(const std::string &oldPath, const std::string &newPath)
{
    const std::string tmpOldBaseName(SmtpfsBaseName(oldPath));
    const std::string tmpOldDirName(SmtpfsDirName(oldPath));
    const std::string tmpNewBaseName(SmtpfsBaseName(newPath));
    const std::string tmpNewDirName(SmtpfsDirName(newPath));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpOldDirName);
    const MtpFsTypeFile *fileToReName = dirParent ? dirParent->File(tmpOldBaseName) : nullptr;
    if (!dirParent || !fileToReName || tmpOldDirName != tmpNewDirName) {
        LOGE("Can not rename %{public}s to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
        return -EINVAL;
    }

    LIBMTP_file_t *file = fileToReName->ToLIBMTPFile();
    CriticalEnter();
    int rval = LIBMTP_Set_File_Name(device_, file, tmpNewBaseName.c_str());
    CriticalLeave();
    free(static_cast<void *>(file->filename));
    free(static_cast<void *>(file));
    if (rval > 0) {
        LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return -EINVAL;
    }
    const_cast<MtpFsTypeFile *>(fileToReName)->SetName(tmpNewBaseName);
    LOGI("File %{public}s renamed to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
    return 0;
}

MtpFsDevice::Capabilities MtpFsDevice::GetCapabilities() const
{
    return capabilities_;
}

MtpFsDevice::Capabilities MtpFsDevice::GetCapabilities(const MtpFsDevice &device)
{
    MtpFsDevice::Capabilities capabilities;

#ifdef HAVE_LIBMTP_CHECK_CAPABILITY
    if (device.device_) {
        capabilities.SetCanGetPartialObject(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_GetPartialObject)));
        capabilities.SetCanSendPartialobject(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_SendPartialObject)));
        capabilities.SetCanEditObjects(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_EditObjects)));
    }
#endif

    return capabilities;
}
