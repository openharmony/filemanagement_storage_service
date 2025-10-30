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
#include "libmtpreleasedevice_fuzzer.h"

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

bool ReleaseDeviceTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size <= MIN_SIZE) {
        return false;
    }

    LIBMTP_raw_device_t *rawdevices = nullptr;
    int numDevices = 0;

    LIBMTP_error_number_t err = LIBMTP_Detect_Raw_Devices(&rawdevices, &numDevices);
    if (err != LIBMTP_ERROR_NONE || numDevices == 0) {
        if (rawdevices) {
            free(rawdevices);
        }
        return false;
    }

    LIBMTP_mtpdevice_t *device = LIBMTP_Open_Raw_Device(&rawdevices[0]);
    if (!device) {
        free(rawdevices);
        return false;
    }

    LIBMTP_mtpdevice_t mtpDevice;
    int ret = ConstructMtpDevice(data, size, &mtpDevice);
    LIBMTP_Release_Device(&mtpDevice);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::ReleaseDeviceTest(data, size);
    return 0;
}