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
#include <libmtp.h>
#include "libmtp_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace OHOS {
constexpr size_t MIN_STORAGE_SIZE = sizeof(uint16_t) * 3 + sizeof(uint32_t) + sizeof(uint64_t) * 3 + sizeof(char *) * 2;
constexpr size_t MIN_EXTENSION_SIZE = sizeof(char *) + sizeof(int) * 2;
constexpr size_t MIN_MTPDEVICE_SIZE = sizeof(uint8_t) * 2 + sizeof(uint32_t) * 8 + sizeof(int);
constexpr size_t MIN_SIZE = std::max({MIN_STORAGE_SIZE, MIN_EXTENSION_SIZE, MIN_MTPDEVICE_SIZE});
constexpr size_t MIN_RAWDEVICE_SIZE = sizeof(uint32_t) * 2 + sizeof(uint16_t) * 2 +
                                      sizeof(uint8_t) + sizeof(int) + sizeof(char *) * 2;
constexpr size_t MIN_FOLDER_SIZE = sizeof(uint32_t) * 3 + sizeof(char *);
constexpr size_t MIN_FILE_SIZE = sizeof(uint32_t) * 3 + sizeof(uint64_t) + sizeof(char *) +
                                 sizeof(time_t) + sizeof(LIBMTP_filetype_t);

template<class T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T*>(data));
}

int ConstructRawDevice(const uint8_t *data, size_t size, LIBMTP_device_entry_t *entry, LIBMTP_raw_device_t *device)
{
    int pos = 0;
    if (entry != nullptr) {
        entry->vendor = TypeCast<char*>(data, &pos);
        entry->product = TypeCast<char*>(data + pos);
        entry->vendor_id = TypeCast<uint16_t>(data + pos);
        entry->product_id = TypeCast<uint16_t>(data + pos);
        entry->device_flags = TypeCast<uint32_t>(data + pos);
        device->bus_location = TypeCast<uint32_t>(data + pos);
        device->devnum = TypeCast<uint8_t>(data + pos);
        device->device_entry = *entry;
    }
    return pos;
}

void ConstructDeviceStorage(const uint8_t *data, size_t size, LIBMTP_devicestorage_t *storage)
{
    int pos = 0;
    if (storage != nullptr) {
        storage->id = TypeCast<uint32_t>(data, &pos);
        storage->StorageType = TypeCast<uint16_t>(data + pos);
        storage->FilesystemType = TypeCast<uint16_t>(data + pos);
        storage->AccessCapability = TypeCast<uint16_t>(data + pos);
        storage->MaxCapacity = TypeCast<uint64_t>(data + pos);
        storage->FreeSpaceInBytes = TypeCast<uint64_t>(data + pos);
        storage->FreeSpaceInObjects = TypeCast<uint64_t>(data + pos);
        storage->StorageDescription = TypeCast<char*>(data + pos);
        storage->VolumeIdentifier = TypeCast<char*>(data + pos);
        storage->next = nullptr;
        storage->prev = nullptr;
    }
}

void ConstructDeviceExtension(const uint8_t *data, size_t size, LIBMTP_device_extension_t *extensions)
{
    int pos = 0;
    if (extensions != nullptr) {
        extensions->name = TypeCast<char*>(data + pos);
        extensions->major = TypeCast<int>(data + pos);
        extensions->minor = TypeCast<int>(data + pos);
        extensions->next = nullptr;
    }
}

