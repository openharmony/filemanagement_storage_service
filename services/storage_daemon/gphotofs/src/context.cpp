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
#include "context.h"
#include <unistd.h>
#include "utils.h"
#include "storage_service_log.h"
using namespace std;

Context::Context() : directory_(""), root_(""), camera_(nullptr), context_(nullptr),
    abilities_(nullptr), debug_func_id_(0), uid_(0), gid_(0), statCache_(nullptr)
{
    context_ = gp_context_new();
    if (context_ == nullptr) {
        LOGE("Context: failed to create gphoto context");
        return;
    }

    int ret;
    if ((ret = gp_camera_new(&camera_)) != GP_OK) {
        LOGE("Context: failed to create camera, ret=%{public}d", ret);
        gp_camera_exit(camera_, context_ ? context_ : NULL);
        gp_camera_unref(camera_);
        camera_ = nullptr;
        gp_context_unref(context_);
        context_ = nullptr;
        return;
    }

    if ((ret = gp_abilities_list_new(&abilities_)) != GP_OK) {
        LOGE("Context: failed to create abilities list, ret=%{public}d", ret);
        abilities_ = nullptr;
        gp_camera_exit(camera_, context_ ? context_ : NULL);
        gp_camera_unref(camera_);
        camera_ = nullptr;
        gp_context_unref(context_);
        context_ = nullptr;
        return;
    }

    if ((ret = gp_abilities_list_load(abilities_, context_)) != GP_OK) {
        LOGE("Context: failed to load abilities, ret=%{public}d", ret);
        gp_abilities_list_free(abilities_);
        abilities_ = nullptr;
        gp_camera_unref(camera_);
        camera_ = nullptr;
        gp_context_unref(context_);
        context_ = nullptr;
        return;
    }

    uid_ = getuid();
    gid_ = getgid();
}

Context::~Context()
{
    LOGI("gphotofs ~context start");
    if (abilities_) {
        gp_abilities_list_free(abilities_);
    }
    if (camera_) {
        gp_camera_exit(camera_, context_ ? context_ : NULL);
        gp_camera_unref(camera_);
    }
    if (context_) {
        gp_context_unref(context_);
    }
    if (statCache_) {
        delete statCache_;
    }
    LOGI("gphotofs ~context end");
}