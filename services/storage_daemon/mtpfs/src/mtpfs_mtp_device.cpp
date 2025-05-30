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
#include <unordered_set>

#include "mtpfs_fuse.h"
#include "mtpfs_libmtp.h"
#include "mtpfs_util.h"
#include "storage_radar.h"
#include "storage_service_log.h"

const int32_t FETCH_NUM = 3000;
const int32_t DIR_COUNT_ONE = 1;
const uint32_t DEFAULT_COUNT = 100;
const uint32_t PTP_ID_START = 300000000;
const uint32_t PTP_ID_INDEX = 200000000;
uint32_t MtpFsDevice::rootNode_ = ~0;
static std::atomic<bool> g_isEventDone;
static std::atomic<bool> isTransferring_;
std::condition_variable MtpFsDevice::eventCon_;
std::mutex MtpFsDevice::eventMutex_;
static const std::string NO_ERROR_PATH = "/FileManagerExternalStorageReadOnlyFlag";
using namespace OHOS::StorageService;

MtpFsDevice::MtpFsDevice() : device_(nullptr), capabilities_(), rootDir_(), moveEnabled_(false)
{
    MtpFsUtil::Off();
    LIBMTP_Init();
    MtpFsUtil::On();
    g_isEventDone.store(true);
    isTransferring_.store(false);
    eventFlag_.store(true);
}

MtpFsDevice::~MtpFsDevice()
{
    LOGI("MtpFsDevice Destructor.");
    eventFlag_.store(false);
    SetTransferValue(false);
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
        DumpLibMtpErrorStack();
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
    g_isEventDone.store(true);
    LOGI("MtpEventCallback received, ret=%{public}d, event=%{public}d, param=%{public}d, g_isEventDone=%{public}d", ret,
         event, param, g_isEventDone.load());
    switch (event) {
        case LIBMTP_EVENT_OBJECT_ADDED:
            LOGI("Received event LIBMTP_EVENT_OBJECT_ADDED.");
            break;
        case LIBMTP_EVENT_OBJECT_REMOVED:
            LOGI("Received event LIBMTP_EVENT_OBJECT_REMOVED.");
            if (param > PTP_ID_START) {
                LOGI("It's HO 5.x version and PTP mode");
                uint32_t handleId = param - PTP_ID_INDEX;
                DelayedSingleton<MtpFileSystem>::GetInstance()->HandleRemove(handleId);
            }
            break;
        case LIBMTP_EVENT_NONE:
            LOGI("Received event LIBMTP_EVENT_NONE.");
            break;
        default:
            break;
    }
    eventCon_.notify_one();
}

int MtpFsDevice::MtpProgressCallback(uint64_t const sent, uint64_t const total, void const *const data)
{
    const char *charData = static_cast<const char*>(data);
    LOGD("MtpProgressCallback enter, sent=%{public}lld, total=%{public}lld, data=%{public}s", sent, total, charData);
    return E_OK;
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
        DumpLibMtpErrorStack();
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
    while (eventFlag_.load()) {
        std::unique_lock<std::mutex> lock(eventMutex_);
        eventCon_.wait(lock, [this] { return !isTransferring_.load() || !eventFlag_.load(); });
        if (!eventFlag_.load()) {
            break;
        }
        if (g_isEventDone.load()) {
            LOGI("Registering MTP event callback");
            int ret = LIBMTP_Read_Event_Async(device_, MtpEventCallback, nullptr);
            if (ret != 0) {
                LOGE("Event registration failed, ret=%{public}d", ret);
                continue;
            }
            g_isEventDone.store(false);
        }
        eventCon_.wait(lock, [this] { return g_isEventDone.load() || !eventFlag_.load(); });
    }
    LOGI("Event thread terminated");
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
    std::unique_lock<std::mutex> lock(deviceMutex_);
    if (LIBMTP_Get_Storage(device_, LIBMTP_STORAGE_SORTBY_NOTSORTED) < 0) {
        LOGE("Could not retrieve device storage.");
        DumpLibMtpErrorStack();
        return false;
    }
    LOGI("Enum mtp device storages success.");
    return true;
}

