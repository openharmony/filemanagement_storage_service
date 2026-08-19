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

#include <cstdarg>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/dm-ioctl.h>
#include <linux/fs.h>
#include <securec.h>
#include <sys/ioctl.h>
#include <sys/sysmacros.h>

#include "dm_device_mock.h"

namespace OHOS {
namespace StorageDaemon {

MockConfig g_mock;

extern "C" int __real_open(const char *path, int flags, ...);

extern "C" int __wrap_open(const char *path, int flags, ...)
{
    if (!g_mock.mockEnabled) {
        // [A] O_CREAT 时需要 mode 参数
        if (flags & O_CREAT) {
            va_list args;
            va_start(args, flags);
            int mode = va_arg(args, int);
            va_end(args);
            return __real_open(path, flags, mode);
        }
        return __real_open(path, flags);
    }
    // mock 模式：只拦截已知的设备路径，其余透传 [B]
    if (strcmp(path, "/dev/mapper/control") == 0) {
        if (g_mock.openControlFail) {
            errno = EACCES;
            return -1;
        }
        return 100;
    }
    if (strncmp(path, "/dev/block/", strlen("/dev/block/")) == 0) {
        if (g_mock.openSourceFail) {
            errno = ENOENT;
            return -1;
        }
        return 200;
    }
    // 非设备路径（如 sysfs 文件）透传给真实实现
    if (flags & O_CREAT) {
        va_list args;
        va_start(args, flags);
        int mode = va_arg(args, int);
        va_end(args);
        return __real_open(path, flags, mode);
    }
    return __real_open(path, flags);
}

extern "C" int __real_close(int fd);

extern "C" int __wrap_close(int fd)
{
    if (!g_mock.mockEnabled) {
        return __real_close(fd);
    }
    // mock 模式：只对 mock fd 计数，其余透传
    if (fd == 100 || fd == 200) {
        g_mock.closeCount++;
        return 0;
    }
    return __real_close(fd);
}

extern "C" int __real_ioctl(int fd, unsigned long request, ...);

extern "C" int __wrap_ioctl(int fd, unsigned long request, ...)
{
    if (!g_mock.mockEnabled) {
        // 透传模式：Linux ioctl 系统调用 ABI 保证第三个参数通过寄存器传递，
        // 即使调用者未显式提供也可安全读取，故仍按 void* 取参后透传
        va_list args;
        va_start(args, request);
        void *argp = va_arg(args, void *);
        va_end(args);
        return __real_ioctl(fd, request, argp);
    }

    // mock 模式：仅对已知 request 提取参数，避免无第三参数时 va_arg 的 UB
    if (request == BLKGETSIZE64) {
        va_list args;
        va_start(args, request);
        uint64_t *bytes = static_cast<uint64_t *>(va_arg(args, void *));
        va_end(args);
        if (g_mock.blkGetSize64Fail) {
            errno = EIO;
            return -1;
        }
        *bytes = g_mock.deviceBytes;
        return 0;
    }

    switch (request) {
        case DM_DEV_STATUS:
        case DM_DEV_CREATE:
        case DM_TABLE_LOAD:
        case DM_DEV_SUSPEND:
        case DM_DEV_REMOVE: {
            va_list args;
            va_start(args, request);
            struct dm_ioctl *dm = static_cast<struct dm_ioctl *>(va_arg(args, void *));
            va_end(args);
            if (request == DM_DEV_STATUS) {
                if (g_mock.statusFail) {
                    errno = ENXIO;
                    return -1;
                }
                dm->dev = makedev(253, 0);
                return 0;
            }
            if (request == DM_DEV_CREATE) {
                if (g_mock.createFail) {
                    errno = EBUSY;
                    return -1;
                }
                dm->dev = makedev(253, 0);
                return 0;
            }
            if (request == DM_TABLE_LOAD) {
                if (g_mock.loadTableFail) {
                    errno = EINVAL;
                    return -1;
                }
                return 0;
            }
            if (request == DM_DEV_SUSPEND) {
                if (g_mock.resumeFail) {
                    errno = EINVAL;
                    return -1;
                }
                return 0;
            }
            // DM_DEV_REMOVE
            g_mock.dmRemoveCalled = true;
            if (g_mock.removeFail) {
                errno = EBUSY;
                return -1;
            }
            return 0;
        }
        default:
            // mock 模式下未知 request 不提取参数、不调真实 ioctl（假 fd 会 UB）
            errno = ENOTTY;
            return -1;
    }
}

extern "C" errno_t __real_memset_s(void *s, size_t smax, int c, size_t n);

extern "C" errno_t __wrap_memset_s(void *s, size_t smax, int c, size_t n)
{
    if (!g_mock.mockEnabled) {
        return __real_memset_s(s, smax, c, n);
    }
    if (g_mock.memsetSFail) {
        return EINVAL;
    }
    return __real_memset_s(s, smax, c, n);
}

extern "C" errno_t __real_strncpy_s(char *dest, size_t destMax, const char *src, size_t count);

extern "C" errno_t __wrap_strncpy_s(char *dest, size_t destMax, const char *src, size_t count)
{
    if (!g_mock.mockEnabled) {
        return __real_strncpy_s(dest, destMax, src, count);
    }
    if (g_mock.ShouldStrncpySFail()) {
        return EINVAL;
    }
    return __real_strncpy_s(dest, destMax, src, count);
}

extern "C" errno_t __real_strcpy_s(char *dest, size_t destMax, const char *src);

extern "C" errno_t __wrap_strcpy_s(char *dest, size_t destMax, const char *src)
{
    if (!g_mock.mockEnabled) {
        return __real_strcpy_s(dest, destMax, src);
    }
    if (g_mock.ShouldStrcpySFail()) {
        return EINVAL;
    }
    return __real_strcpy_s(dest, destMax, src);
}
}  // namespace StorageDaemon
}  // namespace OHOS
