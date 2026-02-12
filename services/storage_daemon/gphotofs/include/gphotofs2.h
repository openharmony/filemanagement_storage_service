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
#ifndef GPHOTOFS2_H
#define GPHOTOFS2_H

#include <string>
#include <fuse.h>
#include <unistd.h>
#include "dir.h"
#include "file.h"
#include "utils.h"
#include "context.h"

struct Options {
    std::string port;
    std::string model;
    std::string usbid;
    int32_t speed = 0;
};

struct FileDesc {
    bool writable;
    File *file;
};

struct ThumbDesc {
    std::mutex lock;
    std::string realPath;
    std::string dirName;
    std::string fileName;
    Context* ctx;
    char* buf = nullptr;
    int fd {-1};
    bool ready {false};
    size_t size = 0;

    ThumbDesc() = default;
    ThumbDesc(const ThumbDesc&) = delete;
    ThumbDesc& operator=(const ThumbDesc&) = delete;

    ~ThumbDesc()
    {
        if (buf != nullptr) {
            free(buf);
            buf = nullptr;
        }
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }
};

extern fuse_operations fuseOperations_;
#endif // GPHOTOFS2_H