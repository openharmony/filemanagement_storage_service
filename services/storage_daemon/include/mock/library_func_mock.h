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

#ifndef FILEMANAGEMENT_APP_FILE_SERVICE_LIBRARY_FUNC_MOCK_H
#define FILEMANAGEMENT_APP_FILE_SERVICE_LIBRARY_FUNC_MOCK_H

#include <gmock/gmock.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>

namespace OHOS {
namespace StorageDaemon {
class LibraryFunc {
public:
    virtual ~LibraryFunc() = default;
    virtual int mount(const char *, const char *, const char *, unsigned long int, const void *) = 0;
    virtual int umount(const char *specialFile) = 0;
    virtual int umount2(const char *specialFile, int flags) = 0;
    virtual int remove(const char *pathname) = 0;
    virtual int lstat(const char *path, struct stat *buf) = 0;
public:
    static inline std::shared_ptr<LibraryFunc> libraryFunc_ = nullptr;
};

class LibraryFuncMock : public LibraryFunc {
public:
    MOCK_METHOD5(mount, int(const char *, const char *, const char *, unsigned long int, const void *));
    MOCK_METHOD1(umount, int(const char *specialFile));
    MOCK_METHOD2(umount2, int(const char *specialFile, int flags));
    MOCK_METHOD1(remove, int(const char *pathname));
    MOCK_METHOD2(lstat, int(const char *path, struct stat *buf));
};
}
}

#endif
