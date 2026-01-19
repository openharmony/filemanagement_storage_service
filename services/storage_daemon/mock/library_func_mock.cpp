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

#include "library_func_mock.h"

using namespace OHOS::StorageDaemon;
/* Mount a filesystem.  */
int mount(const char *specialFile, const char *dir, const char *fstype, unsigned long int rwflag, const void *data)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->mount(specialFile, dir, fstype, rwflag, data);
}

int umount(const char *specialFile)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->umount(specialFile);
}

int umount2(const char *specialFile, int flags)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->umount2(specialFile, flags);
}

int remove(const char *pathname)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->remove(pathname);
}

int lstat(const char *path, struct stat *buf)
{
    if (LibraryFunc::libraryFunc_ == nullptr) {
        return -1;
    }
    return LibraryFunc::libraryFunc_->lstat(path, buf);
}