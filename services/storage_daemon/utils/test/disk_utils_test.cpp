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

#include "mock/file_utils_mock.h"
#include "mock/disk_func_mock.h"
#include "utils/disk_utils.h"
#include "storage_service_errno.h"
#include "securec.h"

#include "disk_func_define.h"
#include "disk_utils.cpp"
#include "disk_func_undef.h"

namespace OHOS {
namespace StorageDaemon {
using namespace testing::ext;
using namespace testing;

class DiskUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    static inline std::shared_ptr<DiskFuncMock> diskFuncMock_ = nullptr;
};

void DiskUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
    diskFuncMock_ = std::make_shared<DiskFuncMock>();
    DiskFuncMock::diskFunc_ = diskFuncMock_;
}

void DiskUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
    DiskFuncMock::diskFunc_ = nullptr;
    diskFuncMock_ = nullptr;
}

void DiskUtilsTest::SetUp(void)
{
    fileUtilMoc_ = std::make_shared<FileUtilMoc>();
    FileUtilMoc::fileUtilMoc = fileUtilMoc_;
    diskFuncMock_ = std::make_shared<DiskFuncMock>();
    DiskFuncMock::diskFunc_ = diskFuncMock_;
}

void DiskUtilsTest::TearDown(void)
{
    if (fileUtilMoc_) {
        Mock::VerifyAndClearExpectations(fileUtilMoc_.get());
    }
    if (diskFuncMock_) {
        Mock::VerifyAndClearExpectations(diskFuncMock_.get());
    }
    FileUtilMoc::fileUtilMoc = nullptr;
    fileUtilMoc_ = nullptr;
    DiskFuncMock::diskFunc_ = nullptr;
    diskFuncMock_ = nullptr;
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_001
 * @tc.desc: Verify ReadMetadata with valid data (normal case).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_001 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string expectedUuid = "550e8400-e29b-41d4-a716-446655440000";
    const std::string expectedType = "ext4";
    const std::string expectedLabel = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {expectedUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {expectedType + "\n"};
                } else if (callCount == 3) {
                    *output = {expectedLabel + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, type, label;
    int32_t result = ReadMetadata(devPath, uuid, type, label);
    
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(uuid, expectedUuid);
    EXPECT_EQ(type, expectedType);
    EXPECT_EQ(label, expectedLabel);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_001 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_002
 * @tc.desc: Verify ReadMetadata with empty filesystem type (testing IsAcceptableUuid indirectly).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_002 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string validUuid = "550e8400-e29b-41d4-a716-446655440000";
    const std::string emptyType = "";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {validUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {emptyType + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, type, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, type, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_002 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_003
 * @tc.desc: Verify ReadMetadata with UUID containing forward slash (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_003 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string invalidUuid = "123/456";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {invalidUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_003 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_004
 * @tc.desc: Verify ReadMetadata with UUID containing backslash (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_004 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string invalidUuid = "123\\456";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {invalidUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_004 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_005
 * @tc.desc: Verify ReadMetadata with "." as UUID (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_005 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string dotUuid = ".";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {dotUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_005 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_006
 * @tc.desc: Verify ReadMetadata with ".." as UUID (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_006 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string dotDotUuid = "..";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {dotDotUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_006 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_007
 * @tc.desc: Verify ReadMetadata with empty UUID (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_007 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string emptyUuid = "";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {emptyUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_007 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_008
 * @tc.desc: Verify ReadMetadata with minimum length UUID (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_008 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string minUuid = "a";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {minUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(uuid, minUuid);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_008 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_009
 * @tc.desc: Verify ReadMetadata with maximum length UUID (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_009 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string maxUuid(255, 'a');
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {maxUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    EXPECT_EQ(uuid, maxUuid);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_009 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_010
 * @tc.desc: Verify ReadMetadata with UUID exceeding maximum length (testing IsAcceptableUuid).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_010 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string tooLongUuid(256, 'a');
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {tooLongUuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string uuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, uuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_READMETADATA);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_010 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_011
 * @tc.desc: Verify ReadMetadata with empty label (normal case with empty label).
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_011 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string uuid = "550e8400-e29b-41d4-a716-446655440000";
    const std::string type = "ext4";
    const std::string emptyLabel = "";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {uuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {emptyLabel + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string resultUuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, resultUuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(resultLabel, emptyLabel);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_011 end";
}

/**
 * @tc.name: DiskUtilsTest_ReadMetadata_012
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_ReadMetadata_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_012 start";
    
    const std::string devPath = "/dev/sda1";
    const std::string uuid = "test-uuid_with.mixed-chars_123";
    const std::string type = "ext4";
    const std::string label = "DATA";
    
    int callCount = 0;
    
    EXPECT_CALL(*fileUtilMoc_, ForkExec(_, _, _)).Times(3)
        .WillRepeatedly([&](std::vector<std::string>& cmd,
                           std::vector<std::string>* output,
                           int* exitStatus) {
            if (output) {
                callCount++;
                if (callCount == 1) {
                    *output = {uuid + "\n"};
                } else if (callCount == 2) {
                    *output = {type + "\n"};
                } else if (callCount == 3) {
                    *output = {label + "\n"};
                }
            }
            if (exitStatus) {
                *exitStatus = 0;
            }
            return 0;
        });
    
    std::string resultUuid, resultType, resultLabel;
    int32_t result = ReadMetadata(devPath, resultUuid, resultType, resultLabel);
    
    EXPECT_EQ(result, E_OK);
    EXPECT_EQ(resultUuid, uuid);
    
    GTEST_LOG_(INFO) << "DiskUtilsTest_ReadMetadata_012 end";
}


/**
 * @tc.name: DiskUtilsTest_IsBlankCD_01
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_01 start";

    std::string diskBlock = "/dev/test/sr01";
    bool isBlankCD = false;
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(nullptr));
    int32_t result = IsBlankCD(diskBlock, isBlankCD);
    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_01 end";
}

/**
 * @tc.name: DiskUtilsTest_IsBlankCD_02
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_02 start";

    std::string diskBlock = "/dev/test/sr02";
    bool isBlankCD = false;
    FILE * tmpFile = tmpfile();
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(-1));
    int32_t result = IsBlankCD(diskBlock, isBlankCD);
    EXPECT_EQ(result, E_ERR);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_02 end";
}

/**
 * @tc.name: DiskUtilsTest_IsBlankCD_03
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_03 start";

    std::string diskBlock = "/dev/test/sr03";
    bool isBlankCD = false;
    FILE * tmpFile = tmpfile();
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(-1));
    int32_t result = IsBlankCD(diskBlock, isBlankCD);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_03 end";
}

/**
 * @tc.name: DiskUtilsTest_IsBlankCD_04
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_04 start";

    std::string diskBlock = "/dev/test/sr04";
    bool isBlankCD = false;
    FILE * tmpFile = tmpfile();
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));
    EXPECT_CALL(*diskFuncMock_, ioctl(_, _)).WillOnce(Return(0));
    int32_t result = IsBlankCD(diskBlock, isBlankCD);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_04 end";
}

/**
 * @tc.name: DiskUtilsTest_IsBlankCD_05
 * @tc.desc: Verify ReadMetadata with valid UUID containing hyphens and underscores.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DiskUtilsTest, DiskUtilsTest_IsBlankCD_05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_05 start";

    std::string diskBlock = "/dev/test/sr06";
    bool isBlankCD = false;
    FILE * tmpFile = tmpfile();
    EXPECT_CALL(*diskFuncMock_, fopen(_, _)).WillOnce(Return(tmpFile));
    EXPECT_CALL(*diskFuncMock_, fileno(_)).WillOnce(Return(0));

    int32_t result = IsBlankCD(diskBlock, isBlankCD);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "DiskUtilsTest_IsBlankCD_05 end";
}
} // STORAGE_DAEMON
} // OHOS
