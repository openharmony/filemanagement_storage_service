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
constexpr int UPLOAD_RECORD_FALSE_LEN = 5;
constexpr int UPLOAD_RECORD_TRUE_LEN = 4;
constexpr int UPLOAD_RECORD_SUCCESS_SENDING_LEN = 7;
constexpr int32_t FROM_ID = 100;
constexpr int32_t TO_ID = 101;
constexpr int32_t UPLOAD_RECORD_SUCCESS_LEN = 7;

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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int result = mtpFileSystem.GetAttr("/", &buf);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(buf.st_mode, S_IFDIR | PERMISSION_ONE);
    EXPECT_EQ(buf.st_nlink, ST_NLINK_TWO);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetattr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapGetattr_002
 * @tc.desc: Verify the WrapGetattr function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapGetattr_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetattr_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    struct stat buf = {0};
    struct fuse_file_info fi = {0};
    const char* path = "/test/../invalid/path";
    
    int result = instance.fuseOperations_.getattr(path, &buf, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetattr_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.MkNod(path, mode, dev);
    EXPECT_EQ(ret, -EINVAL);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkNod_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapMkNod_002
 * @tc.desc: Verify the WrapMkNod function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapMkNod_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkNod_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    mode_t mode = S_IFREG | 0644;
    dev_t dev = 0;
    
    int result = instance.fuseOperations_.mknod(path, mode, dev);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkNod_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.ChMods(path, mode, &fi);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChMod_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapChMod_002
 * @tc.desc: Verify the WrapChMod function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapChMod_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChMod_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    mode_t mode = 0644;
    struct fuse_file_info fi = {0};
    
    int result = instance.fuseOperations_.chmod(path, mode, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChMod_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.Chown(path, uid, gid, &fi);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChown_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapChown_002
 * @tc.desc: Verify the WrapChown function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapChown_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChown_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    uid_t uid = 1000;
    gid_t gid = 1000;
    struct fuse_file_info fi = {0};
    
    int result = instance.fuseOperations_.chown(path, uid, gid, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapChown_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.Write(path, buf, size, offset, &fileInfo);
    EXPECT_NE(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapWrite_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapWrite_002
 * @tc.desc: Verify the WrapWrite function with invalid file handle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapWrite_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapWrite_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    const char* data = "test data";
    size_t size = strlen(data);
    off_t offset = 0;
    struct fuse_file_info fi = {0};
    fi.fh = 0;
    
    int result = instance.fuseOperations_.write(path, data, size, offset, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapWrite_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.Flush(path, &fileInfo);
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.FSync(path, datasync, &fileInfo);
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.ReleaseDir(path, &fileInfo);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReleaseDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapReleaseDir_002
 * @tc.desc: Verify the WrapReleaseDir function with invalid directory handle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapReleaseDir_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReleaseDir_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    struct fuse_file_info fi = {0};
    fi.fh = 0;
    
    int result = instance.fuseOperations_.releasedir(path, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReleaseDir_002 end";
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
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.FSyncDir(path, datasync, &fileInfo);
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

/**
 * @tc.name: MtpfsFuseTest_SetXAttr_001
 * @tc.desc: Test SetXAttr function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_SetXAttr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_SetXAttr_001 start";

    const char *path = "/mnt/data/external";
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.SetXAttr(NULL, NULL, NULL);
    EXPECT_EQ(ret, -ENOENT);

    ret = mtpFileSystem.SetXAttr(path, "user.uploadCompleted", NULL);
    EXPECT_EQ(ret, -ENOENT);

    ret = mtpFileSystem.SetXAttr(path, "user.isUploadCompleted", NULL);
    EXPECT_EQ(ret, 0);

    ret = mtpFileSystem.SetXAttr(path, "user.rmdir", NULL);
    EXPECT_NE(ret, 0);

    mtpFileSystem.device_.RemoveUploadRecord(path);
    GTEST_LOG_(INFO) << "MtpfsFuseTest_SetXAttr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_GetXAttr_001
 * @tc.desc: Test GetXAttr function when path is valid
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_GetXAttr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_GetXAttr_001 start";

    const char *path = "/mnt/data/external";
    char out[UPLOAD_RECORD_SUCCESS_SENDING_LEN + 1] = { 0 };

    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int ret = mtpFileSystem.GetXAttr(NULL, NULL, NULL, 0);
    EXPECT_EQ(ret, 0);

    ret = mtpFileSystem.GetXAttr(path, "user.uploadCompleted", NULL, 0);
    EXPECT_EQ(ret, UPLOAD_RECORD_SUCCESS_SENDING_LEN);

    ret = mtpFileSystem.GetXAttr(path, "user.isUploadCompleted", NULL, 0);
    EXPECT_EQ(ret, UPLOAD_RECORD_SUCCESS_SENDING_LEN);

    ret = mtpFileSystem.GetXAttr(path, "user.isUploadCompleted", out, sizeof(out));
    EXPECT_EQ(ret, 0);

    mtpFileSystem.SetXAttr(path, "user.isUploadCompleted", NULL);
    ret = mtpFileSystem.GetXAttr(path, "user.isUploadCompleted", out, sizeof(out));
    EXPECT_EQ(ret, UPLOAD_RECORD_SUCCESS_SENDING_LEN);

    mtpFileSystem.device_.SetUploadRecord(path, "success");
    ret = mtpFileSystem.GetXAttr(path, "user.isUploadCompleted", out, sizeof(out));
    EXPECT_EQ(ret, UPLOAD_RECORD_SUCCESS_SENDING_LEN);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_GetXAttr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapGetXAttr_002
 * @tc.desc: Verify the WrapGetXAttr function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapGetXAttr_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetXAttr_002 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    const char* name = "user.test";
    char value[256];
    size_t size = sizeof(value);
    
    int result = instance.fuseOperations_.getxattr(path, name, value, size);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetXAttr_002 end";
}

/**
 * @tc.name: MtpfsFuseTest_InitCurrentUidAndCacheMap_001
 * @tc.desc: Test InitCurrentUidAndCacheMap function when init mtpClientWriteMap success.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_InitCurrentUidAndCacheMap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_InitCurrentUidAndCacheMap_001 start";

    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    mtpFileSystem.InitCurrentUidAndCacheMap();
    EXPECT_TRUE(mtpFileSystem.mtpClientWriteMap_.size() > 0);
    mtpFileSystem.mtpClientWriteMap_.clear();

    GTEST_LOG_(INFO) << "MtpfsFuseTest_InitCurrentUidAndCacheMap_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsCurrentUserReadOnly_001
 * @tc.desc: Test IsCurrentUserReadOnly function when currentUser is not readOnly
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsCurrentUserReadOnly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsCurrentUserReadOnly_001 start";

    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    bool result = mtpFileSystem.IsCurrentUserReadOnly();
    EXPECT_FALSE(result);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsCurrentUserReadOnly_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_AccountSubscriber_OnStateChanged_001
 * @tc.desc: Test OnStateChanged function when currentUid was changed
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_AccountSubscriber_OnStateChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_AccountSubscriber_OnStateChanged_001 start";

    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    std::set<OHOS::AccountSA::OsAccountState> states = { OHOS::AccountSA::OsAccountState::SWITCHED };
    bool withHandShake = true;
    OHOS::AccountSA::OsAccountSubscribeInfo subscribeInfo(states, withHandShake);
    AccountSubscriber accountSubscriber(subscribeInfo);

    OHOS::AccountSA::OsAccountStateData osAccountStateData;
    osAccountStateData.state = OHOS::AccountSA::OsAccountState::INVALID_TYPE;
    accountSubscriber.OnStateChanged(osAccountStateData);

    osAccountStateData.state = OHOS::AccountSA::OsAccountState::SWITCHED;
    osAccountStateData.fromId = FROM_ID;
    osAccountStateData.toId = TO_ID;
    accountSubscriber.OnStateChanged(osAccountStateData);
    EXPECT_EQ(mtpFileSystem.currentUid, TO_ID);
    mtpFileSystem.currentUid = 0;

    GTEST_LOG_(INFO) << "MtpfsFuseTest_AccountSubscriber_OnStateChanged_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_GetXAttrTest_001
 * @tc.desc: 测试当 path 为空时,GetXAttr 应返回 0
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_GetXAttrTest_001, TestSize.Level1)
{
    char out[1024];
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int result = mtpFileSystem.GetXAttr(nullptr, "user.isDirFetched", out, sizeof(out));
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: MtpfsFuseTest_GetXAttrTest_002
 * @tc.desc: 测试当 in 为空时,GetXAttr 应返回 0
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_GetXAttrTest_002, TestSize.Level1)
{
    char out[1024];
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int result = mtpFileSystem.GetXAttr("/path/to/file", nullptr, out, sizeof(out));
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: MtpfsFuseTest_GetXAttrTest_003
 * @tc.desc: 测试当 out 为空时,GetXAttr 应返回 UPLOAD_RECORD_SUCCESS_LEN
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_GetXAttrTest_003, TestSize.Level1)
{
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int result = mtpFileSystem.GetXAttr("/path/to/file", "user.isDirFetched", nullptr, 0);
    EXPECT_EQ(result, UPLOAD_RECORD_SUCCESS_LEN);
}

/**
 * @tc.name: MtpfsFuseTest_GetXAttrTest_004
 * @tc.desc: 测试当 attrKey 无效时,GetXAttr 应返回 0
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_GetXAttrTest_004, TestSize.Level1)
{
    char out[1024];
    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    int result = mtpFileSystem.GetXAttr("/path/to/file", "invalid_attr_key", out, sizeof(out));
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: MtpfsFuseTest_AccountConstraintSubscriber_OnConstraintChanged_001
 * @tc.desc: Test OnConstraintChanged function when currentUid was changed
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_AccountConstraintSubscriber_OnConstraintChanged_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_AccountConstraintSubscriber_OnConstraintChanged_001 start";

    MtpFileSystem& mtpFileSystem = MtpFileSystem::GetInstance();
    const std::set<std::string> constraintSet = { "constraint.mtp.client.write" };
    AccountConstraintSubscriber accountConstraintSubscriber(constraintSet);

    OHOS::AccountSA::OsAccountConstraintStateData osAccountConstraintStateData;
    osAccountConstraintStateData.isEnabled = true;
    osAccountConstraintStateData.localId = 100;
    osAccountConstraintStateData.constraint = "constraint.mtp.client.write";
    accountConstraintSubscriber.OnConstraintChanged(osAccountConstraintStateData);
    EXPECT_EQ(mtpFileSystem.mtpClientWriteMap_[100], 1);

    osAccountConstraintStateData.isEnabled = false;
    accountConstraintSubscriber.OnConstraintChanged(osAccountConstraintStateData);
    EXPECT_EQ(mtpFileSystem.mtpClientWriteMap_[100], 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_AccountConstraintSubscriber_OnConstraintChanged_001 end";
}


/**
 * @tc.name: MtpfsFuseTest_WrapMkDir_001
 * @tc.desc: Verify the WrapMkDir function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapMkDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkDir_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/dir";
    mode_t mode = 0755;
    
    int result = instance.fuseOperations_.mkdir(path, mode);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapMkDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapUnLink_001
 * @tc.desc: Verify the WrapUnLink function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapUnLink_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapUnLink_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../nonexistent/file";
    
    int result = instance.fuseOperations_.unlink(path);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapUnLink_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapRmDir_001
 * @tc.desc: Verify the WrapRmDir function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapRmDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRmDir_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../nonexistent/dir";
    
    int result = instance.fuseOperations_.rmdir(path);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRmDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapReName_001
 * @tc.desc: Verify the WrapReName function when paths are incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapReName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReName_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* oldpath = "/test/../old/path";
    const char* newpath = "/test/../new/path";
    unsigned int flags = 0;
    
    int result = instance.fuseOperations_.rename(oldpath, newpath, flags);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReName_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapUTimens_001
 * @tc.desc: Verify the WrapUTimens function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapUTimens_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapUTimens_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    struct timespec ts[2];
    ts[0].tv_sec = time(nullptr);
    ts[0].tv_nsec = 0;
    ts[1].tv_sec = time(nullptr);
    ts[1].tv_nsec = 0;
    struct fuse_file_info fi = {0};
    
    int result = instance.fuseOperations_.utimens(path, ts, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapUTimens_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapOpen_001
 * @tc.desc: Verify the WrapOpen function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapOpen_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapOpen_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/file";
    struct fuse_file_info fi = {0};
    fi.flags = O_RDONLY;
    
    int result = instance.fuseOperations_.open(path, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapOpen_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapRead_001
 * @tc.desc: Verify the WrapRead function with invalid file handle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapRead_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRead_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    char buf[1024];
    size_t size = 1024;
    off_t offset = 0;
    struct fuse_file_info fi = {0};
    fi.fh = 0;
    
    int result = instance.fuseOperations_.read(path, buf, size, offset, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRead_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapStatfs_001
 * @tc.desc: Verify the WrapStatfs function when statInfo is null.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapStatfs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapStatfs_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    struct statvfs* statInfo = nullptr;
    
    int result = instance.fuseOperations_.statfs(path, statInfo);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapStatfs_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapRelease_001
 * @tc.desc: Verify the WrapRelease function with invalid file handle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapRelease_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRelease_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    struct fuse_file_info fi = {0};
    fi.fh = 0;
    
    int result = instance.fuseOperations_.release(path, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapRelease_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapSetXAttr_001
 * @tc.desc: Verify the WrapSetXAttr function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapSetXAttr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapSetXAttr_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    const char* name = "user.test";
    const char* value = "test_value";
    size_t size = strlen(value);
    int flags = 0;
    
    int result = instance.fuseOperations_.setxattr(path, name, value, size, flags);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapSetXAttr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapGetXAttr_001
 * @tc.desc: Verify the WrapGetXAttr function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapGetXAttr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetXAttr_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    const char* name = "user.test";
    char value[256];
    size_t size = sizeof(value);
    
    int result = instance.fuseOperations_.getxattr(path, name, value, size);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapGetXAttr_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapOpenDir_001
 * @tc.desc: Verify the WrapOpenDir function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapOpenDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapOpenDir_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/dir";
    struct fuse_file_info fi = {0};
    
    int result = instance.fuseOperations_.opendir(path, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapOpenDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapReadDir_001
 * @tc.desc: Verify the WrapReadDir function with invalid directory handle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapReadDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReadDir_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/path";
    void* buf = nullptr;
    fuse_fill_dir_t filler = nullptr;
    off_t offset = 0;
    struct fuse_file_info fi = {0};
    fi.fh = 0;
    
    int result = instance.fuseOperations_.readdir(path, buf, filler, offset, &fi, static_cast<FuseReaddirFlags>(0));
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapReadDir_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_WrapCreate_001
 * @tc.desc: Verify the WrapCreate function when path is incorrect.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_WrapCreate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapCreate_001 start";

    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    const char* path = "/test/../invalid/file";
    mode_t mode = S_IFREG | 0644;
    struct fuse_file_info fi = {0};
    fi.flags = O_CREAT | O_RDWR;
    
    int result = instance.fuseOperations_.create(path, mode, &fi);
    EXPECT_NE(result, 0);

    GTEST_LOG_(INFO) << "MtpfsFuseTest_WrapCreate_001 end";
}
 
/**
 * @tc.name: MtpfsFuseTest_OpenFileInternal_001
 * @tc.desc: Test OpenFileInternal with invalid tmpPath
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_OpenFileInternal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OpenFileInternal_001 start";
 
    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    std::string invalidPath = "/nonexistent/path/to/file";
    struct fuse_file_info fi = {0};
    fi.flags = O_RDONLY;
 
    int result = instance.OpenFileInternal("OpenFileInternalTest", invalidPath, &fi);
    EXPECT_NE(result, 0);
 
    GTEST_LOG_(INFO) << "MtpfsFuseTest_OpenFileInternal_001 end";
}
 
/**
 * @tc.name: MtpfsFuseTest_CleanupTemporaryFile_001
 * @tc.desc: Test CleanupTemporaryFile function
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_CleanupTemporaryFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_CleanupTemporaryFile_001 start";
 
    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    std::string stdPath = "/test/path/file.txt";
    std::string tmpPath = "/tmp/test_mtpfs_cleanup";
 
    // Create a temporary file
    {
        std::ofstream file(tmpPath);
        EXPECT_TRUE(file.is_open());
    }
    EXPECT_EQ(access(tmpPath.c_str(), 0), 0);
 
    // Test CleanupTemporaryFile - should delete the tmp file
    instance.CleanupTemporaryFile(stdPath, tmpPath);
 
    // Verify tmp file is deleted
    EXPECT_NE(access(tmpPath.c_str(), 0), 0);
 
    GTEST_LOG_(INFO) << "MtpfsFuseTest_CleanupTemporaryFile_001 end";
}
 
/**
 * @tc.name: MtpfsFuseTest_CleanupTemporaryFile_002
 * @tc.desc: Test CleanupTemporaryFile with non-existent file
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_CleanupTemporaryFile_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_CleanupTemporaryFile_002 start";
 
    MtpFileSystem& instance = MtpFileSystem::GetInstance();
    std::string stdPath = "/test/path/file2.txt";
    std::string tmpPath = "/tmp/nonexistent_file";
 
    // Test CleanupTemporaryFile with non-existent file (should not crash)
    instance.CleanupTemporaryFile(stdPath, tmpPath);
 
    GTEST_LOG_(INFO) << "MtpfsFuseTest_CleanupTemporaryFile_002 end";
}
 
/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_001
 * @tc.desc: Verify IsFilePathValid returns true for normal relative path.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_001 start";
    EXPECT_TRUE(IsFilePathValid("a.txt"));
    EXPECT_TRUE(IsFilePathValid("dir/a.txt"));
    EXPECT_TRUE(IsFilePathValid("dir/subdir/file.bin"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_001 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_002
 * @tc.desc: Verify IsFilePathValid rejects empty path.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_002 start";
    EXPECT_FALSE(IsFilePathValid(""));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_002 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_003
 * @tc.desc: Verify IsFilePathValid rejects ../ at beginning and in the middle.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_003 start";
    EXPECT_FALSE(IsFilePathValid("../a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir/../a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir/sub/../../a.txt"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_003 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_004
 * @tc.desc: Verify IsFilePathValid rejects /.. at tail.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_004 start";
    EXPECT_FALSE(IsFilePathValid("dir/.."));
    EXPECT_FALSE(IsFilePathValid("dir/sub/.."));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_004 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_005
 * @tc.desc: Verify IsFilePathValid rejects Windows style ..\ path traversal.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_005 start";
    EXPECT_FALSE(IsFilePathValid("..\\a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir\\..\\a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir\\sub\\..\\..\\a.txt"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_005 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_006
 * @tc.desc: Verify IsFilePathValid rejects current directory reference ./ and /./.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_006 start";
    EXPECT_FALSE(IsFilePathValid("./a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir/./a.txt"));
    EXPECT_FALSE(IsFilePathValid("dir/./sub/file.txt"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_006 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_007
 * @tc.desc: Verify IsFilePathValid rejects URL-encoded traversal %2e%2e%2f.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_007 start";
    EXPECT_FALSE(IsFilePathValid("%2e%2e%2fsecret.txt"));          // "../secret.txt"
    EXPECT_FALSE(IsFilePathValid("dir/%2e%2e%2fsecret.txt"));      // "dir/../secret.txt"
    EXPECT_FALSE(IsFilePathValid("%2e%2e%5csecret.txt"));          // "..\secret.txt"（%5c == '\')
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_007 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_008
 * @tc.desc: Verify IsFilePathValid rejects double URL-encoded traversal %252e%252e%252f.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_008 start";
    EXPECT_FALSE(IsFilePathValid("%252e%252e%252fsecret.txt"));     // "%2e%2e%2fsecret.txt" -> "../secret.txt"
    EXPECT_FALSE(IsFilePathValid("dir/%252e%252e%252fsecret.txt"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePath_008 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_010
 * @tc.desc: Verify IsFilePathValid does not over-block benign names containing ".." but not as a segment.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_010 start";
    EXPECT_TRUE(IsFilePathValid("abc..def"));          // 不是 ".." 段
    EXPECT_TRUE(IsFilePathValid("dir/abc..def"));
    EXPECT_TRUE(IsFilePathValid("dir..name/file.txt")); // 不是 ".." 段
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_010 end";
}

/**
 * @tc.name: MtpfsFuseTest_IsFilePathValid_011
 * @tc.desc: Verify IsFilePathValid rejects mixed separators traversal patterns.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsFuseTest, MtpfsFuseTest_IsFilePathValid_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_011 start";
    EXPECT_FALSE(IsFilePathValid("dir\\..\\/a.txt"));   // 混合分隔符仍应识别为 ".." 段
    EXPECT_FALSE(IsFilePathValid("dir/..\\a.txt"));
    GTEST_LOG_(INFO) << "MtpfsFuseTest_IsFilePathValid_011 end";
}
} // STORAGE_DAEMON
} // OHOS
