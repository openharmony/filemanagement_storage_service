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

#ifndef OHOS_STORAGE_DAEMON_DISK_FUNC_MOCK_H
#define OHOS_STORAGE_DAEMON_DISK_FUNC_MOCK_H

#include <gmock/gmock.h>

#include <cstdio>
#include <fcntl.h>
#include <grp.h>
#include <memory>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "securec.h"

FILE* Fopen(const char*, const char*);
int Fclose(FILE*);
int Open(const char*, int, ...);
int Close(int);
int Ioctl(int fd, int request, void* arg);
int Fileno(FILE *stream);

namespace OHOS {
namespace StorageDaemon {
class DiskFunc {
public:
    virtual ~DiskFunc() = default;
    virtual FILE* fopen(const char*, const char*) = 0;
    virtual int fclose(FILE*) = 0;
    virtual int open(const char *, int) = 0;
    virtual int close(int) = 0;
    virtual int ioctl(int fd, int request) = 0;
    virtual int fileno(FILE *stream) = 0;
public:
    static inline std::shared_ptr<DiskFunc> diskFunc_ = nullptr;
};

class DiskFuncMock : public DiskFunc {
public:
    MOCK_METHOD2(fopen, FILE*(const char*, const char*));
    MOCK_METHOD1(fclose, int(FILE*));
    MOCK_METHOD2(open, int(const char*, int));
    MOCK_METHOD1(close, int(int));
    MOCK_METHOD2(ioctl, int(int, int));
    MOCK_METHOD1(fileno, int(FILE *));
};
} // namespace StorageDaemon
} // namespace OHOS

#endif // OHOS_STORAGE_DAEMON_DISK_FUNC_MOCK_H