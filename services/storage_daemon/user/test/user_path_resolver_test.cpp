/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "user/user_path_resolver.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <cstdlib>
#include <gtest/gtest.h>

#include "storage_service_errno.h"
#include "storage_service_constant.h"
#include "user/mount_constant.h"
#include "file_utils_mock.h"

namespace OHOS {
namespace StorageDaemon {
namespace Test {
using namespace testing;
using namespace testing::ext;
constexpr const char *STORAGE_ETC_PATH = "/etc/storage_daemon/";
constexpr const char *STORAGE_USER_PATH = "storage_user_path.json";
constexpr const char *STORAGE_MOUNT_INFO = "storage_mount_info.json";

class UserPathResolverTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp();
    void TearDown();

    void CreateFile(const std::string &path, std::string data = "");
    void DeleteFile(const std::string &path);

    static const uint32_t userId_ = 116;
    std::shared_ptr<FileUtilMoc> fileUtilMock_ = nullptr;
};

void UserPathResolverTest::SetUp()
{
    fileUtilMock_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMock_;
}
void UserPathResolverTest::TearDown(void)
{
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMock_ = nullptr;
}

void UserPathResolverTest::CreateFile(const std::string &path, std::string data)
{
    std::error_code errCode;
    if (std::filesystem::exists(path, errCode)) {
        std::filesystem::rename(path, path + "_bak", errCode);
    }

    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return;
    }
    
