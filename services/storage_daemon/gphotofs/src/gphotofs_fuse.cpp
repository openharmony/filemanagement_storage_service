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
#include "gphotofs_fuse.h"

#include "gphotofs2.h"
#include "storage_service_log.h"
#include "utils.h"

GphotoFileSystem::GphotoFileSystem() : args_(FUSE_ARGS_INIT(0, nullptr)) {}
GphotoFileSystem::~GphotoFileSystem()
{
    if (args_.argv != nullptr) {
        fuse_opt_free_args(&args_);
    }
}

bool GphotoFileSystem::ParseOptionsInner()
{
    if (options_.mountPoint_ == nullptr) {
        LOGE("ParseOptionsInner: mountPoint is null");
        options_.good_ = false;
        return false;
    }

    fuse_opt_add_arg(&args_, options_.mountPoint_);

    if (options_.verbose_) {
        LOGI("gphotofs verbose mode enabled");
        fuse_opt_add_arg(&args_, "-f");
    }

    --options_.deviceNo_;

    if (options_.deviceNo_ && options_.deviceFile_) {
        LOGE("gphoto return ParseOptionsInner not good ret");
        options_.good_ = false;
        return false;
    }
    options_.good_ = true;
    return true;
}

bool GphotoFileSystem::Exec()
{
    if (!options_.good_) {
        LOGE("gphoto exec return options not good ret");
        return false;
    }
    if (!SmtpfsCheckDir(options_.mountPoint_)) {
        LOGE("gphoto exec return mountpoint not good");
        return false;
    }

    if (!CreateTmpDir()) {
        LOGE("gphoto exec Can not create tmp");
        return false;
    }
    int ret = fuse_main(args_.argc, args_.argv, &fuseOperations_, nullptr);
    if (!RemoveTmpDir()) {
        LOGE("gphoto remove tmpdir fail");
    }
    if (ret != 0) {
        LOGE("gphoto fuse_main fail, ret = %{public}d", ret);
        return false;
    }
    return true;
}

bool GphotoFileSystem::ParseOptions(int argc, char **argv)
{
    auto smtpfsOptKey = [](const char* templ, size_t offset, int value) -> fuse_opt {
        return {templ, offset, value};
    };

    static struct fuse_opt smtpfs_opts[] = {
        smtpfsOptKey("enable-move", offsetof(GphotoFileSystemOptions, enableMove_), 1),
        smtpfsOptKey("--device %i", offsetof(GphotoFileSystemOptions, deviceNo_), 0),
        smtpfsOptKey("-v", offsetof(GphotoFileSystemOptions, verbose_), 1),
        smtpfsOptKey("--verbose", offsetof(GphotoFileSystemOptions, verbose_), 1),
        FUSE_OPT_END
        };

    constexpr int MIN_ARGC = 2;
    if (argc < MIN_ARGC) {
        LOGE("gphoto wrong usage ret");
        options_.good_ = false;
        return false;
    }

    fuse_opt_free_args(&args_);
    args_ = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&args_, &options_, smtpfs_opts, GphotoFileSystemOptions::OptProc) == -1) {
        LOGE("gphoto wrong fuse_opt_parse ret");
        options_.good_ = false;
        return false;
    }

    if (options_.deviceFile_ && !options_.mountPoint_) {
        options_.mountPoint_ = options_.deviceFile_;
        options_.deviceFile_ = nullptr;
    }

    if (!options_.mountPoint_) {
        LOGE("gphoto mount point missing");
        options_.good_ = false;
        return false;
    }
    if (!ParseOptionsInner()) {
        return false;
    }
    return true;
}

GphotoFileSystem::GphotoFileSystemOptions::GphotoFileSystemOptions()
    : good_(false),
      verbose_(false),
      enableMove_(true),
      deviceNo_(1),
      deviceFile_(nullptr),
      mountPoint_(nullptr)
{}

GphotoFileSystem::GphotoFileSystemOptions::~GphotoFileSystemOptions()
{
    if (deviceFile_ != nullptr) {
        free(deviceFile_);
        deviceFile_ = nullptr;
    }
    if (mountPoint_ != nullptr) {
        free(mountPoint_);
        mountPoint_ = nullptr;
    }
}

int GphotoFileSystem::GphotoFileSystemOptions::OptProc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
    if (data == nullptr) {
        return -1;
    }
    GphotoFileSystemOptions *options = static_cast<GphotoFileSystemOptions *>(data);
    if (key == FUSE_OPT_KEY_NONOPT && options != nullptr) {
        if (options->mountPoint_ && options->deviceFile_) {
            return -1;
        }
        if (options->deviceFile_) {
            options->mountPoint_ = strdup(arg);
            if (options->mountPoint_ == nullptr) {
                return -1;
            }
            if (outargs == nullptr) {
                return -1;
            }
            if (fuse_opt_add_arg(outargs, arg) == -1) {
                return -1;
            }
            return 0;
        }
        options->deviceFile_ = strdup(arg);
        if (options->deviceFile_ == nullptr) {
            return -1;
        }
        return 0;
    }
    return 1;
}