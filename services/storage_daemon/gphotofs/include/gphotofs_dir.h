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
    bool refresh;
    bool loading;
    bool dirty;
    int offset;
    std::map<std::string, File*> files;
    std::map<std::string, Dir*> dirs;
    std::mutex lock;

    explicit Dir(const std::string& name) : name(name), listed(false),
        refresh(false), loading(false), dirty(false), offset(0) {}
    ~Dir();

    void AddFile(File *file);
    void RemoveFile(File *file);
    File* GetFile(const std::string& name);
    void AddDir(Dir *dir);
    void RemoveDir(Dir *dir);
    Dir* GetDir(const std::string& name);
    std::string GetPath(Dir *dir);
    bool Empty();
    void Clear();
    void SetListed(bool stat);
    bool GetListed();
    void SetNextOffset(int next);
    int GetNextOffset();
    void SetRefresh(bool stat);
    bool GetRefresh();
    bool TryBeginLoad();
    void EndLoad();
    void SetDirty(bool stat);
    bool GetDirty();
};

#endif // GPHOTOFS2_DIR_H