const void MtpFsDevice::HandleDir(LIBMTP_file_t *content, MtpFsTypeDir *dir)
{
    if (content == nullptr) {
        LOGE("directory have not any content");
        return;
    }
    if (dir == nullptr) {
        LOGE("dir is nullptr");
        return;
    }
    LOGI("HandleDir content");
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
            if (rootDir_.Dirs().size() != 0) {
                rootDirName_ = rootDir_.Dirs().begin()->Name();
            }
            rootDir_.SetFetched(true);
        }
    }

    if (rootDir_.DirCount() == 1) {
        path = '/' + rootDirName_ + path;
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
        if (dir == nullptr) {
            return nullptr;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        if (!tmp && !dir->IsFetched()) {
            std::unique_lock<std::mutex> lock(deviceMutex_);
            LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(device_, dir->StorageId(), dir->Id());
            HandleDir(content, dir);
            LIBMTPFreeFilesAndFolders(&content);
            dir->SetFetched(true);
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
    {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        dir->SetFetched(true);
        LIBMTP_file_t *content = LIBMTP_Get_Files_And_Folders(device_, dir->StorageId(), dir->Id());
        HandleDir(content, dir);
        LIBMTPFreeFilesAndFolders(&content);
    }
    return dir;
}

const MtpFsTypeDir *MtpFsDevice::OpenDirFetchContent(std::string path)
{
    if (!rootDir_.IsFetched()) {
        for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
            rootDir_.AddDir(MtpFsTypeDir(rootNode_, 0, s->id, std::string(s->StorageDescription)));
            if (rootDir_.Dirs().size() != 0) {
                rootDirName_ = rootDir_.Dirs().begin()->Name();
            }
            rootDir_.SetFetched(true);
        }
    }
    if (rootDir_.DirCount() == 1) {
        path = '/' + rootDirName_ + path;
    }

    std::string member;
    std::istringstream ss(path);
    MtpFsTypeDir *dir = &rootDir_;
    while (std::getline(ss, member, '/')) {
        if (member.empty()) {
            continue;
        }
        if (dir == nullptr) {
            return nullptr;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        if (!tmp) {
            return nullptr;
        }
        dir = const_cast<MtpFsTypeDir *>(tmp);
    }

    if (dir->objHandles != nullptr) {
        CheckDirChildren(dir);
    }

    if (!dir->IsFetched()) {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        FetchDirContent(dir);
    }
    return dir;
}

void MtpFsDevice::CheckDirChildren(MtpFsTypeDir *dir)
{
    if (dir == nullptr) {
        LOGI("dir is nullptr");
        return;
    }
    uint32_t *out;
    int32_t childrenNum = 0;
    {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        childrenNum = LIBMTP_Get_Children(device_, dir->StorageId(), dir->Id(), &out);
        LOGI("LIBMTP_Get_Children end, childrenNum=%{public}d, cacheNum=%{public}u", childrenNum, dir->objHandles->num);
    }
    if (childrenNum < 0) {
        LOGE("LIBMTP_Get_Children fail");
        DumpLibMtpErrorStack();
        return;
    }
    auto diffFdMap = FindDifferenceFds(out, childrenNum, dir->objHandles->handler, dir->objHandles->num);
    if (childrenNum == 0) {
        dir->Clear();
        dir->objHandles->num = 0;
        dir->objHandles->offset = 0;
        LOGI("childrenNum is 0, clear dir=%{public}s content", dir->Name().c_str());
        return;
    }
    if (!diffFdMap.empty()) {
        HandleDiffFdMap(diffFdMap, dir);
        if (dir->objHandles->handler != nullptr) {
            free(dir->objHandles->handler);
            dir->objHandles->handler = nullptr;
            dir->objHandles->num = childrenNum;
            LOGI("update objHandles");
            dir->objHandles->handler = (uint32_t *)malloc(childrenNum * sizeof(uint32_t));
            if (dir->objHandles->handler == nullptr) {
                free(out);
                return;
            }
            if (memcpy_s(dir->objHandles->handler, childrenNum * sizeof(uint32_t), out,
                childrenNum * sizeof(uint32_t)) != EOK) {
                LOGE("memcpy_s fail");
            }
        }
    }
    free(out);
}

void MtpFsDevice::FreeObjectHandles(MtpFsTypeDir *dir)
{
    if (dir->objHandles) {
        if (dir->objHandles->handler) {
            free(dir->objHandles->handler);
            dir->objHandles->handler = nullptr;
        }
        free(dir->objHandles);
        dir->objHandles = nullptr;
    }
}

void MtpFsDevice::FetchDirContent(MtpFsTypeDir *dir)
{
    if (!dir || !device_) {
        LOGE("Invalid dir or device");
        return;
    }

    if (dir->objHandles == nullptr) {
        dir->objHandles = LIBMTP_Get_Object_Handles(device_, dir->StorageId(), dir->Id());
    }

    if (dir->objHandles == nullptr || dir->objHandles->handler == nullptr || dir->objHandles->num == 0) {
        dir->SetFetched(true);
        return;
    }
    LIBMTP_file_t *content = LIBMTP_Get_Patial_Files_Metadata(device_, dir->objHandles, DEFAULT_COUNT);
    HandleDir(content, dir);
    if (dir->objHandles->offset >= dir->objHandles->num) {
        dir->SetFetched(true);
    }
    LIBMTPFreeFilesAndFolders(&content);
}

const MtpFsTypeDir *MtpFsDevice::ReadDirFetchContent(std::string path)
{
    if (!rootDir_.IsFetched()) {
        for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
            rootDir_.AddDir(MtpFsTypeDir(rootNode_, 0, s->id, std::string(s->StorageDescription)));
            if (rootDir_.Dirs().size() != 0) {
                rootDirName_ = rootDir_.Dirs().begin()->Name();
            }
            rootDir_.SetFetched(true);
        }
    }
    if (rootDir_.DirCount() == 1) {
        path = '/' + rootDirName_ + path;
    }

    std::string member;
    std::istringstream ss(path);
    MtpFsTypeDir *dir = &rootDir_;
    while (std::getline(ss, member, '/')) {
        if (member.empty()) {
            continue;
        }
        if (dir == nullptr) {
            return nullptr;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        dir = const_cast<MtpFsTypeDir *>(tmp);
    }
    return dir;
}

bool MtpFsDevice::IsDirFetched(std::string path)
{
    if (!rootDir_.IsFetched()) {
        for (LIBMTP_devicestorage_t *s = device_->storage; s; s = s->next) {
            rootDir_.AddDir(MtpFsTypeDir(rootNode_, 0, s->id, std::string(s->StorageDescription)));
            if (rootDir_.Dirs().size() != 0) {
                rootDirName_ = rootDir_.Dirs().begin()->Name();
            }
            rootDir_.SetFetched(true);
        }
    }
    if (rootDir_.DirCount() == 1) {
        path = '/' + rootDirName_ + path;
    }
    if (path == "/") {
        return true;
    }
    std::string member;
    std::istringstream ss(path);
    MtpFsTypeDir *dir = &rootDir_;
    while (std::getline(ss, member, '/')) {
        if (member.empty()) {
            continue;
        }
        if (dir == nullptr) {
            return false;
        }
        const MtpFsTypeDir *tmp = dir->Dir(member);
        dir = const_cast<MtpFsTypeDir *>(tmp);
    }
    if (dir == nullptr) {
        return false;
    }
    return dir->IsFetched();
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpDirName);
    if (!dirParent || dirParent->Id() == 0) {
        LOGE("Can not remove directory: %{public}s", path.c_str());
        return -EINVAL;
    }
    char *cName = strdup(tmpBaseName.c_str());
    std::unique_lock<std::mutex> lock(deviceMutex_);
    uint32_t newId = LIBMTP_Create_Folder(device_, cName, dirParent->Id(), dirParent->StorageId());
    if (newId == 0) {
        StorageRadar::ReportMtpfsResult("DirCreateNew::LIBMTP_Create_Folder", newId, "path=" + path);
        LOGE("Could not create directory: %{public}s", path.c_str());
        DumpLibMtpErrorStack();
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpDirName);
    const MtpFsTypeDir *dirToRemove = dirParent ? dirParent->Dir(tmpBaseName) : nullptr;
    if (!dirParent || !dirToRemove || dirParent->Id() == 0) {
        LOGE("No such directory %{public}s to remove", path.c_str());
        return -ENOENT;
    }
    if (!dirToRemove->IsEmpty()) {
        LOGE("Directory %{public}s is not empty", path.c_str());
        return -ENOTEMPTY;
    }
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_Delete_Object(device_, dirToRemove->Id());
    if (rval != 0) {
        StorageRadar::ReportMtpfsResult("DirRemove::LIBMTP_Delete_Object", rval, "path=" + path);
        LOGE("Could not remove the directory: %{public}s", path.c_str());
        DumpLibMtpErrorStack();
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpDirName);
    const MtpFsTypeDir *dirToRemove = dirParent ? dirParent->Dir(tmpBaseName) : nullptr;
    if (!dirParent || !dirToRemove || dirParent->Id() == 0) {
        LOGE("No such directory %{public}s to remove", path.c_str());
        return -ENOENT;
    }
    if (!dirToRemove->IsEmpty()) {
        LOGI("Directory %{public}s is not empty", path.c_str());
    }
    LOGI("LIBMTP_Delete_Object %{public}s, Handle=%{public}u", path.c_str(), dirToRemove->Id());
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_Delete_Object(device_, dirToRemove->Id());
    if (rval != 0) {
        LOGE("Could not remove the directory: %{public}s directly", path.c_str());
        DumpLibMtpErrorStack();
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpOldDirName);
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
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int ret = LIBMTP_Set_Folder_Name(device_, folder, tmpNewBaseName.c_str());
    free(static_cast<void *>(folder->name));
    free(static_cast<void *>(folder));
    if (ret != 0) {
        std::string extraData = "oldPath=" + oldPath + "newPath=" + newPath;
        StorageRadar::ReportMtpfsResult("DirReName::LIBMTP_Set_Folder_Name", ret, extraData);
        LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
        DumpLibMtpErrorStack();
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
    const MtpFsTypeDir *dirOldParent = ReadDirFetchContent(tmpOldDirName);
    const MtpFsTypeDir *dirNewParent = ReadDirFetchContent(tmpNewDirName);
    const MtpFsTypeDir *dirToReName = dirOldParent ? dirOldParent->Dir(tmpOldBaseName) : nullptr;
    const MtpFsTypeFile *fileToReName = dirOldParent ? dirOldParent->File(tmpOldBaseName) : nullptr;

    LOGD("dirToReName:%{public}s", dirToReName);
    LOGD("fileToReName:%{public}s", fileToReName);

    if (!dirOldParent || !dirNewParent || dirOldParent->Id() == 0) {
        return -EINVAL;
    }
    const MtpFsTypeBasic *objToReName = dirToReName ? static_cast<const MtpFsTypeBasic *>(dirToReName) :
        static_cast<const MtpFsTypeBasic *>(fileToReName);

    if (!objToReName) {
        LOGE("No such file or directory to rename/move!");
        return -ENOENT;
    }

    LOGD("objToReName:%{public}s", objToReName);
    LOGD("objToReName->id:%{public}d", objToReName->Id());

    if (tmpOldDirName != tmpNewDirName) {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        int rval = LIBMTP_Set_Object_u32(device_, objToReName->Id(), LIBMTP_PROPERTY_ParentObject, dirNewParent->Id());
        if (rval != 0) {
            LOGE("Could not move %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
            DumpLibMtpErrorStack();
            return -EINVAL;
        }
        const_cast<MtpFsTypeBasic *>(objToReName)->SetParent(dirNewParent->Id());
    }
    if (tmpOldBaseName != tmpNewBaseName) {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        int rval = LIBMTP_Set_Object_String(device_, objToReName->Id(), LIBMTP_PROPERTY_Name, tmpNewBaseName.c_str());
        if (rval != 0) {
            LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
            DumpLibMtpErrorStack();
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

    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpOldDirName);
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(pathDirName);
    const MtpFsTypeFile *fileToFetch = dirParent ? dirParent->File(pathBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", path.c_str());
        return -EINVAL;
    }
    if (!fileToFetch) {
        LOGE("No such file %{public}s", path.c_str());
        return -ENOENT;
    }
    SetTransferValue(true);
    LOGI("Start file reading for path=%{public}s", path.c_str());

    unsigned char *tmpBuf;
    unsigned int tmpSize;
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_GetPartialObject(device_, fileToFetch->Id(), offset, size, &tmpBuf, &tmpSize);
    if (tmpSize > 0) {
        if (memcpy_s(buf, tmpSize, tmpBuf, tmpSize) != EOK) {
            LOGE("memcpy_s tmpBuf fail");
        }
        free(tmpBuf);
    }
    SetTransferValue(false);
    if (rval != 0) {
        DumpLibMtpErrorStack();
        return -EIO;
    }
    return tmpSize;
}

int MtpFsDevice::FileWrite(const std::string &path, const char *buf, size_t size, off_t offset)
{
    const std::string pathBaseName(SmtpfsBaseName(path));
    const std::string pathDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(pathDirName);
    const MtpFsTypeFile *fetchFile = dirParent ? dirParent->File(pathBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", path.c_str());
        return -EINVAL;
    }
    if (!fetchFile) {
        LOGE("No such file %{public}s", path.c_str());
        return -ENOENT;
    }
    SetTransferValue(true);
    LOGI("Start file writing for path=%{public}s", path.c_str());

    char *tmp = const_cast<char *>(buf);
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_SendPartialObject(device_, fetchFile->Id(), offset, reinterpret_cast<unsigned char *>(tmp), size);
    SetTransferValue(false);
    if (rval < 0) {
        DumpLibMtpErrorStack();
        return -EIO;
    }
    return size;
}

int MtpFsDevice::FilePull(const std::string &src, const std::string &dst)
{
    const std::string srcBaseName(SmtpfsBaseName(src));
    const std::string srcDirName(SmtpfsDirName(src));
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(srcDirName);
    const MtpFsTypeFile *fileToFetch = dirParent ? dirParent->File(srcBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", src.c_str());
        return -EINVAL;
    }
    if (!fileToFetch) {
        LOGE("No such file %{public}s", src.c_str());
        return -ENOENT;
    }
    SetTransferValue(true);
    if (fileToFetch->Size() == 0) {
        int fd = ::creat(dst.c_str(), S_IRUSR | S_IWUSR);
        ::close(fd);
    } else {
        LOGI("Started fetching %{public}s", src.c_str());
        std::unique_lock<std::mutex> lock(deviceMutex_);
        int rval = LIBMTP_Get_File_To_File(device_, fileToFetch->Id(), dst.c_str(), MtpProgressCallback, src.c_str());
        if (rval != 0) {
            std::string extraData = "src=" + src + "dst=" + dst;
            StorageRadar::ReportMtpfsResult("FilePull::LIBMTP_Get_File_To_File", rval, extraData);
            LOGE("Could not fetch file %{public}s", src.c_str());
            SetTransferValue(false);
            DumpLibMtpErrorStack();
            return -ENOENT;
        }
    }
    SetTransferValue(false);
    LOGI("File fetched %{public}s", src.c_str());
    return 0;
}

int MtpFsDevice::FilePush(const std::string &src, const std::string &dst)
{
    const std::string dstBaseName(SmtpfsBaseName(dst));
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(SmtpfsDirName(dst));
    const MtpFsTypeFile *fileToRemove = dirParent ? dirParent->File(dstBaseName) : nullptr;
    if (!dirParent) {
        LOGE("Can not fetch %{public}s", dst.c_str());
        return -EINVAL;
    }
    SetTransferValue(true);
    if (fileToRemove) {
        std::unique_lock<std::mutex> lock(deviceMutex_);
        int rval = LIBMTP_Delete_Object(device_, fileToRemove->Id());
        if (rval != 0) {
            StorageRadar::ReportMtpfsResult("FilePush::LIBMTP_Delete_Object", rval, "src=" + src + "dst=" + dst);
            LOGE("Can not upload %{public}s to %{public}s", src.c_str(), dst.c_str());
            SetTransferValue(false);
            DumpLibMtpErrorStack();
            return -EINVAL;
        }
    }
    int uploadRval = PerformUpload(src, dst, dirParent, fileToRemove, dstBaseName);
    SetTransferValue(false);
    return uploadRval;
}

int MtpFsDevice::PerformUpload(const std::string &src, const std::string &dst, const MtpFsTypeDir *dirParent,
                               const MtpFsTypeFile *fileToRemove, const std::string &dstBaseName)
{
    struct stat fileStat;
    stat(src.c_str(), &fileStat);
    MtpFsTypeFile fileToUpload(0, dirParent->Id(), dirParent->StorageId(), dstBaseName,
        static_cast<uint64_t>(fileStat.st_size), 0);
    LIBMTP_file_t *f = fileToUpload.ToLIBMTPFile();
    LOGI("Started uploading %{public}s, st_size=%{public}s", dst.c_str(), std::to_string(fileStat.st_size).c_str());
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_Send_File_From_File(device_, src.c_str(), f, MtpProgressCallback, dst.c_str());
    if (rval != 0) {
        if (dst != NO_ERROR_PATH) {
            StorageRadar::ReportMtpfsResult("FilePush::LIBMTP_Send_File_From_File", rval, "src=" + src + "dst=" + dst);
        }
        LOGE("Could not upload file %{public}s", src.c_str());
        DumpLibMtpErrorStack();
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
    SetTransferValue(false);
    free(static_cast<void *>(f->filename));
    free(static_cast<void *>(f));
    return (rval != 0 ? -EINVAL : rval);
}

int MtpFsDevice::FileRemove(const std::string &path)
{
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpDirName);
    const MtpFsTypeFile *fileToRemove = dirParent ? dirParent->File(tmpBaseName) : nullptr;
    if (!dirParent || !fileToRemove) {
        LOGE("No such file %{public}s to remove", path.c_str());
        return -ENOENT;
    }

    LOGI("LIBMTP_Delete_Object handleID=%{public}u", fileToRemove->Id());
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_Delete_Object(device_, fileToRemove->Id());
    if (rval != 0) {
        LOGE("Could not remove the directory %{public}s", path.c_str());
        DumpLibMtpErrorStack();
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
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpOldDirName);
    const MtpFsTypeFile *fileToReName = dirParent ? dirParent->File(tmpOldBaseName) : nullptr;
    if (!dirParent || !fileToReName || tmpOldDirName != tmpNewDirName) {
        LOGE("Can not rename %{public}s to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
        return -EINVAL;
    }

    LIBMTP_file_t *file = fileToReName->ToLIBMTPFile();
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int rval = LIBMTP_Set_File_Name(device_, file, tmpNewBaseName.c_str());
    free(static_cast<void *>(file->filename));
    free(static_cast<void *>(file));
    if (rval > 0) {
        std::string extraData = "oldPath=" + oldPath + "newPath=" + newPath;
        StorageRadar::ReportMtpfsResult("FileRename::LIBMTP_Set_File_Name", rval, extraData);
        LOGE("Could not rename %{public}s to %{public}s", oldPath.c_str(), newPath.c_str());
        DumpLibMtpErrorStack();
        return -EINVAL;
    }
    const_cast<MtpFsTypeFile *>(fileToReName)->SetName(tmpNewBaseName);
    LOGI("File %{public}s renamed to %{public}s", oldPath.c_str(), tmpNewBaseName.c_str());
    return 0;
}

int MtpFsDevice::GetThumbnail(const std::string &path, char *buf)
{
    LOGI("MtpFsDevice: GetThumbnail enter, path: %{public}s", path.c_str());
    const std::string tmpDirName(SmtpfsDirName(path));
    const MtpFsTypeDir *dirParent = ReadDirFetchContent(tmpDirName);
    if (dirParent == nullptr) {
        LOGE("GetThumbnail %{public}s failed, dirParent is nullptr", path.c_str());
        return -ENOENT;
    }
    const std::string tmpBaseName(SmtpfsBaseName(path));
    const MtpFsTypeFile *tmpFile = dirParent->File(tmpBaseName);
    if (tmpFile == nullptr) {
        LOGE("GetThumbnail %{public}s failed, tmpFile is null", path.c_str());
        return -ENOENT;
    }
    unsigned int tmpSize;
    unsigned char *tmpBuf;
    std::unique_lock<std::mutex> lock(deviceMutex_);
    int ret = LIBMTP_Get_Thumbnail(device_, tmpFile->Id(), &tmpBuf, &tmpSize);
    if (ret != 0) {
        LOGE("GetThumbnail %{public}s failed, LIBMTP_Get_Thumbnail error.", path.c_str());
        DumpLibMtpErrorStack();
        tmpBuf = nullptr;
        return -EIO;
    }
    if ((buf != nullptr) && (memcpy_s(buf, tmpSize, tmpBuf, tmpSize) != EOK)) {
        LOGE("GetThumbnail %{public}s failed, memcpy_s thumbnail buffer error, errno=%{public}d.", errno);
        free(tmpBuf);
        tmpBuf = nullptr;
        return -ENOMEM;
    }
    free(tmpBuf);
    tmpBuf = nullptr;
    LOGI("MtpFsDevice: GetThumbnail success, path=%{public}s, size=%{public}u", path.c_str(), tmpSize);
    return tmpSize;
}

MtpFsDevice::Capabilities MtpFsDevice::GetCapabilities() const
{
    return capabilities_;
}

MtpFsDevice::Capabilities MtpFsDevice::GetCapabilities(const MtpFsDevice &device)
{
    MtpFsDevice::Capabilities capabilities;
    if (device.device_) {
        capabilities.SetCanGetPartialObject(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_GetPartialObject)));
        capabilities.SetCanSendPartialobject(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_SendPartialObject)));
        capabilities.SetCanEditObjects(
            static_cast<bool>(LIBMTP_Check_Capability(device.device_, LIBMTP_DEVICECAP_EditObjects)));
    }
    return capabilities;
}

void MtpFsDevice::AddUploadRecord(const std::string path, const std::string value)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGI("uploadRecordMap_ add %{public}s to %{public}s", path.c_str(), value.c_str());
        uploadRecordMap_[path] = value;
        return;
    }

    LOGI("uploadRecordMap_ set path %{public}s to %{public}s", path.c_str(), value.c_str());
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

