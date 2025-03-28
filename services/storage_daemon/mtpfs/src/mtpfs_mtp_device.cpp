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

#include "mtpfs_mtp_device.h"

#include <sstream>
#include <thread>
#include <unistd.h>

#include "mtpfs_fuse.h"
#include "mtpfs_libmtp.h"
#include "mtpfs_util.h"
#include "storage_service_log.h"

const int32_t FETCH_NUM = 3000;
const int32_t DIR_COUNT_ONE = 1;
static bool g_isEventDone = true;
uint32_t MtpFsDevice::rootNode_ = ~0;
std::condition_variable MtpFsDevice::eventCon_;
std::mutex MtpFsDevice::eventMutex_;

MtpFsDevice::MtpFsDevice() : device_(nullptr), capabilities_(), deviceMutex_(), rootDir_(), moveEnabled_(false)
{
    MtpFsUtil::Off();
    LIBMTP_Init();
    g_isEventDone = true;
    MtpFsUtil::On();
}

MtpFsDevice::~MtpFsDevice()
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    LOGI("MtpFsDevice Destructor.");
    eventFlag_ = false;
    eventCon_.notify_one();
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

bool MtpFsDevice::ConvertErrorCode(LIBMTP_error_number_t err)
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

void MtpFsDevice::MtpEventCallback(int ret, LIBMTP_event_t event, uint32_t param, void *data)
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    (void)data;
    g_isEventDone = true;
    LOGI("MtpEventCallback received, ret=%{public}d, event=%{public}d, param=%{public}d, g_isEventDone=%{public}d", ret,
         event, param, g_isEventDone);
    switch (event) {
        case LIBMTP_EVENT_OBJECT_ADDED:
            LOGI("Received event LIBMTP_EVENT_OBJECT_ADDED.");
            break;
        case LIBMTP_EVENT_OBJECT_REMOVED:
            LOGI("Received event LIBMTP_EVENT_OBJECT_REMOVED.");
            break;
        default:
            break;
    }
    eventCon_.notify_one();
}

bool MtpFsDevice::ConnectByDevNo(int devNo)
{
    LOGI("Start to connect by device number = %{public}d.", devNo);
    if (device_) {
        LOGI("Already connected");
        return true;
    }
    int rawDevicesCnt;
    LIBMTP_raw_device_t *rawDevices;
    MtpFsUtil::Off();
    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawDevices, &rawDevicesCnt);
    MtpFsUtil::On();
    if (!ConvertErrorCode(err)) {
        return false;
    }

    if (devNo < 0 || devNo >= rawDevicesCnt) {
        LOGE("Can not connect to device no %{public}d", devNo + 1);
        free(static_cast<void *>(rawDevices));
        return false;
    }
    LIBMTP_raw_device_t *rawDevice = &rawDevices[devNo];

    MtpFsUtil::Off();
    device_ = LIBMTP_Open_Raw_Device_Uncached(rawDevice);
    MtpFsUtil::On();
    free(static_cast<void *>(rawDevices));
    if (!device_) {
        LOGE("device_ is nullptr");
        LIBMTP_Dump_Errorstack(device_);
        return false;
    }
    if (!EnumStorages()) {
        LOGE("EnumStorages failed.");
        return false;
    }
    capabilities_ = MtpFsDevice::GetCapabilities(*this);
    LOGI("Connect by device number success.");
    return true;
}

void MtpFsDevice::ReadEvent()
{
    while (eventFlag_) {
        std::unique_lock<std::mutex> lock(eventMutex_);
        if (g_isEventDone) {
            LOGI("Regist read mtp device event. g_isEventDone=%{public}d", g_isEventDone);
            int ret = LIBMTP_Read_Event_Async(device_, MtpEventCallback, nullptr);
            if (ret != 0) {
                LOGE("Read libmtp event async failed, ret = %{public}d", ret);
                continue;
            }
            g_isEventDone = false;
        }
        eventCon_.wait(lock, [this] { return g_isEventDone || !eventFlag_; });
    }
    LOGI("Device detached, quit read event async thread.");
}

