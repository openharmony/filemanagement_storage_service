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

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace testing::ext;
using namespace testing;

class MtpfsFuseTest : public testing::Test {
public:
    static void SetUpTestCase(void){};
    static void TearDownTestCase(void){};
    void SetUp(){};
    void TearDown(){};
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
    auto mtpfsdevice = std::make_shared<MtpfsDevice>();
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
    auto mtpfsdevice = std::make_shared<MtpfsDevice>();
    LIBMTP_error_number_t err = LIBMTP_ERROR_NONE;
    bool result = mtpfsdevice->ConvertErrorCode(err);
    EXPECT_EQ(result, false);

    LIBMTP_error_number_t err1 = LIBMTP_ERROR_NO_DEVICE_ATTACHED;
    bool result1 = mtpfsdevice->ConvertErrorCode(err1);
    EXPECT_EQ(result1, true);

    LIBMTP_error_number_t err2 = LIBMTP_ERROR_CONNECTING;
    bool result2 = mtpfsdevice->ConvertErrorCode(err2);
    EXPECT_EQ(result2, true);

    LIBMTP_error_number_t err3 = LIBMTP_ERROR_MEMORY_ALLOCATION;
    bool result3 = mtpfsdevice->ConvertErrorCode(err3);
    EXPECT_EQ(result3, true);

    LIBMTP_error_number_t err4 = LIBMTP_ERROR_GENERAL;
    bool result4 = mtpfsdevice->ConvertErrorCode(err4);
    EXPECT_EQ(result4, true);

    LIBMTP_error_number_t err5 = LIBMTP_ERROR_USB_LAYER;
    bool result5 = mtpfsdevice->ConvertErrorCode(err5);
    EXPECT_EQ(result5, true);

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
    auto mtpfsdevice = std::make_shared<MtpfsDevice>();
    bool result = mtpfsdevice->ConnectByDevNo(dev);
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
    auto mtpfsdevice = std::make_shared<MtpfsDevice>();
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
    LIBMTP_mtpdevice_t *device_ = nullptr;
    auto mtpfsdevice = std::make_shared<MtpfsDevice>();
    bool result = mtpfsdevice->Disconnect();
    EXPECT_EQ(device_, nullptr);
    GTEST_LOG_(INFO) << "MtpfsDeviceTest_Disconnect_001 end";
}


}
}

