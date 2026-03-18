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
#include <cstring>
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

// ==================== RawDataCpy Tests ====================

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

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_RawDataCpy_0001
 * @tc.name: Storage_File_Raw_Data_RawDataCpy_0001
 * @tc.desc: Test RawDataCpy with negative size.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_RawDataCpy_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0001 start";

    StorageFileRawData storage;
    char data[10] = "test";
    storage.size = -1;
    EXPECT_EQ(storage.RawDataCpy(data), ERR_INVALID_DATA);
    EXPECT_EQ(storage.data, nullptr);
    EXPECT_FALSE(storage.isMalloc);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_RawDataCpy_0002
 * @tc.name: Storage_File_Raw_Data_RawDataCpy_0002
 * @tc.desc: Test RawDataCpy replacing existing data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_RawDataCpy_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0002 start";

    StorageFileRawData storage;
    char data1[10] = "first";
    storage.size = 10;
    EXPECT_EQ(storage.RawDataCpy(data1), ERR_NONE);
    void* firstPtr = const_cast<void*>(storage.data);
    EXPECT_NE(firstPtr, nullptr);

    // Replace with new data
    char data2[20] = "second_data_larger";
    storage.size = 20;
    EXPECT_EQ(storage.RawDataCpy(data2), ERR_NONE);
    void* secondPtr = const_cast<void*>(storage.data);
    EXPECT_NE(secondPtr, nullptr);
    EXPECT_NE(firstPtr, secondPtr);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_RawDataCpy_0003
 * @tc.name: Storage_File_Raw_Data_RawDataCpy_0003
 * @tc.desc: Test RawDataCpy with size at MAX boundary.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_RawDataCpy_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0003 start";

    StorageFileRawData storage;
    char data[1024];
    memset(data, 'X', 1024);

    // Test with size one less than MAX
    storage.size = StorageFileRawData::MAX_RAW_DATA_SIZE - 1;
    EXPECT_EQ(storage.RawDataCpy(data), ERR_INVALID_DATA);

    // Test with size equal to MAX (should fail)
    storage.size = StorageFileRawData::MAX_RAW_DATA_SIZE;
    EXPECT_EQ(storage.RawDataCpy(data), ERR_INVALID_DATA);

    // Test with size greater than MAX
    storage.size = StorageFileRawData::MAX_RAW_DATA_SIZE + 1;
    EXPECT_EQ(storage.RawDataCpy(data), ERR_INVALID_DATA);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_RawDataCpy_0003 end";
}

// ==================== Destructor Tests ====================

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_Destructor_0001
 * @tc.name: Storage_File_Raw_Data_Destructor_0001
 * @tc.desc: Test destructor with no data allocated.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_Destructor_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_Destructor_0001 start";

    StorageFileRawData* storage = new StorageFileRawData();
    storage->data = nullptr;
    storage->size = 0;
    storage->isMalloc = false;
    delete storage;
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_Destructor_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_Destructor_0002
 * @tc.name: Storage_File_Raw_Data_Destructor_0002
 * @tc.desc: Test destructor with allocated data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_Destructor_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_Destructor_0002 start";

    StorageFileRawData* storage = new StorageFileRawData();
    char data[100] = "test_data_for_destructor";
    storage->size = 100;
    storage->RawDataCpy(data);
    EXPECT_NE(storage->data, nullptr);
    EXPECT_TRUE(storage->isMalloc);

    // Destructor should free allocated memory
    delete storage;
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_Destructor_0002 end";
}

// ==================== InitialState Tests ====================

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_InitialState_0001
 * @tc.name: Storage_File_Raw_Data_InitialState_0001
 * @tc.desc: Test initial state of StorageFileRawData.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_InitialState_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_InitialState_0001 start";

    StorageFileRawData storage;
    EXPECT_EQ(storage.data, nullptr);
    EXPECT_EQ(storage.size, 0);
    EXPECT_FALSE(storage.isMalloc);

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_InitialState_0001 end";
}

// ==================== MultipleCopies Tests ====================

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_MultipleCopies_0001
 * @tc.name: Storage_File_Raw_Data_MultipleCopies_0001
 * @tc.desc: Test creating multiple objects and copying data.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_MultipleCopies_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0001 start";

    const int count = 10;
    StorageFileRawData storages[count];

    for (int i = 0; i < count; i++) {
        std::string dataStr = "data_" + std::to_string(i);
        storages[i].size = static_cast<uint32_t>(dataStr.size() + 1);
        EXPECT_EQ(storages[i].RawDataCpy(dataStr.c_str()), ERR_NONE);
        EXPECT_NE(storages[i].data, nullptr);
        EXPECT_TRUE(storages[i].isMalloc);
    }

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_MultipleCopies_0002
 * @tc.name: Storage_File_Raw_Data_MultipleCopies_0002
 * @tc.desc: Test multiple objects with different sizes.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_MultipleCopies_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0002 start";

    uint32_t sizes[] = {10, 50, 100, 500, 1000};
    const int count = sizeof(sizes) / sizeof(sizes[0]);

    for (int i = 0; i < count; i++) {
        StorageFileRawData storage;
        char* data = new char[sizes[i]];
        memset(data, 'A' + i, sizes[i]);
        storage.size = sizes[i];
        EXPECT_EQ(storage.RawDataCpy(data), ERR_NONE);
        EXPECT_NE(storage.data, nullptr);
        EXPECT_TRUE(storage.isMalloc);
        delete[] data;
    }

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_FILE_RAW_DATA_MultipleCopies_0003
 * @tc.name: Storage_File_Raw_Data_MultipleCopies_0003
 * @tc.desc: Test sequential copies with increasing sizes.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: SR000GGUPG
 */
HWTEST_F(StorageFileRawDataTest, Storage_File_Raw_Data_MultipleCopies_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0003 start";

    StorageFileRawData storage;

    for (uint32_t size = 10; size <= 100; size += 10) {
        char* data = new char[size];
        memset(data, 'X', size);
        storage.size = size;
        EXPECT_EQ(storage.RawDataCpy(data), ERR_NONE);
        EXPECT_NE(storage.data, nullptr);
        EXPECT_TRUE(storage.isMalloc);
        delete[] data;
    }

    GTEST_LOG_(INFO) << "Storage_File_Raw_Data_MultipleCopies_0003 end";
}

}
