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
#ifndef GPHOTOFS2_DIR_H
#define GPHOTOFS2_DIR_H

#include <string>
#include <map>
#include <mutex>

#include "storage_service_log.h"
class File;

struct Dir {
    std::string name;

    bool listed;
    std::map<std::string, File*> files;
    std::map<std::string, Dir*> dirs;
    std::mutex lock;

    Dir(const std::string& name) : name(name), listed(false) {}
    ~Dir();

    void addFile(File *file);
    void removeFile(File *file);
    File* getFile(const std::string& name);
    void addDir(Dir *dir);
    void removeDir(Dir *dir);
    Dir* getDir(const std::string& name);
    std::string getPath(Dir *dir);
    bool empty();
};

#endif // GPHOTOFS2_DIR_H