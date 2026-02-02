/*
* Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "disk_func_mock.h"
using namespace OHOS::StorageDaemon;

FILE *Fopen(const char *pathname, const char *mode)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return nullptr;
    }
    return DiskFunc::diskFunc_->fopen(pathname, mode);
}

int Fclose(FILE *stream)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return -1;
    }
    return DiskFunc::diskFunc_->fclose(stream);
}

int Open(const char *filename, int flags, ...)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return -1;
    }
    return DiskFunc::diskFunc_->open(filename, flags);
}

int Close(int fd)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return -1;
    }
    return DiskFunc::diskFunc_->close(fd);
}

int Ioctl(int fd, int request, void* arg)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return -1;
    }
    return DiskFunc::diskFunc_->ioctl(fd, request);
}

int Fileno(FILE *stream)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return -1;
    }
    return DiskFunc::diskFunc_->fileno(stream);
}

char *Realpath(const char *path, char *resolved_path)
{
    if (DiskFunc::diskFunc_ == nullptr) {
        return nullptr;
    }
    return DiskFunc::diskFunc_->realpath(path, resolved_path);
}