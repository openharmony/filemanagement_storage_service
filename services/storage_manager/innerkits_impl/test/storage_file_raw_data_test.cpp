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

#include <cstdio>
#include <gtest/gtest.h>

#include "storage_file_raw_data.h"
#include "ipc_types.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
class StorageFileRawDataTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_RawDataCpy_0000
 * @tc.name: Storage_File_Raw_Data_RawDataCpy_0000
 * @tc.desc: Test function of RawDataCpy interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_RawDataCpy_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageFileRawDataTest-begin Storage_File_Raw_Data_RawDataCpy_0000";

    StorageFileRawData storage1;
    storage1.size = 10;
    EXPECT_EQ(storage1.RawDataCpy(nullptr), ERR_INVALID_DATA);

    StorageFileRawData storage2;
    char readData1[10];
    storage2.size = 0;
    EXPECT_EQ(storage2.RawDataCpy(readData1), ERR_INVALID_DATA);

    StorageFileRawData storage3;
    char readData2[10] = "test";
    storage3.size = 10;
    EXPECT_EQ(storage3.RawDataCpy(readData2), ERR_NONE);

    GTEST_LOG_(INFO) << "StorageFileRawDataTest-end Storage_File_Raw_Data_RawDataCpy_0000";
}
}