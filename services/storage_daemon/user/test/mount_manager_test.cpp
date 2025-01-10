/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "user/mount_manager.h"

#include <fcntl.h>
#include <gtest/gtest.h>

#include "directory_ex.h"

#include "file_utils_mock.h"
#include "library_func_mock.h"
#include "storage_service_errno.h"

using namespace std;
namespace {
    const string SANDBOX_ROOT_PATH = "/mnt/sandbox/";
    const string EL2_BASE = "/data/storage/el2/base/";
}

namespace OHOS {
namespace StorageDaemon {
using namespace testing;
using namespace testing::ext;

class MountManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
    static inline std::shared_ptr<LibraryFuncMock> libraryFuncMock_ = nullptr;
    static inline shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
};

void MountManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
    LibraryFuncMock::libraryFunc_ = libraryFuncMock_;
}

void MountManagerTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    LibraryFuncMock::libraryFunc_ = nullptr;
    libraryFuncMock_ = nullptr;
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmdfsTwiceMount_001
 * @tc.desc: Verify the HmdfsTwiceMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmdfsTwiceMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsTwiceMount_001 start";
    int32_t userId = 888;
    std::string relativePath;

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true));
    auto ret = MountManager::GetInstance()->HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_SharefsMount_001
 * @tc.desc: Verify the SharefsMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_SharefsMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_SharefsMount_001 start";
    int32_t userId = 888;

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    auto ret = mountManager->SharefsMount(userId);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->SharefsMount(userId);
    EXPECT_EQ(ret, E_MOUNT_SHAREFS);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->SharefsMount(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_SharefsMount_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmSharefsMount_001
 * @tc.desc: Verify the Instance function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmSharefsMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_001 start";
    int32_t userId = 888;
    std::string srcPath;
    std::string dstPath;

    std::shared_ptr<MountManager> mountManager = MountManager::GetInstance();
    ASSERT_TRUE(mountManager != nullptr);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance()->HmSharefsMount(userId, srcPath, dstPath);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = MountManager::GetInstance()->HmSharefsMount(userId, srcPath, dstPath);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance()->HmSharefsMount(userId, srcPath, dstPath);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->HmSharefsMount(userId, srcPath, dstPath);
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->HmSharefsMount(userId, srcPath, dstPath);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_CheckSymlink_001
 * @tc.desc: Verify the CheckSymlink function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_CheckSymlink_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckSymlink_001 start";
    std::string path;
    std::list<std::string> unMountFailList;
    auto ret = MountManager::GetInstance()->CheckSymlink(path, unMountFailList);
    EXPECT_EQ(ret, false);

    path = "/data/test/tdd/test.txt";
    ret = MountManager::GetInstance()->CheckSymlink(path, unMountFailList);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckSymlink_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_SharedMount_001
 * @tc.desc: Verify the SharedMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_SharedMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_SharedMount_001 start";
    std::string path;
    auto ret = MountManager::GetInstance()->SharedMount(path);
    EXPECT_EQ(ret, E_OK);

    path = "test";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    ret = MountManager::GetInstance()->SharedMount(path);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->SharedMount(path);
    EXPECT_EQ(ret, 1);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->SharedMount(path);
    EXPECT_EQ(ret, 1);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->SharedMount(path);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckSymlink_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_BindAndRecMount_001
 * @tc.desc: Verify the BindAndRecMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_BindAndRecMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_BindAndRecMount_001 start";
    std::string srcPath;
    std::string dstPath;
    int32_t userId = 100;
    bool isUseSlave = true;
    auto ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    srcPath = "srcPath";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    dstPath = "dstPath";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckSymlink_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_BindAndRecMount_002
 * @tc.desc: Verify the BindAndRecMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_BindAndRecMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_BindAndRecMount_002 start";
    std::string srcPath = "srcPath";
    std::string dstPath = "dstPath";
    int32_t userId = 100;
    bool isUseSlave = true;

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    auto ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, 1);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, 1);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    isUseSlave = false;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_BindAndRecMount_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_PrepareAppdataDirByUserId_001
 * @tc.desc: Verify the PrepareAppdataDirByUserId function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_PrepareAppdataDirByUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_PrepareAppdataDirByUserId_001 start";
    int32_t userId = 888;
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance()->PrepareAppdataDirByUserId(userId);
    EXPECT_EQ(ret, E_PREPARE_DIR);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).Times(MountManager::GetInstance()->appdataDir_.size())
        .WillRepeatedly(::testing::Return(11));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(false));
    ret = MountManager::GetInstance()->PrepareAppdataDirByUserId(userId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckSymlink_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_PrepareAppdataDir_001
 * @tc.desc: Verify the PrepareAppdataDir function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_PrepareAppdataDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_PrepareAppdataDir_001 start";
    int32_t userId = 888;
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance()->PrepareAppdataDir(userId);
    EXPECT_EQ(ret, E_OK);

    std::vector<int32_t> userIds;
    MountManager::GetInstance()->GetAllUserId(userIds);
    if (!userIds.empty()) {
        EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillRepeatedly(Return(false));
    }

    ret = MountManager::GetInstance()->PrepareAppdataDir(0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_PrepareAppdataDir_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_DirExist_001
 * @tc.desc: Verify the DirExist function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_DirExist_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_DirExist_001 start";
    string path = "test";
    auto ret = MountManager::GetInstance()->DirExist(path);
    EXPECT_FALSE(ret);

    path = "/data/test";
    ret = MountManager::GetInstance()->DirExist(path);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_DirExist_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_CheckPathValid_001
 * @tc.desc: Verify the CheckPathValid function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_CheckPathValid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckPathValid_001 start";
    std::string bundleNameStr = "test";
    uint32_t userId = 888;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance()->CheckPathValid(bundleNameStr, userId);
    EXPECT_FALSE(ret);

    string basePath =
        SANDBOX_ROOT_PATH + to_string(userId);
    string completePath = basePath  + "/" + bundleNameStr + EL2_BASE;
    ForceCreateDirectory(completePath);
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance()->CheckPathValid(bundleNameStr, userId);
    EXPECT_TRUE(ret);

    string subDir = completePath + "test";
    ForceCreateDirectory(subDir);
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance()->CheckPathValid(bundleNameStr, userId);
    EXPECT_FALSE(ret);
    ForceRemoveDirectory(basePath);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckPathValid_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_CheckMaps_001
 * @tc.desc: Verify the CheckMaps function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_CheckMaps_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckMaps_001 start";
    std::string path;
    std::list<std::string> mountFailList;
    auto ret = MountManager::GetInstance()->CheckMaps("", mountFailList);
    EXPECT_FALSE(ret);

    string baseDir = "/data/test/tdd";
    ForceCreateDirectory(baseDir);
    path = baseDir + "test.txt";
    ret = MountManager::GetInstance()->CheckMaps(path, mountFailList);
    EXPECT_FALSE(ret);

    auto fd = open(path.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);

    std::string content = "this is a test\n/data/test/tdd";
    (void)write(fd, content.c_str(), content.size());
    close(fd);
    ret = MountManager::GetInstance()->CheckMaps(path, mountFailList);
    EXPECT_FALSE(ret);

    mountFailList.push_back(baseDir);
    ret = MountManager::GetInstance()->CheckMaps(path, mountFailList);
    EXPECT_TRUE(ret);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_CheckMaps_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_MountSandboxPath_001
 * @tc.desc: Verify the MountSandboxPath function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_MountSandboxPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_MountSandboxPath_001 start";
    std::vector<std::string> srcPathEmpty;
    std::vector<std::string> dstPathEmpty;
    std::string bundleName = "test";
    std::string userId = "100";
    MountManager::GetInstance()->MountSandboxPath(srcPathEmpty, dstPathEmpty, bundleName, userId);

    std::vector<std::string> srcPaths {
        "/data/test/tdd1",
        "/data/test/tdd2"
    };
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPathEmpty, bundleName, userId);
    
    std::vector<std::string> dstPaths {
        "/data/test/tdd3"
    };
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_MountSandboxPath_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_MountSandboxPath_002
 * @tc.desc: Verify the MountSandboxPath function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_MountSandboxPath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_MountSandboxPath_002 start";
    std::string bundleName = "test";
    std::string userId = "100";

    std::vector<std::string> srcPaths {
        "/data/test/tdd1",
    };
    
    std::vector<std::string> dstPaths {
        "/data/test/tdd2"
    };
    
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    MountManager::GetInstance()->MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_MountSandboxPath_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_UMountByListWithDetach_001
 * @tc.desc: Verify the UMountByListWithDetach function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_UMountByListWithDetach_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_UMountByListWithDetach_001 start";
    std::list<std::string> list;
    EXPECT_EQ(MountManager::GetInstance()->UMountByListWithDetach(list), E_OK);
    
    list.push_back("test");
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(1));
    auto ret = MountManager::GetInstance()->UMountByListWithDetach(list);
    if (errno != ENOENT && errno != EINVAL) {
        EXPECT_EQ(ret, errno);
    } else {
        EXPECT_EQ(ret, E_OK);
    }
    
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance()->UMountByListWithDetach(list);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_UMountByListWithDetach_001 end";
}
} // STORAGE_DAEMON
} // OHOS