    file << data;
    file.close();
}

void UserPathResolverTest::DeleteFile(const std::string &path)
{
    std::error_code errCode;
    std::filesystem::remove(path, errCode);
    if (std::filesystem::exists(path + "_bak", errCode)) {
        std::filesystem::rename(path + "_bak", path, errCode);
    }
}

/**
 * @tc.name: UserPathResolverTest_DirInfo_FromJson_001
 * @tc.desc: Verify the DirInfo from_json.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_DirInfo_FromJson_001, TestSize.Level1)
{
    std::map<std::string, DirInfo> testMap {
        {
            R"({"path": "/home/user", "uid": 1000, "gid": 1000, "mode": "0755", "options": "key=value"})",
            DirInfo{"/home/user", 0755, 1000, 1000, {{"key", "value"}}}
        },
        {
            R"({"path": "/home/user"})",
            DirInfo{"/home/user", 0711, 0, 0, {}}
        },
        {
            R"({"path": "", "uid": 0, "gid": 0, "mode": "", "options": ""})",
            DirInfo{"", 0711, 0, 0, {}}
        },
        {
            R"({"mode": 755})",
            DirInfo{"", 0711, 0, 0, {}}
        },
        {
            R"({})",
            DirInfo{"", 0711, 0, 0, {}}
        },
        {
            R"({"path": 12345, "uid": "1000", "gid": "1000", "mode": 755, "options": 123})",
            DirInfo{"", 0711, 0, 0, {}}
        }
    };
    for (auto &test : testMap) {
        GTEST_LOG_(INFO) << test.first << "\n";
        nlohmann::json j = nlohmann::json::parse(test.first);
        DirInfo dirInfo;
        from_json(j, dirInfo);
        EXPECT_EQ(dirInfo.path, test.second.path);
        EXPECT_EQ(dirInfo.mode, test.second.mode);
        EXPECT_EQ(dirInfo.uid, test.second.uid);
        EXPECT_EQ(dirInfo.gid, test.second.gid);
        EXPECT_EQ(dirInfo.options, test.second.options);
    }
}

/**
 * @tc.name: UserPathResolverTest_MountNodeInfo_FromJson_001
 * @tc.desc: Verify the MountNodeInfo from_json.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountNodeInfo_FromJson_001, TestSize.Level1)
{
    std::map<std::string, MountNodeInfo> testMap {
        {
            R"({"src-path": "/src", "dest-path": "/dst", "file-system-type": "ext4", "data": "data",
                "mount-flags": ["rec"], "create-dest-path": true, "dest-path-info": {}, "options": "key1=value1"})",
            MountNodeInfo{"/src", "/dst", "ext4", MS_REC, "data", true,
                std::make_shared<DirInfo>(), {{"key1", "value1"}}}
        },
        {
            R"({"src-path": "", "dest-path": "", "file-system-type": "", "data": "", "mount-flags": [],
                "create-dest-path": false, "dest-path-info": {}, "options": ""})",
            MountNodeInfo{"", "", "", 0, "", false, nullptr, {}}
        },
        {
            R"({"src-path": 123, "dest-path": 123, "file-system-type": 123, "data": 123, "mount-flags": {},
                "create-dest-path": "false", "dest-path-info": 123, "options": 123})",
            MountNodeInfo{"", "", "", 0, "", false, nullptr, {}}
        },
        {
            R"({})",
            MountNodeInfo{"", "", "", 0, "", false, nullptr, {}}
        },
    };
    for (auto &test : testMap) {
        nlohmann::json j = nlohmann::json::parse(test.first);
        MountNodeInfo nodeInfo;
        from_json(j, nodeInfo);
        EXPECT_EQ(nodeInfo.srcPath, test.second.srcPath);
        EXPECT_EQ(nodeInfo.dstPath, test.second.dstPath);
        EXPECT_EQ(nodeInfo.fsType, test.second.fsType);
        EXPECT_EQ(nodeInfo.mountFlags, test.second.mountFlags);
        EXPECT_EQ(nodeInfo.data, test.second.data);
        EXPECT_EQ(nodeInfo.options, test.second.options);
    }
}

/**
 * @tc.name: UserPathResolverTest_MakeDir_001
 * @tc.desc: Verify the MakeDir.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MakeDir_001, TestSize.Level1)
{
    DirInfo dirInfo {
        .path = "test",
        .uid = 0,
    };
    EXPECT_CALL(*fileUtilMock_, PrepareDir(_, _, _, _)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_EQ(dirInfo.MakeDir(), E_OK);
    EXPECT_EQ(dirInfo.MakeDir(), E_PREPARE_DIR);
}

/**
 * @tc.name: UserPathResolverTest_RemoveDir_001
 * @tc.desc: Verify the RemoveDir.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_RemoveDir_001, TestSize.Level1)
{
    DirInfo dirInfo {
        .path = "test",
        .uid = 0,
    };
    EXPECT_CALL(*fileUtilMock_, RmDirRecurse(_)).WillOnce(Return(true)).WillOnce(Return(false));
    EXPECT_EQ(dirInfo.RemoveDir(), E_OK);
    EXPECT_EQ(dirInfo.RemoveDir(), E_DESTROY_DIR);
}

/**
 * @tc.name: UserPathResolverTest_UpdateDirUid_001
 * @tc.desc: Verify the UpdateDirUid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_UpdateDirUid_001, TestSize.Level1)
{
    DirInfo dirInfo {
        .path = "test",
        .uid = 0,
        .options = {{"update_uid", ""}},
    };
    auto tempUid = dirInfo.uid;
    dirInfo.UpdateDirUid(userId_);
    EXPECT_EQ(dirInfo.uid, USER_ID_BASE * userId_ + tempUid);

    dirInfo.options.clear();
    tempUid = dirInfo.uid;
    dirInfo.UpdateDirUid(userId_);
    EXPECT_EQ(dirInfo.uid, tempUid);
}

/**
 * @tc.name: UserPathResolverTest_UpdateDirUid_002
 * @tc.desc: Verify the UpdateDirUid.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_UpdateDirUid_002, TestSize.Level1)
{
    DirInfo dirInfo1 {};
    dirInfo1.uid = 100;
    std::string str = "update_uid";
    dirInfo1.options.emplace(str, "");
    const auto oldUid1 = dirInfo1.uid;
    const int32_t userId1 = 10;
    dirInfo1.UpdateDirUid(userId1);
    EXPECT_EQ(dirInfo1.uid, USER_ID_BASE * static_cast<uid_t>(userId1) + oldUid1);
    DirInfo dirInfo2 {};
    dirInfo2.uid = 100;
    dirInfo2.options.clear();
    const auto oldUid2 = dirInfo2.uid;
    dirInfo2.UpdateDirUid(10);
    EXPECT_EQ(dirInfo2.uid, oldUid2);
    DirInfo dirInfo3 {};
    dirInfo3.uid = 100;
    dirInfo3.options.emplace(str, "");
    const auto oldUid3 = dirInfo3.uid;
    dirInfo3.UpdateDirUid(-1);
    EXPECT_EQ(dirInfo3.uid, oldUid3);
    DirInfo dirInfo4 {};
    dirInfo4.uid = 123;
    dirInfo4.options.emplace(str, "");
    const auto oldUid4 = dirInfo4.uid;
    dirInfo4.UpdateDirUid(0);
    EXPECT_EQ(dirInfo4.uid, USER_ID_BASE * static_cast<uid_t>(0) + oldUid4);
    EXPECT_EQ(dirInfo4.uid, oldUid4);
}

/**
 * @tc.name: UserPathResolverTest_MountDir_001
 * @tc.desc: Verify the MountDir when srcPath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountDir_001, TestSize.Level1)
{
    MountNodeInfo nodeInfo {
        .srcPath = "test",
        .dstPath = "test",
        .createDstPath = false,
    };
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillRepeatedly(Return(false));
    EXPECT_EQ(nodeInfo.MountDir(), E_NON_EXIST);

    nodeInfo.createDstPath = true;
    EXPECT_EQ(nodeInfo.MountDir(), E_NON_EXIST);

    nodeInfo.dstPathInfo = std::make_unique<DirInfo>();
    EXPECT_CALL(*fileUtilMock_, PrepareDir(_, _, _, _)).WillOnce(Return(true));
    EXPECT_EQ(nodeInfo.MountDir(), E_NON_EXIST);
}

/**
 * @tc.name: UserPathResolverTest_MountDir_002
 * @tc.desc: Verify the MountDir when dstPath not exist.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountDir_002, TestSize.Level1)
{
    MountNodeInfo nodeInfo {
        .srcPath = "test",
        .createDstPath = false,
    };
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillOnce(Return(true));
    EXPECT_EQ(nodeInfo.MountDir(), E_NON_EXIST);

    nodeInfo.dstPath = "";
    nodeInfo.dstPath = "test";
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillOnce(Return(false));
    EXPECT_EQ(nodeInfo.MountDir(), E_NON_EXIST);
}

/**
 * @tc.name: UserPathResolverTest_MountDir_003
 * @tc.desc: Verify the MountDir when need check mounted.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountDir_003, TestSize.Level1)
{
    MountNodeInfo nodeInfo {
        .srcPath = "",
        .dstPath = "test",
        .createDstPath = false,
        .options = {{"check_mounted", ""}},
    };
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMock_, IsPathMounted(_)).WillOnce(Return(true));
    EXPECT_EQ(nodeInfo.MountDir(), E_OK);

    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMock_, IsPathMounted(_)).WillOnce(Return(false));
    EXPECT_CALL(*fileUtilMock_, Mount(_, _, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(nodeInfo.MountDir(), E_OK);
}

/**
 * @tc.name: UserPathResolverTest_MountDir_004
 * @tc.desc: Verify the MountDir when mount failed.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountDir_004, TestSize.Level1)
{
    MountNodeInfo nodeInfo {
        .srcPath = "",
        .dstPath = "test",
        .createDstPath = false,
    };
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillOnce(Return(true));
    EXPECT_CALL(*fileUtilMock_, Mount(_, _, _, _, _)).WillOnce(Return(E_ERR));
    errno = 0;
    EXPECT_EQ(nodeInfo.MountDir(), E_ERR);
}

/**
 * @tc.name: UserPathResolverTest_MountDir_005
 * @tc.desc: Verify the MountDir when mount success.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_MountDir_005, TestSize.Level1)
{
    MountNodeInfo nodeInfo {
        .srcPath = "",
        .dstPath = "test",
        .createDstPath = false,
    };
    EXPECT_CALL(*fileUtilMock_, IsDir(_)).WillRepeatedly(Return(true));

    EXPECT_CALL(*fileUtilMock_, Mount(_, _, _, _, _)).WillOnce(Return(E_OK));
    errno = 0;
    EXPECT_EQ(nodeInfo.MountDir(), E_OK);

    EXPECT_CALL(*fileUtilMock_, Mount(_, _, _, _, _)).WillOnce(Return(E_ERR));
    errno = EEXIST;
    EXPECT_EQ(nodeInfo.MountDir(), E_OK);

    EXPECT_CALL(*fileUtilMock_, Mount(_, _, _, _, _)).WillOnce(Return(E_ERR));
    errno = EBUSY;
    EXPECT_EQ(nodeInfo.MountDir(), E_OK);
}

/**
 * @tc.name: UserPathResolverTest_GetUserBasePath_001
 * @tc.desc: Verify the GetUserBasePath.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_GetUserBasePath_001, TestSize.Level1)
{
    std::vector<DirInfo> dirInfoList;
    uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1;
    EXPECT_EQ(UserPathResolver::GetUserBasePath(userId_, flags, dirInfoList), E_OK);

    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
    EXPECT_EQ(UserPathResolver::GetUserBasePath(userId_, flags, dirInfoList), E_OPEN_JSON_FILE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
}

/**
 * @tc.name: UserPathResolverTest_GetUserServicePath_001
 * @tc.desc: Verify the GetUserServicePath.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_GetUserServicePath_001, TestSize.Level1)
{
    std::vector<DirInfo> dirInfoList;
    uint32_t flags = IStorageDaemonEnum::CRYPTO_FLAG_EL1;
    EXPECT_EQ(UserPathResolver::GetUserServicePath(userId_, flags, dirInfoList), E_OK);

    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
    EXPECT_EQ(UserPathResolver::GetUserServicePath(userId_, flags, dirInfoList), E_OPEN_JSON_FILE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
}

/**
 * @tc.name: UserPathResolverTest_GetAppdataPath_001
 * @tc.desc: Verify the GetAppdataPath.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_GetAppdataPath_001, TestSize.Level1)
{
    std::vector<DirInfo> dirInfoList;
    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
    EXPECT_EQ(UserPathResolver::GetAppdataPath(userId_, dirInfoList), E_OPEN_JSON_FILE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);


    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH, "{[]}");
    EXPECT_EQ(UserPathResolver::GetAppdataPath(userId_, dirInfoList), E_JSON_PARSE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);

    EXPECT_EQ(UserPathResolver::GetAppdataPath(userId_, dirInfoList), E_OK);
}

/**
 * @tc.name: UserPathResolverTest_GetVirtualPath_001
 * @tc.desc: Verify the GetVirtualPath.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_GetVirtualPath_001, TestSize.Level1)
{
    std::vector<DirInfo> dirInfoList;
    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);
    EXPECT_EQ(UserPathResolver::GetVirtualPath(userId_, dirInfoList), E_OPEN_JSON_FILE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);

    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH, "{[]}");
    EXPECT_EQ(UserPathResolver::GetVirtualPath(userId_, dirInfoList), E_JSON_PARSE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_USER_PATH);

    EXPECT_EQ(UserPathResolver::GetVirtualPath(userId_, dirInfoList), E_OK);
}

/**
 * @tc.name: UserPathResolverTest_GetAppDataMountNodeList_001
 * @tc.desc: Verify the GetAppDataMountNodeList.
 * @tc.type: FUNC
 * @tc.require: AR000H09L6
 */
HWTEST_F(UserPathResolverTest, UserPathResolverTest_GetAppDataMountNodeList_001, TestSize.Level1)
{
    std::vector<MountNodeInfo> dirInfoList;
    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_MOUNT_INFO);
    EXPECT_EQ(UserPathResolver::GetAppDataMountNodeList(userId_, dirInfoList), E_OPEN_JSON_FILE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_MOUNT_INFO);

    CreateFile(string(STORAGE_ETC_PATH) + STORAGE_MOUNT_INFO, "{[]}");
    EXPECT_EQ(UserPathResolver::GetAppDataMountNodeList(userId_, dirInfoList), E_JSON_PARSE_ERROR);
    DeleteFile(string(STORAGE_ETC_PATH) + STORAGE_MOUNT_INFO);

    EXPECT_EQ(UserPathResolver::GetAppDataMountNodeList(userId_, dirInfoList), E_OK);
}

} // Test
} // STORAGE_DAEMON
} // OHOS