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

#include "storge_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <list>

#include "fuzzer/FuzzedDataProvider.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"

namespace OHOS {
namespace StorageDaemon {

/**
 * @brief Type cast helper for extracting typed data from fuzz input
 */
template<typename T>
T TypeCast(const uint8_t *data, int *pos = nullptr)
{
    if (pos) {
        *pos += sizeof(T);
    }
    return *(reinterpret_cast<const T *>(data));
}

/**
 * @brief Fuzz test for string utility functions
 */
bool StringUtilsFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(char)) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Generate random strings for testing
    std::string str1 = fdp.ConsumeRandomLengthString(size / 2);
    std::string str2 = fdp.ConsumeRandomLengthString(size / 2);

    // Test IsEndWith
    IsEndWith(str1, str2);

    // Test StringPrintf with various format strings
    const char *testFormat = "%s test %d";
    StringPrintf(testFormat, str1.c_str(), fdp.ConsumeIntegral<int>());

    // Test StringIsNumber
    StringIsNumber(str1);

    // Test IsStringExist
    std::list<std::string> strList;
    strList.push_back(str1);
    strList.push_back(str2);
    IsStringExist(strList, str1);

    // Test ListToString
    ListToString(strList);

    // Test ConvertStringToInt
    int64_t value = 0;
    ConvertStringToInt(str1, value);
    ConvertStringToInt(str1, value, BASE_OCTAL);

    // Test ParseKeyValuePairs
    ParseKeyValuePairs(str1, fdp.ConsumeIntegral<char>());

    // Test ReplaceAndCount
    std::string strCopy = str1;
    ReplaceAndCount(strCopy, str2, str1);

    // Test SplitLine
    std::string token = " ";
    std::string lineCopy = str1;
    SplitLine(lineCopy, token);

    return true;
}

/**
 * @brief Fuzz test for file utility functions
 */
bool FileUtilsFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    // Generate random data for testing
    uint32_t userId = fdp.ConsumeIntegral<uint32_t>();
    std::string path = fdp.ConsumeRandomLengthString(256);

    // Test IsDir and IsFile
    IsDir(path);
    IsFile(path);

    // Test IsFileExist and IsFolder
    IsFileExist(path);
    IsFolder(path);

    // Test StringToUint32
    uint32_t num = 0;
    StringToUint32(path, num);

    // Test StringToBool
    bool boolResult = false;
    StringToBool(path, boolResult);

    // Test GetSubDirs
    std::vector<std::string> dirList;
    GetSubDirs(path, dirList);

    // Test ReadDigitDir
    struct FileList fileList = { userId, path };
    std::vector<FileList> dirInfo;
    dirInfo.push_back(fileList);
    ReadDigitDir(path, dirInfo);

    // Test ReadFile
    std::string fileContent;
    ReadFile(path, &fileContent);

    // Test Split
    std::string pattern = fdp.ConsumeRandomLengthString(10);
    if (!pattern.empty()) {
        Split(path, pattern);
    }

    // Test IsPathMounted
    std::string mountPath = path;
    IsPathMounted(mountPath);

    // Test IsValidPath and IsValidRgmName
    IsValidPath(path);
    IsValidRgmName(path);

    // Test IsValidBusinessPath
    std::string userIdStr = std::to_string(userId);
    IsValidBusinessPath(path, userIdStr);

    // Test GetFileSize
    GetFileSize(path);

    return true;
}

/**
 * @brief Fuzz test for directory operations (read-only, safe operations)
 */
bool DirectoryOpsFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return false;
    }

    FuzzedDataProvider fdp(data, size);

    std::string path = fdp.ConsumeRandomLengthString(256);
    uint32_t userId = fdp.ConsumeIntegral<uint32_t>();

    // Test IsTempFolder
    std::string subPath = fdp.ConsumeRandomLengthString(64);
    IsTempFolder(path, subPath);

    // Test IsBusinessPath
    std::string userIdStr = std::to_string(userId);
    IsBusinessPath(path, userIdStr);

    // Test OpenSubFile (read-only)
    std::vector<std::string> subFiles;
    OpenSubFile(path, subFiles);

    return true;
}

/**
 * @brief Comprehensive fuzz test combining all utility functions
 */
void ComprehensiveFuzzTest(const uint8_t *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    // Split data into multiple segments for different tests
    size_t segmentSize = size / 3;
    if (segmentSize == 0) {
        segmentSize = size;
    }

    // Test string utilities
    StringUtilsFuzzTest(data, segmentSize);

    // Test file utilities
    if (size > segmentSize) {
        FileUtilsFuzzTest(data + segmentSize, segmentSize);
    }

    // Test directory operations
    if (size > segmentSize * 2) {
        DirectoryOpsFuzzTest(data + segmentSize * 2, size - segmentSize * 2);
    }
}

} // namespace StorageDaemon
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run comprehensive fuzz tests */
    OHOS::StorageDaemon::ComprehensiveFuzzTest(data, size);
    return 0;
}

