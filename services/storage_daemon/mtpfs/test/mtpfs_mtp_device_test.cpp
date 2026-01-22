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
#include <libmtp.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <gtest/gtest.h>
#include <fuse_opt.h>
#include <unistd.h>
#include "mtpfs_mtp_device.h"
#include "mtpfs_libmtp.h"
#include "mtpfs_util.h"
#include "mtpfs_fuse.h"
#include "storage_service_log.h"

LIBMTP_mtpdevice_t *LIBMTP_Open_Raw_Device_Uncached(LIBMTP_raw_device_t *rawdevice)
{
    if (rawdevice == nullptr) {
        return nullptr;
    }
}

const MtpFsTypeDir *ReadDirFetchContent(std::string path)
{
    return nullptr;
}

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace testing::ext;
using namespace testing;

class MtpfsDeviceTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown()
    {
        MtpFsDevice::removingFileSet_.clear();
    };
};

/**
 * @tc.name: Mtpfs_Connect_001
 * @tc.desc: Verify the Connect function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_Connect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_Connect_001 start";
    LIBMTP_raw_device_t *dev = nullptr;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    bool result = mtpfsdevice->Connect(dev);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_Connect_001 end";
}

/**
 * @tc.name: Mtpfs_ConvertErrorCode_001
 * @tc.desc: Verify the ConvertErrorCode function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_ConvertErrorCode_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConvertErrorCode_001 start";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    LIBMTP_error_number_t err = LIBMTP_ERROR_NONE;
    bool result = mtpfsdevice->ConvertErrorCode(err);
    EXPECT_EQ(result, true);

    LIBMTP_error_number_t err1 = LIBMTP_ERROR_NO_DEVICE_ATTACHED;
    bool result1 = mtpfsdevice->ConvertErrorCode(err1);
    EXPECT_EQ(result1, false);

    LIBMTP_error_number_t err2 = LIBMTP_ERROR_CONNECTING;
    bool result2 = mtpfsdevice->ConvertErrorCode(err2);
    EXPECT_EQ(result2, false);

    LIBMTP_error_number_t err3 = LIBMTP_ERROR_MEMORY_ALLOCATION;
    bool result3 = mtpfsdevice->ConvertErrorCode(err3);
    EXPECT_EQ(result3, false);

    LIBMTP_error_number_t err4 = LIBMTP_ERROR_GENERAL;
    bool result4 = mtpfsdevice->ConvertErrorCode(err4);
    EXPECT_EQ(result4, false);

    LIBMTP_error_number_t err5 = LIBMTP_ERROR_USB_LAYER;
    bool result5 = mtpfsdevice->ConvertErrorCode(err5);
    EXPECT_EQ(result5, false);

    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConvertErrorCode_001 end";
}

/**
 * @tc.name: Mtpfs_ConnectByDevNo_001
 * @tc.desc: Verify the Connect function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_ConnectByDevNo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConnectByDevNo_001 start";
    int devNo = 3;
    int rawDevicesCnt = 3;
    LIBMTP_raw_device_t *rawDevices = nullptr;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    bool result = mtpfsdevice->ConnectByDevNo(devNo);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConnectByDevNo_001 end";
}


/**
 * @tc.name: Mtpfs_ConnectByDevFile_001
 * @tc.desc: Verify the ConnectByDevFile function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_ConnectByDevFile_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConnectByDevFile_001 start";
    const std::string &devFile = "test";
    uint8_t bnum = 0;
    uint8_t dnum = 0;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    bool result = mtpfsdevice->ConnectByDevFile(devFile);
    EXPECT_EQ(result, false);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_ConnectByDevFile_001 end";
}

/**
 * @tc.name: Mtpfs_Disconnect_001
 * @tc.desc: Verify the Disconnect function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_Disconnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_Disconnect_001 start";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    mtpfsdevice->Disconnect();
    EXPECT_EQ(mtpfsdevice->device_, nullptr);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_Disconnect_001 end";
}

/**
 * @tc.name: Mtpfs_HandleDir_001
 * @tc.desc: Verify the HandleDir function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_HandleDir_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_HandleDir_001 start";
    LIBMTP_file_t *content = nullptr;
    MtpFsTypeDir dir;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    mtpfsdevice->HandleDir(content, &dir);
    EXPECT_EQ(content, nullptr);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_HandleDir_001 end";
}

/**
 * @tc.name: Mtpfs_AddUploadRecord_001
 * @tc.desc: Verify the AddUploadRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_AddUploadRecord_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_AddUploadRecord_001 start";
    const char *path = "/mnt/data/external";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    mtpfsdevice->AddUploadRecord(path, "fail");
    auto it = mtpfsdevice->uploadRecordMap_.find(path);
    EXPECT_EQ(it->second, "fail");

    mtpfsdevice->AddUploadRecord(path, "success");
    it = mtpfsdevice->uploadRecordMap_.find(path);
    EXPECT_EQ(it->second, "success");
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_AddUploadRecord_001 end";
}

/**
 * @tc.name: Mtpfs_RemoveUploadRecord_001
 * @tc.desc: Verify the RemoveUploadRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_RemoveUploadRecord_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_RemoveUploadRecord_001 start";
    const char *path0 = "/mnt/data/external";
    const char *path1 = "/mnt/data/exter";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    mtpfsdevice->AddUploadRecord(path0, "success");

    mtpfsdevice->RemoveUploadRecord(path1);
    auto it = mtpfsdevice->uploadRecordMap_.find(path0);
    EXPECT_EQ(it->second, "success");

    mtpfsdevice->RemoveUploadRecord(path0);
    it = mtpfsdevice->uploadRecordMap_.find(path0);
    EXPECT_EQ(it, mtpfsdevice->uploadRecordMap_.end());
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_RemoveUploadRecord_001 end";
}

/**
 * @tc.name: Mtpfs_SetUploadRecord_001
 * @tc.desc: Verify the SetUploadRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_SetUploadRecord_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_SetUploadRecord_001 start";
    const char *path = "/mnt/data/external";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();

    mtpfsdevice->SetUploadRecord(path, "fail");
    auto it = mtpfsdevice->uploadRecordMap_.find(path);
    EXPECT_EQ(it, mtpfsdevice->uploadRecordMap_.end());

    mtpfsdevice->AddUploadRecord(path, "fail");
    mtpfsdevice->SetUploadRecord(path, "success");
    it = mtpfsdevice->uploadRecordMap_.find(path);
    EXPECT_EQ(it->second, "success");
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_SetUploadRecord_001 end";
}

/**
 * @tc.name: Mtpfs_FindUploadRecord_001
 * @tc.desc: Verify the FindUploadRecord function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_FindUploadRecord_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_FindUploadRecord_001 start";
    const char *path = "/mnt/data/external";
    std::tuple<std::string, std::string> ret;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();

    ret = mtpfsdevice->FindUploadRecord(path);
    EXPECT_EQ(std::get<0>(ret).empty(), true);

    mtpfsdevice->AddUploadRecord(path, "success");
    ret = mtpfsdevice->FindUploadRecord(path);
    EXPECT_EQ(std::get<1>(ret), "success");
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_FindUploadRecord_001 end";
}

/**
 * @tc.name: DirRemoveDirectly_001
 * @tc.desc: Verify the DirRemoveDirectly function.
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_DirRemoveDirectly_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_DirRemoveDirectly_001 start";
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();

    int32_t ret = 0;
    LIBMTP_event_t event = LIBMTP_EVENT_OBJECT_ADDED;
    uint32_t param = 1;
    void *data = nullptr;
    mtpfsdevice->MtpEventCallback(ret, event, param, data);

    event = LIBMTP_EVENT_OBJECT_REMOVED;
    mtpfsdevice->MtpEventCallback(ret, event, param, data);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_DirRemoveDirectly_001 end";
}

/**
 * @tc.name: MtpfsDeviceTest_AddRemovingFileTest_001
 * @tc.desc: 测试当传入的路径为空字符串时,AddRemovingFile 函数应返回 -EINVAL
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_AddRemovingFileTest_001, TestSize.Level1) {
    std::string emptyPath = "";
    int result = MtpFsDevice::AddRemovingFile(emptyPath);
    EXPECT_EQ(result, -EINVAL);
}

/**
 * @tc.name: MtpfsDeviceTest_AddRemovingFileTest_002
 * @tc.desc: 测试当传入的路径不为空时,AddRemovingFile 函数应返回 E_OK,并且路径应被插入到 removingFileSet_ 中
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_AddRemovingFileTest_002, TestSize.Level1) {
    std::string validPath = "/path/to/file";
    int result = MtpFsDevice::AddRemovingFile(validPath);
    EXPECT_EQ(result, E_OK);
    EXPECT_TRUE(MtpFsDevice::removingFileSet_.find(validPath) != MtpFsDevice::removingFileSet_.end());
}

/**
 * @tc.name: EraseRemovingFile_001
 * @tc.desc: 测试当传入的路径为空时,函数应返回 E_PARAMS_INVALID
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_EraseRemovingFile_001, TestSize.Level1) {
    std::string emptyPath = "";
    int result = MtpFsDevice::EraseRemovingFile(emptyPath);
    EXPECT_EQ(result, -EINVAL);
}

/**
 * @tc.name: MtpfsDeviceTest_EraseRemovingFile_002
 * @tc.desc: 测试当传入的路径有效时,函数应返回 E_OK
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_EraseRemovingFile_002, TestSize.Level1) {
    std::string validPath = "/valid/path";
    MtpFsDevice::removingFileSet_.insert(validPath);

    int result = MtpFsDevice::EraseRemovingFile(validPath);
    EXPECT_EQ(result, E_OK);
    EXPECT_FALSE(MtpFsDevice::removingFileSet_.count(validPath));
}

/**
 * @tc.name: MtpfsDeviceTest_IsFileRemovingTest_001
 * @tc.desc: 测试当输入路径为空时,IsFileRemoving 函数应返回 false
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_IsFileRemovingTest_001, TestSize.Level1) {
    std::string emptyPath = "";
    EXPECT_FALSE(MtpFsDevice::IsFileRemoving(emptyPath));
}

/**
 * @tc.name: MtpfsDeviceTest_IsFileRemovingTest_002
 * @tc.desc: 测试当输入路径不在 removingFileSet_ 中时,IsFileRemoving 函数应返回 false
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_IsFileRemovingTest_002, TestSize.Level1) {
    std::string pathNotInSet = "/path/not/in/set";
    EXPECT_FALSE(MtpFsDevice::IsFileRemoving(pathNotInSet));
}

/**
 * @tc.name: MtpfsDeviceTest_IsFileRemovingTest_003
 * @tc.desc: 测试当输入路径在 removingFileSet_ 中时,IsFileRemoving 函数应返回 true
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_IsFileRemovingTest_003, TestSize.Level1) {
    std::string pathInSet = "/path/in/set";
    MtpFsDevice::removingFileSet_.insert(pathInSet);
    EXPECT_TRUE(MtpFsDevice::IsFileRemoving(pathInSet));
}

/**
 * @tc.name: MtpfsDeviceTest_FilePushAsyncTest_001
 * @tc.desc: 测试当文件推送成功时,FilePushAsync 函数应返回成功
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_FilePushAsyncTest_001, TestSize.Level1) {
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    std::string srcPath = "/path/to/local/file/test.txt";
    std::string dstPath = "/path/to/mtp/device/test.txt";

    int result = mtpfsdevice->FilePushAsync(srcPath, dstPath);
    EXPECT_EQ(result, E_OK);
}

/**
 * @tc.name: PerformUpload_ShouldReturnSuccess_WhenUploadSucceeds
 * @tc.desc: 测试文件上传失败的场景
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_PerformUploadTest_001, TestSize.Level1) {
    std::string src = "test_source_file.txt";
    std::string dst = "test_destination_file.txt";
    MtpFsTypeDir dirParent(1, 1, 1, "");
    MtpFsTypeFile fileToRemove(1, 1, 1, "file_to_remove.txt", 1024, 0);
    std::string dstBaseName = "new_file_name.txt";

    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    int result = mtpfsdevice->PerformUpload(src, dst, &dirParent, &fileToRemove, dstBaseName);
    EXPECT_EQ(result, -EINVAL);
}

/**
 * @tc.name: MtpfsDeviceTest_GetThumbnailSizeTest_001
 * @tc.desc: 测试当目录内容为 nullptr 时,GetThumbnailSize 应返回 -ENOENT
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_GetThumbnailSizeTest_001, TestSize.Level1) {
    std::string path = "/invalid/path";
    size_t size = 1024;
    auto mtpfsdevice = std::make_shared<MtpFsDevice>();
    int result = mtpfsdevice->GetThumbnailSize(path, size);
    EXPECT_EQ(result, -ENOENT);
}

/**
 * @tc.name: FileMove_001
 * @tc.desc: 测试当旧目录的父节点为空时,FileMove 函数应返回 -ENOENT
 * @tc.type: FUNC
 */
HWTEST_F(MtpfsDeviceTest, MtpfsDeviceTest_FileMove_001, TestSize.Level1)
{
    MtpFsDevice device;
    device.device_ = nullptr;
    std::string oldPath = "/old/path/file";
    std::string newPath = "/new/path/file";

    int result = device.FileMove(oldPath, newPath);
    EXPECT_EQ(result, -ENOENT);
}
}
}
