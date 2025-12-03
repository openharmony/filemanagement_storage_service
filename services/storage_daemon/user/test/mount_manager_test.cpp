/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "os_account_manager.h"
#include "parameter_mock.h"
#include "storage_service_errno.h"
#include "string_utils.h"
#include "utils/mount_argument_utils.h"

using namespace std;
namespace {
    const string SANDBOX_ROOT_PATH = "/mnt/sandbox/";
    const string EL2_BASE = "/data/storage/el2/base/";
}

namespace OHOS {
int32_t g_checkOsAccountConstraintEnabled;
bool g_isEnabled = false;

namespace AccountSA {
ErrCode OsAccountManager::CheckOsAccountConstraintEnabled(const int id, const std::string &constraint, bool &isEnabled)
{
    isEnabled = g_isEnabled;
    return g_checkOsAccountConstraintEnabled;
}
}
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
    static inline shared_ptr<ParamMoc> paramMoc_ = nullptr;
};

void MountManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    fileUtilMoc_ = make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    libraryFuncMock_ = std::make_shared<LibraryFuncMock>();
    LibraryFuncMock::libraryFunc_ = libraryFuncMock_;
    paramMoc_ = std::make_shared<ParamMoc>();
    ParamMoc::paramMoc_ = paramMoc_;
}

void MountManagerTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    LibraryFuncMock::libraryFunc_ = nullptr;
    libraryFuncMock_ = nullptr;
    ParamMoc::paramMoc_ = nullptr;
    paramMoc_ = nullptr;
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_GetInstance_001
 * @tc.desc: Verify the GetInstance function.
 * @tc.type: FUNC
 * @tc.require: AR000HSKSO
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_GetInstance_001 start";

    MountManager &mountManager1 = MountManager::GetInstance();
    MountManager &mountManager2 = MountManager::GetInstance();
    ASSERT_TRUE(&mountManager1 == &mountManager2);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_GetInstance_001 end";
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
    auto ret = MountManager::GetInstance().HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_MOUNT_HMDFS_MEDIA);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance().HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_MOUNT_HMDFS_MEDIA);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance().HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_MOUNT_HMDFS_MEDIA);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance().HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_MOUNT_HMDFS_MEDIA);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true)).WillOnce(Return(false))
        .WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance().HmdfsTwiceMount(userId, relativePath);
    EXPECT_EQ(ret, E_MOUNT_HMDFS_MEDIA);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsTwiceMount_001 end";
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

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    auto ret = MountManager::GetInstance().SharefsMount(userId);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance().SharefsMount(userId);
    EXPECT_EQ(ret, E_MOUNT_SHAREFS);

    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().SharefsMount(userId);
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

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, false, "");
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, false, "");
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, false, "");
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, false, "");
    EXPECT_EQ(ret, E_MOUNT_HM_SHAREFS);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, false, "");
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmSharefsMount_002
 * @tc.desc: Verify the Instance function when isUseShared is true.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmSharefsMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_002 start";
    int32_t userId = 888;
    std::string srcPath;
    std::string dstPath;

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true))
        .WillOnce(Return(true)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    auto ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, true, "");
    EXPECT_EQ(ret, E_OK);
    ret = MountManager::GetInstance().HmSharefsMount(userId, srcPath, dstPath, true, "");
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmSharefsMount_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmdfsMount_001
 * @tc.desc: Verify the HmdfsMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmdfsMount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_001 start";
    int32_t userId = 901;
    std::string relativePath = "";
    bool mountCloudDisk = false;
    g_checkOsAccountConstraintEnabled = E_MOUNT_HMDFS;
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    auto ret = MountManager::GetInstance().HmdfsMount(userId, relativePath, mountCloudDisk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmdfsMount_002
 * @tc.desc: Verify the HmdfsMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmdfsMount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_002 start";
    int32_t userId = 901;
    std::string relativePath = "";
    bool mountCloudDisk = false;
    g_checkOsAccountConstraintEnabled = E_OK;
    g_isEnabled = true;
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    auto ret = MountManager::GetInstance().HmdfsMount(userId, relativePath, mountCloudDisk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_HmdfsMount_003
 * @tc.desc: Verify the HmdfsMount function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_HmdfsMount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_003 start";
    int32_t userId = 901;
    std::string relativePath = "";
    bool mountCloudDisk = false;
    g_checkOsAccountConstraintEnabled = E_OK;
    g_isEnabled = false;
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    auto ret = MountManager::GetInstance().HmdfsMount(userId, relativePath, mountCloudDisk);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_HmdfsMount_003 end";
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
    auto ret = MountManager::GetInstance().CheckSymlink(path, unMountFailList);
    EXPECT_EQ(ret, false);

    path = "/data/test/tdd/test.txt";
    ret = MountManager::GetInstance().CheckSymlink(path, unMountFailList);
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
    int32_t userId = 100;
    auto ret = MountManager::GetInstance().SharedMount(userId, path);
    EXPECT_EQ(ret, E_OK);

    path = "test1";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    ret = MountManager::GetInstance().SharedMount(userId, path);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance().SharedMount(userId, path);
    EXPECT_EQ(ret, E_MOUNT_SHARED);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance().SharedMount(userId, path);
    EXPECT_EQ(ret, E_MOUNT_SHARED);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance().SharedMount(userId, path);
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
    auto ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_NON_EXIST);

    srcPath = "srcPath";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_NON_EXIST);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_NON_EXIST);

    dstPath = "dstPath";
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_NON_EXIST);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
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
    auto ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_MOUNT_BIND_AND_REC);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, 1);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
    EXPECT_EQ(ret, E_OK);

    isUseSlave = false;
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    ret = MountManager::GetInstance().BindAndRecMount(userId, srcPath, dstPath, isUseSlave);
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
    auto ret = MountManager::GetInstance().PrepareAppdataDirByUserId(userId);
    EXPECT_EQ(ret, E_CREATE_DIR_APPDATA);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).Times(MountManager::GetInstance().appdataDir_.size())
        .WillRepeatedly(::testing::Return(11));
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false)).WillOnce(Return(false)).WillOnce(Return(false));
    ret = MountManager::GetInstance().PrepareAppdataDirByUserId(userId);
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
    auto ret = MountManager::GetInstance().PrepareAppdataDir(userId);
    EXPECT_EQ(ret, E_OK);

    std::vector<int32_t> userIds;
    MountManager::GetInstance().GetAllUserId(userIds);
    if (!userIds.empty()) {
        EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillRepeatedly(Return(false));
    }

    ret = MountManager::GetInstance().PrepareAppdataDir(0);
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
    auto ret = MountManager::GetInstance().DirExist(path);
    EXPECT_FALSE(ret);

    path = "/data/test";
    ret = MountManager::GetInstance().DirExist(path);
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
    auto ret = MountManager::GetInstance().CheckPathValid(bundleNameStr, userId);
    EXPECT_FALSE(ret);

    string basePath =
        SANDBOX_ROOT_PATH + to_string(userId);
    string completePath = basePath  + "/" + bundleNameStr + EL2_BASE;
    ForceCreateDirectory(completePath);
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().CheckPathValid(bundleNameStr, userId);
    EXPECT_TRUE(ret);

    string subDir = completePath + "test";
    ForceCreateDirectory(subDir);
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().CheckPathValid(bundleNameStr, userId);
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
    auto ret = MountManager::GetInstance().CheckMaps("", mountFailList);
    EXPECT_FALSE(ret);

    string baseDir = "/data/test/tdd";
    ForceCreateDirectory(baseDir);
    path = baseDir + "test.txt";
    ret = MountManager::GetInstance().CheckMaps(path, mountFailList);
    EXPECT_FALSE(ret);

    auto fd = open(path.c_str(), O_RDWR | O_CREAT);
    ASSERT_GT(fd, 0);

    std::string content = "this is a test\n/data/test/tdd";
    (void)write(fd, content.c_str(), content.size());
    close(fd);
    ret = MountManager::GetInstance().CheckMaps(path, mountFailList);
    EXPECT_FALSE(ret);

    mountFailList.push_back(baseDir);
    ret = MountManager::GetInstance().CheckMaps(path, mountFailList);
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
    MountManager::GetInstance().MountSandboxPath(srcPathEmpty, dstPathEmpty, bundleName, userId);

    std::vector<std::string> srcPaths {
        "/data/test/tdd1",
        "/data/test/tdd2"
    };
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPathEmpty, bundleName, userId);
    
    std::vector<std::string> dstPaths {
        "/data/test/tdd3"
    };
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
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
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(false));
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
    
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
    MountManager::GetInstance().MountSandboxPath(srcPaths, dstPaths, bundleName, userId);
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
    EXPECT_EQ(MountManager::GetInstance().UMountByListWithDetach(list), E_OK);
    
    list.push_back("test");
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(1));
    auto ret = MountManager::GetInstance().UMountByListWithDetach(list);
    if (errno != ENOENT && errno != EINVAL) {
        EXPECT_EQ(ret, errno);
    } else {
        EXPECT_EQ(ret, E_OK);
    }
    
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().UMountByListWithDetach(list);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_UMountByListWithDetach_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountFileMgrFuse_001
 * @tc.desc: Verify the MountFileMgrFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_MountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountFileMgrFuse_001 start";

    int32_t userId = 100;
    std::string path = "/mnt/data/100/smb/testMountFileMgrFuse";
    ForceCreateDirectory(path);
    int32_t fuseFd = -1;
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    int32_t ret = MountManager::GetInstance().MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_MOUNT_FILE_MGR_FUSE);

    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().MountFileMgrFuse(userId, path, fuseFd);
    EXPECT_EQ(ret, E_OK);
    ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountFileMgrFuse_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_UMountFileMgrFuse_001
 * @tc.desc: Verify the UMountFileMgrFuse function.
 * @tc.type: FUNC
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_UMountFileMgrFuse_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountFileMgrFuse_001 start";

    int32_t userId = 100;
    std::string path = "/mnt/data/100/smb/testUMountFileMgrFuse";
    ForceCreateDirectory(path);
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(1));
    int32_t ret = MountManager::GetInstance().UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().UMountFileMgrFuse(userId, path);
    EXPECT_EQ(ret, E_OK);
    ForceRemoveDirectory(path);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountFileMgrFuse_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_IsFileOccupied_001
 * @tc.desc: Verify the IsFileOccupied function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_IsFileOccupied_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_IsFileOccupied_001 start";
    std::string path;
    std::vector<std::string> input;
    std::vector<std::string> output;
    bool isOccupy;
    int32_t ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    path = "/data/test/tdd/";
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_OK);

    path = "/data/test/tdd/test.txt";
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_OK);

    path = "/storage/Users/currentUser/";
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_OK);

    path = "/data/test/tdd/";
    input = {"aa", "bb", "1.txt"};
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_OK);

    path = "/data/test/tdd/test.txt";
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    path = "/storage/Users/currentUser/";
    input = {"aa", "bb", "1.txt"};
    ret = MountManager::GetInstance().IsFileOccupied(path, input, output, isOccupy);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_IsFileOccupied_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_OpenProcForPath_001
 * @tc.desc: Verify the OpenProcForPath function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_OpenProcForPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_OpenProcForPath_001 start";
    std::string path = "/data/test/tdd/";
    bool isOccupy = false;
    bool isDir = true;
    int32_t ret = MountManager::GetInstance().OpenProcForPath(path, isOccupy, isDir);
    EXPECT_EQ(ret, E_OK);

    path = "/data/test/tdd/test.txt";
    isDir = false;
    ret = MountManager::GetInstance().OpenProcForPath(path, isOccupy, isDir);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_OpenProcForPath_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_OpenProcForMulti_001
 * @tc.desc: Verify the OpenProcForMulti function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_OpenProcForMulti_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_OpenProcForMulti_001 start";
    std::string path = "/data/test/tdd/";
    std::set<std::string> occupyFiles;
    int32_t ret = MountManager::GetInstance().OpenProcForMulti(path, occupyFiles);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_OpenProcForMulti_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_FindProcForPath_001
 * @tc.desc: Verify the FindProcForPath function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_FindProcForPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_FindProcForPath_001 start";
    std::string pidPath = "/test/111";
    std::string path = "/data/test/tdd/";
    bool isDir = false;
    bool ret = MountManager::GetInstance().FindProcForPath(pidPath, path, isDir);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_FindProcForPath_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerTest_FindProcForMulti_001
 * @tc.desc: Verify the FindProcForMulti function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerTest_FindProcForMulti_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_FindProcForMulti_001 start";
    std::string pidPath = "/test/111";
    std::string path = "/data/test/tdd/";
    std::set<std::string> occupyFiles;
    MountManager::GetInstance().FindProcForMulti(pidPath, path, occupyFiles);
    ASSERT_TRUE(occupyFiles.empty());
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerTest_FindProcForMulti_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountDfsDocs_001
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_MountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_002 start";

    int32_t userId = 100;
    std::string relativePath = "";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(false));
    int32_t ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_PREPARE_DIR);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    std::string tempPath(PATH_MAX + 1, 'a');
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    ret = MountManager::GetInstance().MountDfsDocs(userId, tempPath, deviceId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    relativePath = "@";
    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_001 end";
}