void MtpFsDevice::SetUploadRecord(const std::string path, const std::string value)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGE("uploadRecordMap_ not contain %{public}s", path.c_str());
        return;
    }

    LOGI("uploadRecordMap_ set path %{public}s upload status from %{public}s to %{public}s", path.c_str(),
         it->second.c_str(), value.c_str());
    it->second = value;
    return;
}

std::tuple<std::string, std::string> MtpFsDevice::FindUploadRecord(const std::string path)
{
    std::unique_lock<std::mutex> lock(uploadRecordMutex_);
    auto it = uploadRecordMap_.find(path);
    if (it == uploadRecordMap_.end()) {
        LOGE("uploadRecordMap_ not contain %{public}s", path.c_str());
        return {"", "fail"};
    }
    return {it->first, it->second};
}

char *MtpFsDevice::GetDeviceFriendlyName()
{
    std::unique_lock<std::mutex> lock(deviceMutex_);
    char *name = LIBMTP_Get_Friendlyname(device_);
    if (name == nullptr) {
        LOGI("get device name is null");
        DumpLibMtpErrorStack();
        return nullptr;
    }
    return name;
}

void MtpFsDevice::DumpLibMtpErrorStack()
{
    if (device_ == nullptr) {
        LOGE("MtpFsDevice::DumpLibMtpErrorStack error, device_ is nullptr.");
        return;
    }
    LIBMTP_Dump_Errorstack(device_);
    LIBMTP_Clear_Errorstack(device_);
}