void MtpFsDevice::InitDevice()
{
    std::thread([this]() { ReadEvent(); }).detach();
}

bool MtpFsDevice::ConnectByDevFile(const std::string &devFile)
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
    LOGI("Start to enum mtp device storages.");
    CriticalEnter();
    LIBMTP_Clear_Errorstack(device_);
    if (LIBMTP_Get_Storage(device_, LIBMTP_STORAGE_SORTBY_NOTSORTED) < 0) {
        LOGE("Could not retrieve device storage.");
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return false;
    }
    CriticalLeave();
    LOGI("Enum mtp device storages success.");
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

const void MtpFsDevice::HandleDirByFetch(LIBMTP_file_t *content, MtpFsTypeDir *dir)
{
    if (content == nullptr) {
        LOGE("directory have not any content");
        dir->dirs_.clear();
        dir->files_.clear();
        return;
    }
    if (dir == nullptr) {
        LOGE("dir is nullptr");
        return;
    }
    LOGI("HandleDir clear dir content");
    dir->dirs_.clear();
    dir->files_.clear();
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

void MtpFsDevice::RefreshDirContent(std::string path)
{
    if (!rootDir_.IsFetched()) {
        for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
            rootDir_.AddDir(MtpFsTypeDir(rootNode_, 0, s->id, std::string(s->StorageDescription)));
            rootDir_.SetFetched();
        }
    }
    if (rootDir_.DirCount() == DIR_COUNT_ONE) {
        path = '/' + rootDir_.Dirs().begin()->Name() + path;
    }
    if (path == "/") {
        return;
    }

    std::string member;
    std::istringstream ss(path);
    MtpFsTypeDir *dir = &rootDir_;
    while (std::getline(ss, member, '/')) {
        if (member.empty()) {
            LOGI("member is empty");
            continue;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        if (!tmp) {
            LOGE("tmp is nullptr");
            return;
        }
        dir = const_cast<MtpFsTypeDir *>(tmp);
    }

    uint32_t *out = (uint32_t *)malloc(sizeof(uint32_t));
    if (!out) {
        LOGE("malloc failed");
        return;
    }
    CriticalEnter();
    int32_t num = LIBMTP_Get_Children(device_, dir->StorageId(), dir->Id(), &out);
    CriticalLeave();
    free(out);
    LOGI("LIBMTP_Get_Children path=%{public}s, num=%{public}d", path.c_str(), num);
    if (num > FETCH_NUM && dir->IsFetched()) {
        LOGW("LIBMTP_Get_Children path content num over 3000 and is fetched, no need to update");
        return;
    }

    CriticalEnter();
    dir->SetFetched();
    LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(device_, dir->StorageId(), dir->Id());
    CriticalLeave();
    HandleDirByFetch(content, dir);
    LIBMTPFreeFilesAndFolders(&content);
    LOGI("LIBMTP_Get_Files_And_Folders fetchcontent end");
}

static uint64_t GetFormattedTimestamp()
{
    const int64_t secFactor = 1000;
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    uint64_t milliSeconds = static_cast<uint64_t>(millisecs.count() / secFactor);
    return milliSeconds;
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
        MtpFsTypeDir dirToUpload(newId, dirParent->Id(), dirParent->StorageId(), tmpBaseName);
        uint64_t time = GetFormattedTimestamp();
        dirToUpload.SetModificationDate(time);
        const_cast<MtpFsTypeDir *>(dirParent)->SetModificationDate(time);
        const_cast<MtpFsTypeDir *>(dirParent)->AddDir(dirToUpload);
        LOGI("Directory %{public}s created", path.c_str());
    }
    if (cName) {
        free(static_cast<void *>(cName));
    }
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
        LOGE("Directory %{public}s is not empty", path.c_str());
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
    uint64_t time = GetFormattedTimestamp();
    const_cast<MtpFsTypeDir *>(dirParent)->SetModificationDate(time);
    const_cast<MtpFsTypeDir *>(dirParent)->RemoveDir(*dirToRemove);
    LOGI("Folder %{public}s removed", path.c_str());
    return 0;
}