int ConstructMtpDevice(const uint8_t *data, size_t size, LIBMTP_mtpdevice_t *device)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return 0;
    }
    LIBMTP_devicestorage_t storage;
    LIBMTP_device_extension_t extensions;
    LIBMTP_error_t errorStack;

    int pos = 0;
    if (device == nullptr) {
        return 0;
    }
    device->object_bitsize = TypeCast<uint8_t>(data, &pos);
    ConstructDeviceStorage(data, size, &storage);
    device->storage = &storage;
    errorStack = TypeCast<LIBMTP_error_t>(data + pos);
    device->errorstack = &errorStack;
    device->maximum_battery_level = TypeCast<uint8_t>(data + pos);
    device->default_music_folder = TypeCast<uint32_t>(data + pos);
    device->default_playlist_folder = TypeCast<uint32_t>(data + pos);
    device->default_picture_folder = TypeCast<uint32_t>(data + pos);
    device->default_video_folder = TypeCast<uint32_t>(data + pos);
    device->default_organizer_folder = TypeCast<uint32_t>(data + pos);
    device->default_zencast_folder = TypeCast<uint32_t>(data + pos);
    device->default_album_folder = TypeCast<uint32_t>(data + pos);
    device->default_text_folder = TypeCast<uint32_t>(data + pos);
    ConstructDeviceExtension(data, size, &extensions);
    device->extensions = &extensions;
    device->cached = TypeCast<int>(data + pos);
    device->next = nullptr;

    return pos;
}

int ConstructFolder(const uint8_t *data, size_t size, int pos, LIBMTP_folder_t *folder)
{
    if (folder != nullptr) {
        folder->folder_id = TypeCast<uint32_t>(data + pos);
        folder->parent_id = TypeCast<uint32_t>(data + pos);
        folder->storage_id = TypeCast<uint32_t>(data + pos);
        folder->name = TypeCast<char*>(data + pos);
        folder->sibling = nullptr;
        folder->child = nullptr;
    }
}

int ConstructFile(const uint8_t *data, size_t size, int pos, LIBMTP_file_t *file)
{
    if (file != nullptr) {
        file->item_id = TypeCast<uint32_t>(data + pos);
        file->parent_id = TypeCast<uint32_t>(data + pos);
        file->storage_id = TypeCast<uint32_t>(data + pos);
        file->filename = TypeCast<char*>(data + pos);
        file->filesize = TypeCast<uint64_t>(data + pos);
        file->modificationdate = TypeCast<time_t>(data + pos);
        file->filetype = TypeCast<LIBMTP_filetype_t>(data + pos);
        file->next = nullptr;
    }
}

bool CheckSpecificDeviceFuzzTest(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    if (data == nullptr || size <= sizeof(int) * numPara32) {
        return false;
    }

    int pos = 0;
    int busno = TypeCast<int>(data, &pos);
    int devno = TypeCast<int>(data + pos);

    LIBMTP_Check_Specific_Device(busno, devno);
    return true;
}

bool OpenRawDeviceUncachedTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_RAWDEVICE_SIZE) {
        return false;
    }

    LIBMTP_device_entry_t entry = {
        .vendor = nullptr,
        .vendor_id = 0,
        .product = nullptr,
        .product_id = 0,
        .device_flags = 0
    };
    LIBMTP_raw_device_t device = {
        .device_entry = entry,
        .bus_location = 0,
        .devnum = 0
    };
    ConstructRawDevice(data, size, &entry, &device);

    LIBMTP_mtpdevice_t *ret = LIBMTP_Open_Raw_Device_Uncached(&device);
    if (ret == NULL) {
        return false;
    }
    return true;
}

bool DetectRawDevicesTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_RAWDEVICE_SIZE) {
        return false;
    }

    LIBMTP_device_entry_t entry = {
        .vendor = nullptr,
        .vendor_id = 0,
        .product = nullptr,
        .product_id = 0,
        .device_flags = 0
    };
    LIBMTP_raw_device_t device = {
        .device_entry = entry,
        .bus_location = 0,
        .devnum = 0
    };
    int pos = ConstructRawDevice(data, size, &entry, &device);
    int numdevs = TypeCast<int>(data + pos);

    LIBMTP_raw_device_t* device_list = &device;
    LIBMTP_error_number_t result = LIBMTP_Detect_Raw_Devices(&device_list, &numdevs);
    if (result != 0) {
        return false;
    }
    return true;
}

bool ReleaseDeviceTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int ret = ConstructMtpDevice(data, size, &mtpDevice);
    LIBMTP_Release_Device(&mtpDevice);
    return true;
}

