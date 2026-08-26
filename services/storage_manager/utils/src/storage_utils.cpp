/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "utils/storage_utils.h"

#include <climits>
#include <cstdlib>
#include <regex>
#include <filesystem>

#include "ipc_skeleton.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "storage_service_constants.h"

namespace OHOS {
namespace StorageManager {
constexpr const char *PATH_INVALID_FLAG1 = "../";
constexpr const char *PATH_INVALID_FLAG2 = "/..";
constexpr int32_t PATH_INVALID_FLAG_LEN = 3;
constexpr char FILE_SEPARATOR_CHAR = '/';
constexpr size_t INPUT_LIST_LEN = 50000;
constexpr size_t PKG_NAME_LEN = 128;
int64_t GetRoundSize(int64_t size)
{
    int64_t val = 1;
    int64_t multiple = UNIT;
    while (val * multiple < size) {
        auto tmpVal = static_cast<uint64_t>(val);
        tmpVal <<= 1;
        val = static_cast<int64_t>(tmpVal);
        if (val > THRESHOLD && multiple < ONE_GB) {
            val = 1;
            multiple *= UNIT;
        }
    }
    return val * multiple;
}

std::string GetAnonyString(const std::string &value)
{
    constexpr size_t INT32_SHORT_ID_LENGTH = 20;
    constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
    constexpr size_t INT32_MIN_ID_LENGTH = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

bool IsPathStartWithFileMgr(int32_t userId, const std::string &path)
{
    const std::string prefix = "/mnt/data/" + std::to_string(userId) + "/userExternal/";
    if (path.size() <= prefix.size()) {
        LOGE("path is too short, path: %{public}s", GetAnonyString(path).c_str());
        return false;
    }
    if (path.compare(0, prefix.length(), prefix) != 0) {
        LOGE("path is not start with %{public}s, path: %{public}s", prefix.c_str(), GetAnonyString(path).c_str());
        return false;
    }
    return true;
}

int GetCurrentUserId()
{
    int uid = -1;
    uid = IPCSkeleton::GetCallingUid();
    int userId = uid / 200000;
    return userId;
}

bool IsFilePathInvalid(const std::string &filePath)
{
    if (filePath.empty()) {
        LOGE("File path is empty");
        return true;
    }
    std::filesystem::path path(filePath);
    if (!path.is_absolute()) {
        LOGE("Relative path is not allowed");
        return true;
    }
    char resolvedPath[PATH_MAX];
    if (filePath.size() >= PATH_MAX) {
        LOGE("FilePath size is invalid");
        return true;
    }
    errno = 0;
    if (!realpath(filePath.c_str(), resolvedPath)) {
        if (errno == ENOENT) {
            LOGW("Path does not exist");
            return ContainsRelativePathReference(filePath);
        }
        LOGE("Realpath isfailed");
        return true;
    }
    if (std::string(resolvedPath) != filePath) {
        LOGE("Symbolic links is not allowed");
        return true;
    }
    return false;
}

bool ContainsRelativePathReference(const std::string &filePath)
{
    size_t pos = filePath.find(PATH_INVALID_FLAG1);
    while (pos != std::string::npos) {
        if (pos == 0 || filePath[pos - 1] == FILE_SEPARATOR_CHAR) {
            LOGE("Relative path is not allowed, path contain ../");
            return true;
        }
        pos = filePath.find(PATH_INVALID_FLAG1, pos + PATH_INVALID_FLAG_LEN);
    }
    pos = filePath.rfind(PATH_INVALID_FLAG2);
    if ((pos != std::string::npos) && (filePath.size() - pos == PATH_INVALID_FLAG_LEN)) {
        LOGE("Relative path is not allowed, path tail is /..");
        return true;
    }
    return false;
}

bool IsPathStartWithDlp(const std::string &dstPath)
{
    if (dstPath.empty()) {
        LOGE("IsDlpPathValid: dstPath is empty");
        return false;
    }
    const std::string prefix = "/data/service/el1/public/dlp_credential_service/";
    if (dstPath.compare(0, prefix.length(), prefix) != 0) {
        LOGE("IsDlpPathValid: path %{public}s does not start with dlp prefix", dstPath.c_str());
        return false;
    }
    return true;
}

bool CheckPkgNameRange(const std::string &pkgName)
{
    if (pkgName.empty()) {
        LOGE("CheckPkgNameRange pkgName is empty");
        return false;
    }
    if (pkgName.length() > PKG_NAME_LEN) {
        LOGE("CheckPkgNameRange pkgName is invalid");
        return false;
    }
    return true;
}

bool CheckAppIndexRange(int32_t appIndex)
{
    if (appIndex < 0) {
        LOGE("CheckAppIndexRange appIndex is out of range");
        return false;
    }
    return true;
}

bool CheckLevelRange(uint32_t level)
{
    if ((level < StorageService::EL1_SYS_KEY) || (level > StorageService::EL5_USER_KEY)) {
        LOGE("CheckLevelRange level is out of range");
        return false;
    }
    return true;
}

bool CheckInputListRange(const std::vector<std::string> &inputList)
{
    if (inputList.empty()) {
        LOGE("CheckInputListRange inputList is empty");
        return false;
    }
    if (inputList.size() > INPUT_LIST_LEN) {
        LOGE("CheckInputListRange inputList is out of range");
        return false;
    }
    return true;
}

bool CheckIdRange(const std::string &id)
{
    if (id.empty()) {
        LOGE("CheckIdRange id is empty");
        return false;
    }
    std::string idPattern = R"([0-9a-zA-Z]{1,65})";
    std::regex idRegex(idPattern);
    if (!std::regex_match(id, idRegex)) {
        LOGE("CheckIdRange id is invalid");
        return false;
    }
    return true;
}
} // namespace STORAGE_Manager
} // namespace OHOS