std::map<uint32_t, std::string> MtpFsDevice::FindDifferenceFds(
    uint32_t *newFd, int32_t newNum, uint32_t *oldFd, int32_t oldNum)
{
    std::map<uint32_t, std::string> diffFdMap;
    if (newFd == nullptr || oldFd == nullptr) {
        LOGE("newFd or oldFd is nullptr");
        return diffFdMap;
    }

    std::unordered_set<uint32_t> oldTemp(oldFd, oldFd + oldNum);
    for (uint32_t i = 0; i < newNum; ++i) {
        LOGD("FindDifferenceFds newFd=%{public}u", newFd[i]);
        if (!oldTemp.count(newFd[i])) {
            diffFdMap.insert(std::make_pair(newFd[i], "add"));
        }
    }

    std::unordered_set<uint32_t> newTemp(newFd, newFd + newNum);
    for (uint32_t i = 0; i < oldNum; ++i) {
        LOGD("FindDifferenceFds oldFd=%{public}u", oldFd[i]);
        if (!newTemp.count(oldFd[i])) {
            diffFdMap.insert(std::make_pair(oldFd[i], "remove"));
        }
    }
    return diffFdMap;
}

void MtpFsDevice::HandleDiffFdMap(std::map<uint32_t, std::string> &diffFdMap, MtpFsTypeDir *dir)
{
    if (dir == nullptr || diffFdMap.size() == 0) {
        LOGE("dir is nullptr or diffFdMap is empty");
        return;
    }
    for (auto diffFd : diffFdMap) {
        LOGI("HandleDiffFdMap diffFd=%{public}u, type=%{public}s", diffFd.first, diffFd.second.c_str());
        if (diffFd.second == "add") {
            std::unique_lock<std::mutex> lock(deviceMutex_);
            LIBMTP_file_t *file = LIBMTP_Get_Filemetadata(device_, diffFd.first);
            if (file == nullptr) {
                LOGI("HandleDiffFdMap Get File metadata fail, diffFd=%{public}u", diffFd.first);
                continue;
            }
            if (file->filetype == LIBMTP_FILETYPE_FOLDER) {
                dir->AddDir(MtpFsTypeDir(file));
            } else {
                dir->AddFile(MtpFsTypeFile(file));
            }
            continue;
        }

        for (auto iter = dir->dirList_.begin(); iter != dir->dirList_.end(); iter++) {
            if (iter->Id() == diffFd.first) {
                dir->dirList_.erase(iter);
                break;
            }
        }
        for (auto iter = dir->fileList_.begin(); iter != dir->fileList_.end(); iter++) {
            if (iter->Id() == diffFd.first) {
                dir->fileList_.erase(iter);
                break;
            }
        }
    }
}

