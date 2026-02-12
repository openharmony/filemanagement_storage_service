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
#ifndef GPHOTOFS2_FILE_H
#define GPHOTOFS2_FILE_H

#include <string>
#include <gphoto2.h>
#include <mutex>
#include <unistd.h>

#include "utils.h"

struct File {
    std::string name;
    CameraFile *camFile;
    int tmpFd;
    std::string tmpPath;
    off_t size;
    int mtime;
    int ref;
    bool changed;
    bool tmpFileCreated;
    std::mutex lock;

    File(const std::string& name, const CameraFileInfo& info)
        : name(name), mtime(info.file.mtime), size(info.file.size),
          tmpFd(-1), tmpFileCreated(false), ref(0), changed(false), camFile(nullptr) {}

    File(const std::string& name)
        : name(name), mtime(Now()), size(0),
          tmpFd(-1), tmpFileCreated(false), ref(0), changed(false), camFile(nullptr) {}

    // Disable copy constructor and assignment operator
    File(const File&) = delete;
    File& operator=(const File&) = delete;

    ~File()
    {
        if (tmpFd >= 0) {
            close(tmpFd);
            tmpFd = -1;
        }
        if (tmpFileCreated && !tmpPath.empty()) {
            unlink(tmpPath.c_str());
            tmpPath.clear();
        }
        if (camFile) gp_file_unref(camFile);
    }
};


#endif // GPHOTOFS2_FILE_H