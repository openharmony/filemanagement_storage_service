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
#ifndef GPHOTOFS_FUSE_H
#define GPHOTOFS_FUSE_H
#include <string>
#include <fuse.h>

class GphotoFileSystem {
    enum {
        KEY_ENABLE_MOVE = -1000,
        KEY_DEVICE_NO,
        KEY_LIST_DEVICES,
        KEY_VERBOSE,
        KEY_VERSION,
        KEY_HELP
    };
public:
    static GphotoFileSystem &GetInstance()
    {
        static GphotoFileSystem instance;
        return instance;
    }
    bool ParseOptions(int argc, char **argv);
    bool Exec();
    bool ParseOptionsInner();

private:
    struct GphotoFileSystemOptions {
    public:
        int good_;
        int verbose_;
        int enableMove_;
        int deviceNo_;
        char *deviceFile_;
        char *mountPoint_;

        GphotoFileSystemOptions();
        ~GphotoFileSystemOptions();

        static int OptProc(void *data, const char *arg, int key, struct fuse_args *outargs);
    };
    GphotoFileSystem();
    ~GphotoFileSystem();
    struct fuse_args args_;
    GphotoFileSystemOptions options_;
};
#endif // GPHOTOFS_FUSE_H