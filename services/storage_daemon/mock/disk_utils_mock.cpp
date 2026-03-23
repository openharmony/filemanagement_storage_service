/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "disk_utils_mock.h"

using namespace std;
namespace OHOS {
namespace StorageDaemon {

int CreateDiskNode(const std::string &path, dev_t dev)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->CreateDiskNode(path, dev);
}

int DestroyDiskNode(const std::string &path)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->DestroyDiskNode(path);
}

int GetDevSize(const std::string &path, uint64_t *size)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->GetDevSize(path, size);
}

int GetMaxVolume(dev_t device)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->GetMaxVolume(device);
}

int IsBlankCD(const std::string &diskPath, bool &isBlankCD)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->IsBlankCD(diskPath, isBlankCD);
}

int IsExistCD(const std::string &diskPath, bool &isExistCD)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->IsExistCD(diskPath, isExistCD);
}

int Eject(const std::string &devPath)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->Eject(devPath);
}

int32_t ReadMetadata(const std::string &devPath, std::string &uuid, std::string &type, std::string &label)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->ReadMetadata(devPath, uuid, type, label);
}

std::string GetBlkidData(const std::string &devPath, const std::string &type)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return " ";
    }
    return IDiskUtilMoc::diskUtilMoc->GetBlkidData(devPath, type);
}

std::string GetBlkidDataByCmd(std::vector<std::string> &cmd)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return " ";
    }
    return IDiskUtilMoc::diskUtilMoc->GetBlkidDataByCmd(cmd);
}

std::string GetAnonyString(const std::string &value)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return " ";
    }
    return IDiskUtilMoc::diskUtilMoc->GetAnonyString(value);
}

int32_t ReadVolumeUuid(const std::string &devPath, std::string &uuid)
{
    if (IDiskUtilMoc::diskUtilMoc == nullptr) {
        return 0;
    }
    return IDiskUtilMoc::diskUtilMoc->ReadVolumeUuid(devPath, uuid);
}
}
}