int MtpFsDevice::DirRemoveDirectly(const std::string &path)
{
    LOGI("DirRemoveDirectly %{public}s begin", path.c_str());
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = DirFetchContent(tmpDirName);
    const MtpFsTypeDir *dirToRemove = dirParent ? dirParent->Dir(tmpBaseName) : nullptr;
    if (!dirParent || !dirToRemove || dirParent->Id() == 0) {
        LOGE("No such directory %{public}s to remove", path.c_str());
        return -ENOENT;
    }
    if (!dirToRemove->IsEmpty()) {
        LOGI("Directory %{public}s is not empty", path.c_str());
    }
    CriticalEnter();
    LOGI("LIBMTP_Delete_Object %{public}s, Handle=%{public}u", path.c_str(), dirToRemove->Id());
    int rval = LIBMTP_Delete_Object(device_, dirToRemove->Id());
    CriticalLeave();
    if (rval != 0) {
        LOGE("Could not remove the directory: %{public}s directly", path.c_str());
        LIBMTP_Dump_Errorstack(device_);
        LIBMTP_Clear_Errorstack(device_);
        return -EINVAL;
    }
    uint64_t time = GetFormattedTimestamp();
    const_cast<MtpFsTypeDir *>(dirParent)->SetModificationDate(time);
    const_cast<MtpFsTypeDir *>(dirParent)->RemoveDir(*dirToRemove);
    LOGI("Folder %{public}s removed directly", path.c_str());
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

    if (!objectToReName) {
        LOGE("No such file or directory to rename/move!");
        return -ENOENT;
    }

    LOGD("objectToReName:%{public}s", objectToReName);
    LOGD("objectToReName->id:%{public}d", objectToReName->Id());

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
    char *tmp = const_cast<char *>(buf);
    int rval = LIBMTP_SendPartialObject(device_, fileToFetch->Id(), offset,
        reinterpret_cast<unsigned char *>(tmp), size);
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
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", dst.c_str());
        return -EINVAL;
    }
    if (fileToRemove) {
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
    LOGI("LIBMTP_Delete_Object handleID=%{public}u", fileToRemove->Id());
    int rval = LIBMTP_Delete_Object(device_, fileToRemove->Id());
    CriticalLeave();
    if (rval != 0) {
        LOGE("Could not remove the directory %{public}s", path.c_str());
        return -EINVAL;
    }
    uint64_t time = GetFormattedTimestamp();
    const_cast<MtpFsTypeDir *>(dirParent)->SetModificationDate(time);
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

void MtpFsDevice::AddUploadRecord(const std::string path, bool value)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGI("uploadRecordMap_ add %{public}s to %{public}d", path.c_str(), value);
        uploadRecordMap_[path] = value;
        return;
    }

    LOGI("uploadRecordMap_ set path %{public}s to %{public}d", path.c_str(), value);
    it->second = value;
    return;
}

void MtpFsDevice::RemoveUploadRecord(const std::string path)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGE("uploadRecordMap_ not contain %{public}s", path.c_str());
        return;
    }

    LOGI("uploadRecordMap_ remove %{public}s", path.c_str());
    uploadRecordMap_.erase(it);
    return;
}

void MtpFsDevice::SetUploadRecord(const std::string path, bool value)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGE("uploadRecordMap_ not contain %{public}s", path.c_str());
        return;
    }

    LOGI("uploadRecordMap_ set path %{public}s to %{public}d", path.c_str(), value);
    it->second = value;
    return;
}

std::tuple<std::string, bool> MtpFsDevice::FindUploadRecord(const std::string path)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGE("uploadRecordMap_ not contain %{public}s", path.c_str());
        return {"", false};
    }
    return {it->first, it->second};
}