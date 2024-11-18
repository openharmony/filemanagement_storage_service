/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <fstream>
#include <gtest/gtest.h>
#include <fuse_opt.h>
#include "mtpfs_fuse.h"
#include <unistd.h>
#include "mtpfs_util.h"
#include "storage_service_log.h"

#define PERMISSION_ONE 0775
#define PERMISSION_TWO 0644

const int32_t ST_NLINK_TWO = 2;
const int32_t FILE_SIZE = 512;
const int32_t BS_SIZE = 1024;
const int32_t ARG_SIZE = 2;

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;

class MtpfsFuseTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
};

/**
 * @tc.name: MtpfsFuseTest_WrapGetattr_001
 * @tc.desc: Verify the WrapGetattr function when msg is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapGetattr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetattr_001 start";

    struct stat buf;
    struct fuse_file_info fi;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int result = mtpFileSystem->GetAttr("/", &buf, &fi);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(buf.st_mode, S_IFDIR | PERMISSION_ONE);
    EXPECT_EQ(buf.st_nlink, ST_NLINK_TWO);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetattr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapMkNod_001
 * @tc.desc: Verify the WrapMkNod function when msg is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapMkNod_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkNod_001 start";

    const char *path = "/mnt/data/external";
    mode_t mode = S_IFDIR;
    dev_t dev = 1;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->MkNod(path, mode, dev);
    EXPECT_EQ(ret, -EINVAL);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkNod_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapChMod_001
 * @tc.desc: Test WrapChMod function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapChMod_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChMod_001 start";

    const char *path = "/mnt/data/external";
    mode_t mode = 0755;
    struct fuse_file_info fi;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->ChMods(path, mode, &fi);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChMod_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapChown_001
 * @tc.desc: Test WrapChown function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapChown_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChown_001 start";

    const char *path = "/mnt/data/external";
    uid_t uid = 1000;
    gid_t gid = 1000;
    struct fuse_file_info fi;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->Chown(path, uid, gid, &fi);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChown_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapWrite_001
 * @tc.desc: Test WrapWrite function when path is invalid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapWrite_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapWrite_001 start";

    const char *path = "/mnt/data/external";
    const char *buf = "test buffer";
    size_t size = strlen(buf);
    off_t offset = 0;
    struct fuse_file_info fileInfo;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->Write(path, buf, size, offset, &fileInfo);
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapWrite_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapFlush_001
 * @tc.desc: Test WrapFlush function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapFlush_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFlush_001 start";

    const char *path = "/mnt/data/external";
    struct fuse_file_info fileInfo;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->Flush(path, &fileInfo);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFlush_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapFSync_001
 * @tc.desc: Test WrapFSync function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapFSync_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFSync_001 start";

    const char *path = "/mnt/data/external";
    int datasync = 1;
    struct fuse_file_info fileInfo;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->FSync(path, datasync, &fileInfo);
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFSync_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapReleaseDir_001
 * @tc.desc: Test WrapReleaseDir function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapReleaseDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReleaseDir_001 start";

    const char *path = "/mnt/data/external";
    struct fuse_file_info fileInfo;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->ReleaseDir(path, &fileInfo);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReleaseDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapFSyncDir_001
 * @tc.desc: Test WrapFSyncDir function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapFSyncDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFSyncDir_001 start";

    const char *path = "/mnt/data/external";
    struct fuse_file_info fileInfo;
    int datasync = 1;
    auto mtpFileSystem = DelayedSingleton<MtpFileSystem>::GetInstance();
    int ret = mtpFileSystem->FSyncDir(path, datasync, &fileInfo);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapFSyncDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_OptProc_001
 * @tc.desc: Test OptProc function
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_OptProc_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_001 start";

    MtpFileSystem::MtpFileSystemOptions options;
    options.mountPoint_ = new char[10];
    options.deviceFile_ = new char[10];
    int result = options.OptProc(&options, "arg", FUSE_OPT_KEY_NONOPT, nullptr);
    EXPECT_EQ(result, -1);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_OptProc_002
 * @tc.desc: Test OptProc function
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_OptProc_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_002 start";

    MtpFileSystem::MtpFileSystemOptions options;
    options.deviceFile_ = new char[10];
    int result = options.OptProc(&options, "arg", FUSE_OPT_KEY_NONOPT, nullptr);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_002 end";
}

/**
 * @tc.name: MtpfsFuseTest_OptProc_003
 * @tc.desc: Test OptProc function
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_OptProc_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_003 start";

    MtpFileSystem::MtpFileSystemOptions options;
    options.mountPoint_ = new char[10];
    int result = options.OptProc(&options, "arg", FUSE_OPT_KEY_NONOPT, nullptr);
    EXPECT_EQ(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_003 end";
}

/**
 * @tc.name: MtpfsFuseTest_OptProc_004
 * @tc.desc: Test OptProc function
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_OptProc_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_004 start";

    MtpFileSystem::MtpFileSystemOptions options;
    int result = options.OptProc(&options, "arg", 0, nullptr);
    EXPECT_EQ(result, 1);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_OptProc_004 end";
}
} // STORAGE_DAEMON
} // OHOS