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
#ifndef GPHOTOFS2_CONTEXT_H
#define GPHOTOFS2_CONTEXT_H

#include <string>
#include <gphoto2/gphoto2.h>
#include <fuse.h>
#include <mutex>
#include "dir.h"

class Context {
public:
    Context();
    ~Context();
    Camera *camera() { return camera_; }
    GPContext *context() { return context_; }
    uid_t uid() { return uid_; }
    gid_t gid() { return gid_; }
    Dir& root() { return root_; }
    struct statvfs *statCache() { return statCache_; }
    void cacheStat(struct statvfs *newStat)
    {
        if (newStat == nullptr) {
            return;
        }
        if (statCache_ != nullptr) {
            delete statCache_;
        }
        statCache_ = new struct statvfs();
        *statCache_ = *newStat;
    }

private:
    Camera *camera_;
    GPContext *context_;
    CameraAbilitiesList *abilities_;
    int debug_func_id_;

    uid_t uid_;
    gid_t gid_;

    std::string directory_;
    Dir root_;
    struct statvfs *statCache_;
};


#endif // GPHOTOFS2_CONTEXT_H