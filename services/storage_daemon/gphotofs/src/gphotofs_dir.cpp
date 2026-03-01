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

void Dir::addFile(File *file)
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

void Dir::removeFile(File *file)
{
    if (file == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    files.erase(file->name);
}

void Dir::addDir(Dir *dir)
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

void Dir::removeDir(Dir *dir)
{
    if (dir == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lockGuard(lock);
    dirs.erase(dir->name);
}

File* Dir::getFile(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = files.find(name);
    if (it == files.end()) return nullptr;
    return it->second;
}

std::string Dir::getPath(Dir *dir)
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

Dir* Dir::getDir(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(lock);
    auto it = dirs.find(name);
    if (it == dirs.end()) return nullptr;
    return it->second;
}

bool Dir::empty()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    return files.empty() && dirs.empty();
}

Dir::~Dir()
{
    std::lock_guard<std::mutex> lockGuard(lock);
    for (auto& it : files) {
        if (it.second != nullptr) {
            delete it.second;
            it.second = nullptr;
        }
    }
    for (auto& it : dirs) {
        if (it.second != nullptr) {
            delete it.second;
            it.second = nullptr;
        }
    }
}