bool GetFilesAndFoldersTest(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t) * numPara32) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t storage = TypeCast<uint32_t>(data + pos);
    uint32_t parent = TypeCast<uint32_t>(data + pos);
    LIBMTP_file_t *ret = LIBMTP_Get_Files_And_Folders(&mtpDevice, storage, parent);
    if (ret == NULL) {
        return false;
    }
    return true;
}

bool CreateFolderTest(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t) * numPara32 + sizeof(char*)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    char* name = TypeCast<char*>(data + pos);
    uint32_t parentId = TypeCast<uint32_t>(data + pos);
    uint32_t storageId = TypeCast<uint32_t>(data + pos);
    uint32_t ret = LIBMTP_Create_Folder(&mtpDevice, name, parentId, storageId);
    if (ret == 0) {
        return false;
    }
    return true;
}

bool SetFolderNameTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + MIN_FOLDER_SIZE + sizeof(char*)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    LIBMTP_folder_t folder;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    pos = ConstructFolder(data, size, pos, &folder);
    char* newName = TypeCast<char*>(data + pos);
    uint32_t ret = LIBMTP_Set_Folder_Name(&mtpDevice, &folder, newName);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool GetFileToFileTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t) + sizeof(LIBMTP_progressfunc_t)
        + sizeof(char const *const) + sizeof(void const *const)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;

    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t id = TypeCast<uint32_t>(data + pos);
    char const *const path = TypeCast<char const *const>(data + pos);
    LIBMTP_progressfunc_t const callback = TypeCast<LIBMTP_progressfunc_t const>(data + pos);
    void const * const userData = TypeCast<void const * const>(data + pos);

    int ret = LIBMTP_Get_File_To_File(&mtpDevice, id, path, callback, userData);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool SendFileFromFileTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + MIN_FILE_SIZE + sizeof(LIBMTP_progressfunc_t)
        + sizeof(char const *const) + sizeof(void const *const)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    LIBMTP_file_t file;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    char const *const path = TypeCast<char const *const>(data + pos);
    pos = ConstructFile(data, size, pos, &file);
    LIBMTP_progressfunc_t const callback = TypeCast<LIBMTP_progressfunc_t const>(data + pos);
    void const * const userData = TypeCast<void const * const>(data + pos);

    int ret = LIBMTP_Send_File_From_File(&mtpDevice, path, &file, callback, userData);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool SetFileNameTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + MIN_FILE_SIZE + sizeof(const char*)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    LIBMTP_file_t file;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    pos = ConstructFile(data, size, pos, &file);
    char *newName = TypeCast<char *>(data + pos);

    int ret = LIBMTP_Set_File_Name(&mtpDevice, &file, newName);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool DeleteObjectTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t objectId = TypeCast<uint32_t>(data + pos);

    int ret = LIBMTP_Delete_Object(&mtpDevice, objectId);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool SetObjectU32Test(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t const) * numPara32 + sizeof(LIBMTP_property_t const)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t const objectId = TypeCast<uint32_t const>(data + pos);
    LIBMTP_property_t const attributeId = TypeCast<LIBMTP_property_t const>(data + pos);
    uint32_t const value = TypeCast<uint32_t const>(data + pos);

    int ret = LIBMTP_Set_Object_u32(&mtpDevice, objectId, attributeId, value);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool SetObjectStringTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + sizeof(uint32_t const) +
        sizeof(LIBMTP_property_t const) + sizeof(char const *const)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t const objectId = TypeCast<uint32_t const>(data + pos);
    LIBMTP_property_t const attributeId = TypeCast<LIBMTP_property_t const>(data + pos);
    char const * const string = TypeCast<char const * const>(data + pos);

    int ret = LIBMTP_Set_Object_String(&mtpDevice, objectId, attributeId, string);
    if (ret != 0) {
        return false;
    }
    return true;
}

