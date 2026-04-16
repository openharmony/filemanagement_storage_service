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
 * See the License for the specific language governing permissions
 * and limitations under the License.
 */
#include "gphotofs_dir.h"
#include "gphotofs_file.h"
#include "storage_service_log.h"

void Dir::AddFile(File *file)
{
    if (file == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = files.find(file->name);
    if (it != files.end() && it->second != nullptr) {
        delete it->second;
        it->second = nullptr;
    }
    files[file->name] = file;
}

void Dir::RemoveFile(File *file)
{
    if (file == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    files.erase(file->name);
}

void Dir::AddDir(Dir *dir)
{
    if (dir == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = dirs.find(dir->name);
    if (it != dirs.end() && it->second != nullptr) {
        delete it->second;
        it->second = nullptr;
    }
    dirs[dir->name] = dir;
}

void Dir::RemoveDir(Dir *dir)
{
    if (dir == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    dirs.erase(dir->name);
}

File* Dir::GetFile(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = files.find(name);
    if (it == files.end()) return nullptr;
    return it->second;
}

std::string Dir::GetPath(Dir *dir)
{
    if (dir == nullptr) {
        return "";
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    for (const auto& it : dirs) {
        if (it.second == dir) {
            return it.first;
        }
    }
    return "";
}

Dir* Dir::GetDir(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = dirs.find(name);
    if (it == dirs.end()) return nullptr;
    return it->second;
}

void Dir::SetListed(bool stat)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    listed = stat;
}

bool Dir::GetListed()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return listed;
}

void Dir::SetNextOffset(int next)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    offset = next;
}

int Dir::GetNextOffset()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return offset;
}

void Dir::SetRefresh(bool stat)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    refresh = stat;
}

bool Dir::GetRefresh()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return refresh;
}

bool Dir::TryBeginLoad()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    if (listed || loading) {
        return false;
    }
    loading = true;
    return true;
}

void Dir::EndLoad()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    loading = false;
}

void Dir::SetDirty(bool stat)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    dirty = stat;
}

bool Dir::GetDirty()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return dirty;
}

void Dir::Clear()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    for (auto &it : files) {
        delete it.second;
        it.second = nullptr;
    }
    files.clear();
    for (auto &it : dirs) {
        delete it.second;
        it.second = nullptr;
    }
    dirs.clear();
    offset = 0;
    listed = false;
    refresh = false;
    dirty = false;
    LOGI("dir clear end");
}

bool Dir::Empty()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return files.empty() && dirs.empty();
}

Dir::~Dir()
{
    Clear();
}