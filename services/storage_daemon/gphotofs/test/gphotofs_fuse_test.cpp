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

#include <gtest/gtest.h>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include <fuse.h>

#include "gphotofs_fuse.h"
#include "gphotofs2.h"

static constexpr mode_t DEFAULT_FILE_MODE = 0644;
static constexpr mode_t DEFAULT_DIR_MODE = 0755;

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class GphotofsFuseTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: GetInstance_001
 * @tc.desc: GetInstance returns singleton reference
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_001 start";

    auto &a = GphotoFileSystem::GetInstance();
    auto &b = GphotoFileSystem::GetInstance();
    EXPECT_EQ(&a, &b);

    GTEST_LOG_(INFO) << "GetInstance_001 end";
}

/**
 * @tc.name: OptionsCtor_001
 * @tc.desc: Options default values
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, OptionsCtor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsCtor_001 start";

    GphotoFileSystem::GphotoFileSystemOptions opt;
    EXPECT_FALSE(opt.good_);
    EXPECT_FALSE(opt.verbose_);
    EXPECT_TRUE(opt.enableMove_);
    EXPECT_EQ(opt.deviceNo_, 1);
    EXPECT_EQ(opt.deviceFile_, nullptr);
    EXPECT_EQ(opt.mountPoint_, nullptr);

    GTEST_LOG_(INFO) << "OptionsCtor_001 end";
}

/**
 * @tc.name: OptProcNullData_001
 * @tc.desc: OptProc returns -1 when data is null
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, OptProcNullData_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptProcNullData_001 start";

    GphotoFileSystem::GphotoFileSystemOptions opt;
    int ret = opt.OptProc(nullptr, "arg", FUSE_OPT_KEY_NONOPT, nullptr);
    EXPECT_EQ(ret, -1);

    GTEST_LOG_(INFO) << "OptProcNullData_001 end";
}

/**
 * @tc.name: InsufficientArgs_001
 * @tc.desc: ParseOptions returns false when argc is insufficient
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, InsufficientArgs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsufficientArgs_001 start";

    auto &fs = GphotoFileSystem::GetInstance();
    char *argv[] = { const_cast<char *>("gphotofs") };
    int argc = 1;
    bool ok = fs.ParseOptions(argc, argv);
    EXPECT_FALSE(ok);

    GTEST_LOG_(INFO) << "InsufficientArgs_001 end";
}

/**
 * @tc.name: InvalidPath_001
 * @tc.desc: Key fuse hooks return -EINVAL on invalid path like "/a/../b"
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, InvalidPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InvalidPath_001 start";

    const char *badPath = "/a/../b";

    struct stat st {};
    struct fuse_file_info fi {};
    EXPECT_EQ(fuseOperations_.getattr(badPath, &st, &fi), -EINVAL);

    EXPECT_EQ(fuseOperations_.readdir(badPath, nullptr, nullptr, 0, &fi, static_cast<fuse_readdir_flags>(0)), -EINVAL);

    fi.flags = O_RDONLY;
    EXPECT_EQ(fuseOperations_.open(badPath, &fi), -EINVAL);

    char buf[16] {};
    fi.fh = 0;
    EXPECT_EQ(fuseOperations_.read(badPath, buf, sizeof(buf), 0, &fi), -EINVAL);

    GTEST_LOG_(INFO) << "InvalidPath_001 end";
}

/**
 * @tc.name: InvalidPath_002
 * @tc.desc: Key fuse hooks return -EINVAL on invalid path like "/a/../b"
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, InvalidPath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InvalidPath_002 start";

    const char *badPath = "/a/../b";

    struct fuse_file_info fi {};

    const char data[] = "x";
    EXPECT_EQ(fuseOperations_.write(badPath, data, strlen(data), 0, &fi), -EROFS);

    EXPECT_EQ(fuseOperations_.truncate(badPath, 0, &fi), -EROFS);

    fi.flags = O_CREAT | O_RDWR;
    EXPECT_EQ(fuseOperations_.create(badPath, S_IFREG | DEFAULT_FILE_MODE, &fi), -EROFS);

    GTEST_LOG_(INFO) << "InvalidPath_002 end";
}

/**
 * @tc.name: InvalidPath_003
 * @tc.desc: Key fuse hooks return -EINVAL on invalid path like "/a/../b"
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, InvalidPath_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InvalidPath_003 start";

    const char *badPath = "/a/../b";

    EXPECT_EQ(fuseOperations_.mkdir(badPath, DEFAULT_DIR_MODE), -EROFS);
    EXPECT_EQ(fuseOperations_.rmdir(badPath), -EINVAL);
    EXPECT_EQ(fuseOperations_.unlink(badPath), -EINVAL);

    struct statvfs sv {};
    EXPECT_EQ(fuseOperations_.statfs(badPath, &sv), -EINVAL);

    char value[32] {};
    EXPECT_EQ(fuseOperations_.getxattr(badPath, "user.isDirFetched", value, sizeof(value)), -EINVAL);

    GTEST_LOG_(INFO) << "InvalidPath_003 end";
}

/**
 * @tc.name: ReadlinkAlwaysInvalid_001
 * @tc.desc: readlink always returns -EINVAL (camera fs does not support symlink)
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, ReadlinkAlwaysInvalid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadlinkAlwaysInvalid_001 start";

    char buf[64] {};
    EXPECT_EQ(fuseOperations_.readlink("/", buf, sizeof(buf)), -EINVAL);
    EXPECT_EQ(fuseOperations_.readlink("/a/b", buf, sizeof(buf)), -EINVAL);

    GTEST_LOG_(INFO) << "ReadlinkAlwaysInvalid_001 end";
}

/**
 * @tc.name: FixedReturn_001
 * @tc.desc: Hooks that return fixed values
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, FixedReturn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FixedReturn_001 start";

    struct fuse_file_info fi {};

    EXPECT_EQ(fuseOperations_.chmod("/any", DEFAULT_FILE_MODE, &fi), 0);
    EXPECT_EQ(fuseOperations_.chown("/any", 1000, 1000, &fi), 0);
    EXPECT_EQ(fuseOperations_.flush("/any", &fi), 0);
    EXPECT_EQ(fuseOperations_.access("/any", 0), 0);

    char linkBuf[8] {};
    EXPECT_EQ(fuseOperations_.readlink("/any", linkBuf, sizeof(linkBuf)), -EINVAL);

    EXPECT_EQ(fuseOperations_.mknod("/any", 0, 0), -EROFS);
    EXPECT_EQ(fuseOperations_.symlink("/any_src", "/any_dst"), -EROFS);
    EXPECT_EQ(fuseOperations_.link("/any_src", "/any_dst"), -EROFS);

    EXPECT_EQ(fuseOperations_.rename("/old", "/new", 0), -EROFS);

    GTEST_LOG_(INFO) << "FixedReturn_001 end";
}

/**
 * @tc.name: GetattrNullPath_001
 * @tc.desc: getattr returns -EINVAL when path is null
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, GetattrNullPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetattrNullPath_001 start";

    struct stat st {};
    struct fuse_file_info fi {};
    EXPECT_EQ(fuseOperations_.getattr(nullptr, &st, &fi), -EINVAL);

    GTEST_LOG_(INFO) << "GetattrNullPath_001 end";
}

/**
 * @tc.name: GetxattrNullArgs_001
 * @tc.desc: getxattr returns -EINVAL when path or name is null
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, GetxattrNullArgs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetxattrNullArgs_001 start";

    char value[32] {};

    // path is null
    EXPECT_EQ(fuseOperations_.getxattr(nullptr, "user.isDirFetched", value, sizeof(value)), -EINVAL);

    // name(in) is null
    EXPECT_EQ(fuseOperations_.getxattr("/any", nullptr, value, sizeof(value)), -EINVAL);

    GTEST_LOG_(INFO) << "GetxattrNullArgs_001 end";
}

/**
 * @tc.name: GetxattrUnknownKey_001
 * @tc.desc: getxattr returns 0 when key is not recognized (and path is valid)
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, GetxattrUnknownKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetxattrUnknownKey_001 start";

    char value[32] {};
    // For unknown key, implementation returns 0 without accessing fuse context
    EXPECT_EQ(fuseOperations_.getxattr("/any", "user.unknownKey", value, sizeof(value)), 0);

    GTEST_LOG_(INFO) << "GetxattrUnknownKey_001 end";
}

/**
 * @tc.name: XattrFixedReturn_001
 * @tc.desc: setxattr/removexattr return -EROFS, listxattr returns 0 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, XattrFixedReturn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "XattrFixedReturn_001 start";

    EXPECT_EQ(fuseOperations_.setxattr("/any", "user.k", "v", 1, 0), -EROFS);
    EXPECT_EQ(fuseOperations_.removexattr("/any", "user.k"), -EROFS);

    char listBuf[64] {};
    EXPECT_EQ(fuseOperations_.listxattr("/any", listBuf, sizeof(listBuf)), 0);

    GTEST_LOG_(INFO) << "XattrFixedReturn_001 end";
}

/**
 * @tc.name: DirOpsFixedReturn_001
 * @tc.desc: opendir/releasedir/fsyncdir return 0
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, DirOpsFixedReturn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DirOpsFixedReturn_001 start";

    struct fuse_file_info fi {};
    EXPECT_EQ(fuseOperations_.opendir("/a/../b", &fi), -EINVAL);
    EXPECT_EQ(fuseOperations_.releasedir("/any", &fi), 0);
    EXPECT_EQ(fuseOperations_.fsyncdir("/any", 0, &fi), 0);

    GTEST_LOG_(INFO) << "DirOpsFixedReturn_001 end";
}

/**
 * @tc.name: MiscFixedReturn_001
 * @tc.desc: fsync/utimens/ioctl return 0, cancel return 0
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, MiscFixedReturn_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MiscFixedReturn_001 start";

    struct fuse_file_info fi {};
    struct timespec ts[2] {};

    EXPECT_EQ(fuseOperations_.fsync("/any", 0, &fi), 0);
    EXPECT_EQ(fuseOperations_.utimens("/any", ts, &fi), 0);
    EXPECT_EQ(fuseOperations_.ioctl("/any", 0, nullptr, &fi, 0, nullptr), 0);

    GTEST_LOG_(INFO) << "MiscFixedReturn_001 end";
}

/**
 * @tc.name: MiscFixedReturn_002
 * @tc.desc: poll/lock/flock return 0, cancel return 0
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, MiscFixedReturn_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MiscFixedReturn_002 start";

    struct fuse_file_info fi {};
    struct flock fl {};

    EXPECT_EQ(fuseOperations_.poll("/any", &fi, nullptr, nullptr), 0);

    EXPECT_EQ(fuseOperations_.lock("/any", &fi, 0, &fl), 0);
    EXPECT_EQ(fuseOperations_.flock("/any", &fi, 0), 0);

    GTEST_LOG_(INFO) << "MiscFixedReturn_002 end";
}

/**
 * @tc.name: WriteInReadOnly_001
 * @tc.desc: write/truncate/create/mkdir should return -EROFS in read-only mode even for valid path
 * @tc.type: FUNC
 */
HWTEST_F(GphotofsFuseTest, WriteInReadOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteInReadOnly_001 start";

    const char *path = "/valid";
    struct fuse_file_info fi {};
    const char data[] = "abc";

    EXPECT_EQ(fuseOperations_.write(path, data, strlen(data), 0, &fi), -EROFS);
    EXPECT_EQ(fuseOperations_.truncate(path, 0, &fi), -EROFS);

    fi.flags = O_CREAT | O_RDWR;
    EXPECT_EQ(fuseOperations_.create(path, S_IFREG | DEFAULT_FILE_MODE, &fi), -EROFS);
    EXPECT_EQ(fuseOperations_.mkdir(path, DEFAULT_DIR_MODE), -EROFS);

    GTEST_LOG_(INFO) << "WriteInReadOnly_001 end";
}

} // namespace StorageDaemon
} // namespace OHOS