bool GetPartialObjectTest(const uint8_t *data, size_t size)
{
    int numPara32 = 2;
    size_t sizeReq = MIN_SIZE + sizeof(uint32_t const) * numPara32 +
                     sizeof(uint64_t const) + sizeof(unsigned int) + sizeof(unsigned char*);
    if (data == nullptr || size <= sizeReq) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t const id = TypeCast<uint32_t const>(data + pos);
    uint64_t offset = TypeCast<uint64_t>(data + pos);
    uint32_t maxbytes = TypeCast<uint32_t>(data + pos);
    unsigned char* paraData = TypeCast<unsigned char*>(data + pos);
    unsigned int paraSize = TypeCast<unsigned int>(data + pos);

    int ret = LIBMTP_GetPartialObject(&mtpDevice, id, offset, maxbytes, &paraData, &paraSize);
    if (ret == -1) {
        return false;
    }
    return true;
}

bool SendPartialObjectTest(const uint8_t *data, size_t size)
{
    size_t sizeReq = MIN_SIZE + sizeof(uint32_t const) + sizeof(uint64_t const) +
                     sizeof(unsigned int) + sizeof(unsigned char);
    if (data == nullptr || size <= sizeReq) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    uint32_t const id = TypeCast<uint32_t const>(data + pos);
    uint64_t offset = TypeCast<uint64_t>(data + pos);
    unsigned char paraData = TypeCast<unsigned char>(data + pos);
    unsigned int paraSize = TypeCast<unsigned int>(data + pos);

    int ret = LIBMTP_SendPartialObject(&mtpDevice, id, offset, &paraData, paraSize);
    if (ret == -1) {
        return false;
    }
    return true;
}

bool GetStorageTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE + sizeof(int)) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    int pos = ConstructMtpDevice(data, size, &mtpDevice);
    int sortby = TypeCast<int>(data + pos);
    int ret = LIBMTP_Get_Storage(&mtpDevice, sortby);
    if (ret == -1) {
        return false;
    }
    return true;
}

bool CheckCapabilityTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    LIBMTP_devicecap_t cap;
    ConstructMtpDevice(data, size, &mtpDevice);
    if (&mtpDevice == NULL || mtpDevice.params == NULL) {
        return true;
    } else {
        cap = LIBMTP_DEVICECAP_GetPartialObject;
        LIBMTP_Check_Capability(&mtpDevice, cap);
        cap = LIBMTP_DEVICECAP_SendPartialObject;
        LIBMTP_Check_Capability(&mtpDevice, cap);
        cap = LIBMTP_DEVICECAP_EditObjects;
        LIBMTP_Check_Capability(&mtpDevice, cap);
        cap = LIBMTP_DEVICECAP_MoveObject;
        LIBMTP_Check_Capability(&mtpDevice, cap);
        cap = LIBMTP_DEVICECAP_CopyObject;
        LIBMTP_Check_Capability(&mtpDevice, cap);
    }
    return true;
}

bool DumpErrorstackTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    ConstructMtpDevice(data, size, &mtpDevice);
    LIBMTP_Dump_Errorstack(&mtpDevice);
    return true;
}

bool ClearErrorstackTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return false;
    }
    LIBMTP_mtpdevice_t mtpDevice;
    ConstructMtpDevice(data, size, &mtpDevice);
    LIBMTP_Clear_Errorstack(&mtpDevice);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::CheckSpecificDeviceFuzzTest(data, size);
    OHOS::OpenRawDeviceUncachedTest(data, size);
    OHOS::DetectRawDevicesTest(data, size);
    OHOS::ReleaseDeviceTest(data, size);
    OHOS::GetFilesAndFoldersTest(data, size);
    OHOS::CreateFolderTest(data, size);
    OHOS::SetFolderNameTest(data, size);
    OHOS::GetFileToFileTest(data, size);
    OHOS::SendFileFromFileTest(data, size);
    OHOS::SetFileNameTest(data, size);
    OHOS::DeleteObjectTest(data, size);
    OHOS::SetObjectU32Test(data, size);
    OHOS::SetObjectStringTest(data, size);
    OHOS::GetPartialObjectTest(data, size);
    OHOS::SendPartialObjectTest(data, size);
    OHOS::GetStorageTest(data, size);
    OHOS::CheckCapabilityTest(data, size);
    OHOS::DumpErrorstackTest(data, size);
    OHOS::ClearErrorstackTest(data, size);
    return 0;
}