/**
 * @tc.name: Storage_Manager_MountManagerTest_MountDfsDocs_002
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_MountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_002 start";

    int32_t userId = 100;
    std::string relativePath = "/data";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EPERM;
    int32_t ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EBUSY;
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('0'), Return(0)));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EEXIST;
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('0'), Return(0)));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('0'), Return(0)));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_002 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountDfsDocs_003
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_MountDfsDocs_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_003 start";

    int32_t userId = 100;
    std::string relativePath = "/data";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EBUSY;
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('1'), Return(1)));
    int32_t ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EEXIST;
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('1'), Return(1)));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    EXPECT_CALL(*libraryFuncMock_, mount(_, _, _, _, _)).WillOnce(Return(0));
    errno = EEXIST;
    EXPECT_CALL(*paramMoc_, GetParameter(_, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>('1'), Return(1)));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_003 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_MountDfsDocs_004
 * @tc.desc: Verify the MountDfsDocs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_MountDfsDocs_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_004 start";

    int32_t userId = 100;
    std::string relativePath = "/data";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EPERM;
    int32_t ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_USER_MOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EBUSY;
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    errno = EEXIST;
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    errno = 0;
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*fileUtilMoc_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().MountDfsDocs(userId, relativePath, deviceId, deviceId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_MountDfsDocs_004 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_MountDisShareFile_001
 * @tc.desc: Verify the MountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_MountDisShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountDisShareFile_001 start";
    int32_t userId = 100;
    std::map<std::string, std::string> shareFiles = {{"/data/sharefile1", "/data/sharefile2"}};

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(false));
    auto ret = MountManager::GetInstance().MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_NON_EXIST);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true));
    ret = MountManager::GetInstance().MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_MOUNT_SHARE_FILE);
 
    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(1));
    ret = MountManager::GetInstance().MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_MOUNT_SHARE_FILE);

    EXPECT_CALL(*fileUtilMoc_, IsDir(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMoc_, Mount(_, _, _, _, _)).WillOnce(Return(0));
    ret = MountManager::GetInstance().MountDisShareFile(userId, shareFiles);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountDisShareFile_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_UMountDisShareFile_001
 * @tc.desc: Verify the UMountDisShareFile function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_UMountDisShareFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_UMountDisShareFile_001 start";
    int32_t userId = 100;
    std::string networkId = "/storage/cloud/100/files/Docs";
    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(1));
    errno = 5;
    auto ret = MountManager::GetInstance().UMountDisShareFile(userId, networkId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_UMountDisShareFile_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_UMountDfsDocs_001
 * @tc.desc: Verify the UMountDfsDocs function with invalid relativePath.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_UMountDfsDocs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountDfsDocs_001 start";

    int32_t userId = 100;
    std::string relativePath = "";
    std::string networkId = "test_network";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";

    int32_t ret = MountManager::GetInstance().UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    std::string longPath(PATH_MAX + 1, 'a');
    ret = MountManager::GetInstance().UMountDfsDocs(userId, longPath, networkId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    std::string invalidPath = "test@path";
    ret = MountManager::GetInstance().UMountDfsDocs(userId, invalidPath, networkId, deviceId);
    EXPECT_EQ(ret, E_PARAMS_INVALID);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountDfsDocs_001 end";
}

/**
 * @tc.name: Storage_Manager_MountManagerTest_UMountDfsDocs_002
 * @tc.desc: Verify the UMountDfsDocs function when UMount2 fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MountManagerTest, Storage_Manager_MountManagerTest_UMountDfsDocs_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountDfsDocs_002 start";

    int32_t userId = 100;
    std::string relativePath = "valid_path";
    std::string networkId = "test_network";
    std::string deviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    std::string dstPath = StringPrintf("/mnt/data/%d/hmdfs/%s", userId, deviceId.c_str());

    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(1));
    int32_t ret = MountManager::GetInstance().UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_USER_UMOUNT_ERR);

    EXPECT_CALL(*fileUtilMoc_, UMount2(_, _)).WillOnce(Return(2));
    ret = MountManager::GetInstance().UMountDfsDocs(userId, relativePath, networkId, deviceId);
    EXPECT_EQ(ret, E_USER_UMOUNT_ERR);

    GTEST_LOG_(INFO) << "Storage_Manager_MountManagerTest_UMountDfsDocs_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_FindMountsByNetworkId_001
 * @tc.desc: Verify the FindMountsByNetworkId function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_FindMountsByNetworkId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_FindMountsByNetworkId_001 start";
    std::string networkId = "hmdfs";
    std::list<std::string> mounts;
    auto ret = MountManager::GetInstance().FindMountsByNetworkId(networkId, mounts);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_FindMountsByNetworkId_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_FilterNotMountedPath_001
 * @tc.desc: Verify the FilterNotMountedPath function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_FilterNotMountedPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_FilterNotMountedPath_001 start";
    std::map<std::string, std::string> notMountPaths = {{"/storage/media/100", "/data/service/el2/100/hmdfs/account"}};
    auto ret = MountManager::GetInstance().FilterNotMountedPath(notMountPaths);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_FilterNotMountedPath_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_MountCryptoPathAgain_001
 * @tc.desc: Verify the MountCryptoPathAgain function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_MountCryptoPathAgain_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountCryptoPathAgain_001 start";
    uint32_t userId = 101;
    auto ret = MountManager::GetInstance().MountCryptoPathAgain(userId);
    EXPECT_EQ(ret, -ENOENT);

    userId = 100;
    std::string path = "/data/virt_service/rgm_hmos/anco_hmos_data/media/0";
    ForceCreateDirectory(path);
    ret = MountManager::GetInstance().MountCryptoPathAgain(userId);
    ForceRemoveDirectory(path);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountCryptoPathAgain_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_MountPointToList_001
 * @tc.desc: Verify the MountPointToList function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_MountPointToList_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountPointToList_001 start";
    std::list<std::string> hmdfsList;
    std::list<std::string> hmdfsLists;
    std::list<std::string> sharefsList;
    std::string line;
    uint32_t userId = 100;
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);
    line = "sourcesourcesourcesourcesourcesourcesource destination hmdfs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);
    line = "s destination hmdfs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);

    line = "sourcesourcesourcesourcesourcesourcesource destination sharefs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);
    line = "s destination sharefs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);

    line = "source destination hmfs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);
    line = "source destinationdestinationdestinationdestinationdestination hmfs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);

    line = "source destination f2fs";
    MountManager::GetInstance().MountPointToList(hmdfsList, hmdfsLists, sharefsList, line, userId);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MountPointToList_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_CheckProcessUserId_001
 * @tc.desc: Verify the CheckProcessUserId function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_CheckProcessUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CheckProcessUserId_001 start";
    int32_t userId = 100;
    vector<ProcessInfo> proInfos = {{1234, "testproc1"}, {5678, "testproc2"}};
    vector<ProcessInfo> processKillInfos;
    auto ret = MountManager::GetInstance().CheckProcessUserId(userId, proInfos, processKillInfos);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CheckProcessUserId_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_CheckProcessUserId_002
 * @tc.desc: Verify the CheckProcessUserId function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_CheckProcessUserId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CheckProcessUserId_002 start";
    int32_t userId = 100;
    vector<ProcessInfo> proInfos = {{1, "testproc1"}};
    vector<ProcessInfo> processKillInfos;
    auto ret = MountManager::GetInstance().CheckProcessUserId(userId, proInfos, processKillInfos);
    EXPECT_EQ(ret, E_OK);

    userId = 0;
    ret = MountManager::GetInstance().CheckProcessUserId(userId, proInfos, processKillInfos);
    EXPECT_EQ(ret, E_OK);

    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CheckProcessUserId_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_CloudAndFuseDirFlag_001
 * @tc.desc: Verify the CloudAndFuseDirFlag function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_CloudAndFuseDirFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CloudAndFuseDirFlag_001 start";
    std::string path;
    bool ret = MountManager::GetInstance().CloudAndFuseDirFlag(path);
    EXPECT_TRUE(ret);
    
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_CloudAndFuseDirFlag_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_MediaFuseDirFlag_001
 * @tc.desc: Verify the MediaFuseDirFlag function.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_MediaFuseDirFlag_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MediaFuseDirFlag_001 start";
    std::string path;
    bool ret = MountManager::GetInstance().MediaFuseDirFlag(path);
    EXPECT_TRUE(ret);
    
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_MediaFuseDirFlag_001 end";
}


/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_001
 * @tc.desc: Verify the HandleDisDstPath function when dstPath not exists.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_001 start";
    std::string dstPath = "/data/test/non_exist_dir";
    
    // Mock filesystem not exists
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(true));
    
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_001 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_002
 * @tc.desc: Verify the HandleDisDstPath function when dstPath not exists and MkDirRecurse failed.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_002 start";
    std::string dstPath = "/data/test/non_exist_dir";
    
    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _)).WillOnce(Return(false));
    
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_ERR);
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_002 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_003
 * @tc.desc: Verify the HandleDisDstPath function when dstPath exists and is not empty.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_003 start";
    std::string dstPath = "/data/test/exist_non_empty_dir";
    std::string dstPathNotEmpty = "/data/test/exist_non_empty_dir/dir1";

    std::error_code ec;
    std::filesystem::create_directories(dstPathNotEmpty, ec);

    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_ERR);

    std::filesystem::remove_all(dstPath, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        GTEST_LOG_(WARNING) << "Cleanup failed: " << ec.message();
    }
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_003 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_004
 * @tc.desc: Verify the HandleDisDstPath function when dstPath exists, is empty
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_004 start";
    std::string dstPath = "/data/test/dir_without_remote_share";
    

    std::error_code ec;
    std::filesystem::create_directories(dstPath, ec);
    
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_ERR);

    std::filesystem::remove_all(dstPath, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        GTEST_LOG_(WARNING) << "Cleanup failed: " << ec.message();
    }
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_004 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_005
 * @tc.desc: Verify the HandleDisDstPath function when all conditions met and MkDirRecurse succeed after remove.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_005 start";
    std::string dstPath = "/data/test/.remote_share/sub_dir";
    
    std::error_code ec;
    std::filesystem::create_directories(dstPath, ec);

    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(false));
    
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_ERR);

    std::filesystem::remove_all(dstPath, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        GTEST_LOG_(WARNING) << "Cleanup failed: " << ec.message();
    }
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_005 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_006
 * @tc.desc: Verify the HandleDisDstPath function when MkDirRecurse failed after remove.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_006 start";
    std::string dstPath = "/data/test/.remote_share/sub_dir";

    std::error_code ec;
    std::filesystem::create_directories(dstPath, ec);

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _))
        .WillOnce(Return(false));

    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_ERR);

    std::filesystem::remove_all(dstPath, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        GTEST_LOG_(WARNING) << "Cleanup failed: " << ec.message();
    }
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_006 end";
}

/**
 * @tc.name: Storage_Daemon_MountManagerExtTest_HandleDisDstPath_007
 * @tc.desc: Verify the HandleDisDstPath function when MkDirRecurse failed after remove.
 * @tc.type: FUNC
 * @tc.require: IB49AM
 */
HWTEST_F(MountManagerTest, Storage_Daemon_MountManagerExtTest_HandleDisDstPath_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_007 start";
    std::string dstPath = "/data/test/.remote_share/sub_dir";

    std::error_code ec;
    std::filesystem::create_directories(dstPath, ec);

    EXPECT_CALL(*fileUtilMoc_, MkDirRecurse(_, _))
        .WillOnce(Return(true));

    EXPECT_CALL(*fileUtilMoc_, RmDirRecurse(_)).WillOnce(Return(true));
    auto ret = MountManager::GetInstance().HandleDisDstPath(dstPath);
    EXPECT_EQ(ret, E_OK);

    std::filesystem::remove_all(dstPath, ec);
    if (ec && ec != std::errc::no_such_file_or_directory) {
        GTEST_LOG_(WARNING) << "Cleanup failed: " << ec.message();
    }
    GTEST_LOG_(INFO) << "Storage_Daemon_MountManagerExtTest_HandleDisDstPath_007 end";
}
} // STORAGE_DAEMON
} // OHOS
