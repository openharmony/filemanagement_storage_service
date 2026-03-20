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
#include "libmtpsendpartialobject_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace OHOS {
constexpr size_t MIN_STORAGE_SIZE = sizeof(uint16_t) * 3 + sizeof(uint32_t) + sizeof(uint64_t) * 3 + sizeof(char *) * 2;
constexpr size_t MIN_EXTENSION_SIZE = sizeof(char *) + sizeof(int) * 2;
constexpr size_t MIN_MTPDEVICE_SIZE = sizeof(uint8_t) * 2 + sizeof(uint32_t) * 8 + sizeof(int) +
                                      sizeof(LIBMTP_error_t);
constexpr size_t MIN_SIZE = std::max({MIN_STORAGE_SIZE, MIN_EXTENSION_SIZE, MIN_MTPDEVICE_SIZE});

template<class T>
T TypeCast(const uint8_t *data, size_t size, size_t *pos = nullptr)
{
    if (data == nullptr || size < sizeof(T)) {
        return T{};
    }
    if (pos) {
        if (*pos + sizeof(T) > size) {
            return T{};
        }
        T result = *(reinterpret_cast<const T*>(data + *pos));
        *pos += sizeof(T);
        return result;
    }
    return *(reinterpret_cast<const T*>(data));
}

void ConstructDeviceStorage(const uint8_t *data, size_t size, LIBMTP_devicestorage_t *storage)
{
    size_t pos = 0;
    if (storage != nullptr) {
        storage->id = TypeCast<uint32_t>(data, size, &pos);
        storage->StorageType = TypeCast<uint16_t>(data, size, &pos);
        storage->FilesystemType = TypeCast<uint16_t>(data, size, &pos);
        storage->AccessCapability = TypeCast<uint16_t>(data, size, &pos);
        storage->MaxCapacity = TypeCast<uint64_t>(data, size, &pos);
        storage->FreeSpaceInBytes = TypeCast<uint64_t>(data, size, &pos);
        storage->FreeSpaceInObjects = TypeCast<uint64_t>(data, size, &pos);
        storage->StorageDescription = nullptr;
        storage->VolumeIdentifier = nullptr;
        storage->next = nullptr;
        storage->prev = nullptr;
    }
}

void ConstructDeviceExtension(const uint8_t *data, size_t size, LIBMTP_device_extension_t *extensions)
{
    size_t pos = 0;
    if (extensions != nullptr) {
        extensions->name = nullptr;
        extensions->major = TypeCast<int>(data, size, &pos);
        extensions->minor = TypeCast<int>(data, size, &pos);
        extensions->next = nullptr;
    }
}

int ConstructMtpDevice(const uint8_t *data, size_t size, LIBMTP_mtpdevice_t *device,
    LIBMTP_devicestorage_t *storage, LIBMTP_device_extension_t *extensions)
{
    if (data == nullptr || size <= MIN_SIZE || device == nullptr ||
        storage == nullptr || extensions == nullptr) {
        return 0;
    }

    size_t pos = 0;
    device->object_bitsize = TypeCast<uint8_t>(data, size, &pos);
    ConstructDeviceStorage(data, size, storage);
    device->storage = storage;
    device->errorstack = nullptr;
    device->maximum_battery_level = TypeCast<uint8_t>(data, size, &pos);
    device->default_music_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_playlist_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_picture_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_video_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_organizer_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_zencast_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_album_folder = TypeCast<uint32_t>(data, size, &pos);
    device->default_text_folder = TypeCast<uint32_t>(data, size, &pos);
    ConstructDeviceExtension(data, size, extensions);
    device->extensions = extensions;
    device->cached = TypeCast<int>(data, size, &pos);
    device->next = nullptr;
    device->params = nullptr;

    return pos;
}

bool SendPartialObjectTest(const uint8_t *data, size_t size)
{
    size_t sizeReq = MIN_SIZE + sizeof(uint32_t const) + sizeof(uint64_t const) +
                     sizeof(unsigned int) + sizeof(unsigned char);
    if (data == nullptr || size <= sizeReq) {
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
    free(rawdevices);

    LIBMTP_mtpdevice_t mtpDevice;
    LIBMTP_devicestorage_t storage;
    LIBMTP_device_extension_t extensions;
    size_t pos = ConstructMtpDevice(data, size, &mtpDevice, &storage, &extensions);
    uint32_t const id = TypeCast<uint32_t const>(data, size, &pos);
    uint64_t offset = TypeCast<uint64_t>(data, size, &pos);
    unsigned char paraData = TypeCast<unsigned char>(data, size, &pos);
    unsigned int paraSize = TypeCast<unsigned int>(data, size, &pos);

    int ret = LIBMTP_SendPartialObject(&mtpDevice, id, offset, &paraData, paraSize);
    if (ret == -1) {
        return false;
    }
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SendPartialObjectTest(data, size);
    return 0;
}