void MtpFsDevice::HandleRemoveEvent(uint32_t handleId)
{
    LOGI("HandleRemoveEvent HandleID=%{public}u", handleId);
    std::unique_lock<std::mutex> lock(deviceMutex_);
    LIBMTP_file_t *file = LIBMTP_Get_Filemetadata(device_, handleId);
    if (file == nullptr) {
        LOGI("HandleRemoveEvent it is a remove action");
        return;
    }
    LOGI("HandleRemoveEvent it is a rename action, HandleID=%{public}u, ParentID=%{public}u, filename=%{public}s",
        file->item_id, file->parent_id, file->filename);
    bool res = UpdateFileNameByFd(rootDir_, handleId, file);
    if (res) {
        LOGI("UpdateFileNameByFd has already updated");
    }
    if (file != nullptr) {
        LIBMTP_destroy_file_t(file);
    }
}

bool MtpFsDevice::UpdateFileNameByFd(const MtpFsTypeDir &fileDir, uint32_t fileFd, LIBMTP_file_t *file)
{
    if (file == nullptr) {
        LOGE("UpdateFileNameByFd failed: input fileDir or file is nullptr.");
        return false;
    }
    if (fileDir.IsEmpty()) {
        LOGE("UpdateFileNameByFd failed: fileDir:%{public}s is empty", fileDir.Name().c_str());
        return false;
    }
    for (auto it = fileDir.fileList_.begin(); it != fileDir.fileList_.end(); ++it) {
        if (it->Id() == fileFd) {
            const_cast<MtpFsTypeDir &>(fileDir).ReplaceFile(*it, MtpFsTypeFile(file));
            return true;
        }
    }
    for (const MtpFsTypeDir &dir : fileDir.dirList_) {
        if (UpdateFileNameByFd(dir, fileFd, file)) {
            return true;
        }
    }
    return false;
}

void MtpFsDevice::SetTransferValue(bool value)
{
    std::lock_guard<std::mutex> lock(eventMutex_);
    isTransferring_.store(value);
    eventCon_.notify_one();
}