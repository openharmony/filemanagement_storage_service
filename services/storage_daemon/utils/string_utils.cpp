/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "utils/hi_audit.h"
#include <dirent.h>
#include <fcntl.h>
#include <regex>
#include <unistd.h>
#include <memory>
#include <sstream>
#include <cinttypes>

#include "res_type.h"
#include "res_sched_client.h"
#include "securec.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
static constexpr int32_t BUFF_SIZE = 1024;
static constexpr int32_t THREAD_QOS_HIGH_LEVEL = 7; // 设置 qos 7 优先级41
static constexpr int32_t THREAD_QOS_LOW_LEVEL = -1; // 取消 qos 7
static constexpr const char *APP_EL1_PATH = "/data/app/el1";
static constexpr int32_t TWO_CHARACRER = 2;
static constexpr int32_t FOUR_CHARACRER = 4;
static constexpr int32_t FIVE_CHARACRER = 4;
std::string StringPrintf(const char *format, ...)
{
    va_list ap;
    va_list apBackup;
    va_start(ap, format);
    va_copy(apBackup, ap);
    char buf[BUFF_SIZE] = {0};
    std::string result;

    int count = vsnprintf_s(buf, sizeof(buf), sizeof(buf), format, apBackup);
    if (count < 0) {
        LOGE("[L8:StringUtils] StringPrintf: <<< EXIT FAILED <<< vsnprintf_s error, errno=%{public}d", errno);
    } else if (count < BUFF_SIZE) {
        result.append(buf, count);
    } else {
        LOGI("[L8:StringUtils] StringPrintf: allocate larger buffer, len=%{public}d", count + 1);

        char *newBuf = new (std::nothrow) char[count + 1];
        if (newBuf != nullptr) {
            count = vsnprintf_s(newBuf, count + 1, count + 1, format, ap);
            if (count >= 0) {
                result.append(newBuf, count);
            }
        }

        delete[] newBuf;
    }

    va_end(apBackup);
    va_end(ap);

    return result;
}

std::vector<std::string> SplitLine(std::string &line, std::string &token)
{
    std::vector<std::string> result;
    std::string::size_type start;
    std::string::size_type end;

    start = 0;
    end = line.find(token);
    while (std::string::npos != end) {
        result.push_back(line.substr(start, end - start));
        start = end + token.size();
        end = line.find(token, start);
    }

    if (start != line.length()) {
        result.push_back(line.substr(start));
    }

    return result;
}

bool WriteFileSync(const char *path, const uint8_t *data, size_t size, std::string &errMsg)
{
    LOGD("[L8:StringUtils] WriteFileSync: >>> ENTER <<< path=%{public}s, size=%{public}zu", path, size);
    FILE *f = fopen(path, "w");
    if (f == nullptr) {
        errMsg = "f == nullptr, errno" + std::to_string(errno);
        LOGE("[L8:StringUtils] WriteFileSync: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return false;
    }
    ChMod(path, S_IRUSR | S_IWUSR);
    int fd = fileno(f);
    if (fd == -1) {
        errMsg = "fd == -1, errno" + std::to_string(errno);
        LOGE("[L8:StringUtils] WriteFileSync: <<< EXIT FAILED <<< fd=-1, errno=%{public}d", errno);
        (void)fclose(f);
        return false;
    }

    long len = write(fd, data, size);
    if (len < 0) {
        errMsg = "fd == -1, errno" + std::to_string(errno);
        LOGE("[L8:StringUtils] WriteFileSync: <<< EXIT FAILED <<< write failed, errno=%{public}d", errno);
        (void)fclose(f);
        return false;
    }
    if (static_cast<size_t>(len) != size) {
        errMsg = "len != size, errno" + std::to_string(errno);
        LOGE("[L8:StringUtils] WriteFileSync: <<< EXIT FAILED <<< len mismatch, len=%{public}ld, size=%{public}zu",
            len, size);
        (void)fclose(f);
        return false;
    }

    if (fsync(fd) != 0) {
        errMsg = "fsync(fd), errno" + std::to_string(errno);
        LOGE("[L8:StringUtils] WriteFileSync: <<< EXIT FAILED <<< fsync failed, errno=%{public}d", errno);
        (void)fclose(f);
        return false;
    }
    (void)fclose(f);
    LOGD("[L8:StringUtils] WriteFileSync: <<< EXIT SUCCESS <<<");
    return true;
}

bool SaveStringToFileSync(const std::string &path, const std::string &data, std::string &errMsg)
{
    if (path.empty() || data.empty()) {
        LOGE("[L8:StringUtils] SaveStringToFileSync: <<< EXIT FAILED <<< empty input");
        return false;
    }
    LOGI("[L8:StringUtils] SaveStringToFileSync: >>> ENTER <<< path=%{public}s, size=%{public}zu",
        path.c_str(), data.length());
    bool ret = WriteFileSync(path.c_str(), reinterpret_cast<const uint8_t *>(data.c_str()), data.size(), errMsg);
    if (ret) {
        LOGI("[L8:StringUtils] SaveStringToFileSync: <<< EXIT SUCCESS <<<");
    } else {
        LOGE("[L8:StringUtils] SaveStringToFileSync: <<< EXIT FAILED <<<");
    }
    return ret;
}

bool StringIsNumber(const std::string &content)
{
    if (content.empty()) {
        return false;
    }
    bool isNum = true;
    for (char c : content) {
        if (!isdigit(c)) {
            isNum = false;
            break;
        }
    }
    return isNum;
}

bool IsStringExist(const std::list<std::string> &strList, const std::string &content)
{
    auto it = std::find(strList.begin(), strList.end(), content);
    return it != strList.end();
}

std::string ListToString(const std::list<std::string> &strList)
{
    if (strList.empty()) {
        return "";
    }
    std::string result;
    for (auto &iter : strList) {
        result += iter + ",";
    }
    return result.empty() ? "" : result.substr(0, result.length() -1);
}

void GetAllUserIds(std::vector<int32_t> &userIds)
{
    LOGD("[L8:StringUtils] GetAllUserIds: >>> ENTER <<<");
    auto procDir = std::unique_ptr<DIR, int (*)(DIR*)>(opendir(APP_EL1_PATH), closedir);
    if (!procDir) {
        LOGE("[L8:StringUtils] GetAllUserIds: <<< EXIT FAILED <<< open dir failed, errno=%{public}d", errno);
        return;
    }
    std::regex pattern("^[1-9]\\d*$");
    struct dirent *entry;
    while ((entry = readdir(procDir.get())) != nullptr) {
        if (entry->d_type != DT_DIR) {
            continue;
        }
        std::string name = entry->d_name;
        if (!std::regex_match(name, pattern)) {
            continue;
        }
        char *endptr = nullptr;
        errno = 0;
        int64_t tollRes = strtoll(name.c_str(), &endptr, BASE_DECIMAL);
        if (errno != 0 || endptr != name.c_str() + name.size()) {
            continue;
        }
        if (tollRes < INT32_MIN || tollRes > INT32_MAX) {
            continue;
        }
        int32_t userId = static_cast<int32_t>(tollRes);
        if (userId < StorageService::DEFAULT_USER_ID) {
            continue;
        }
        userIds.push_back(userId);
    }
    LOGD("[L8:StringUtils] GetAllUserIds: <<< EXIT SUCCESS <<< userIdCount=%{public}zu", userIds.size());
}

bool ConvertStringToInt(const std::string &str, int64_t &value, int32_t base)
{
    if (str.empty()) {
        return false;
    }

    errno = 0;
    char* endptr = nullptr;

    int64_t result = std::strtoll(str.c_str(), &endptr, base);
    
    if (endptr == str.c_str()) {
        return false;
    }
    if (errno == ERANGE && (result == LLONG_MAX || result == LLONG_MIN)) {
        return false;
    }
    if (*endptr != '\0') {
        return false;
    }
    value = result;
    LOGD("[L8:StringUtils] ConvertStringToInt: <<< EXIT SUCCESS <<< value=%{public}" PRId64, value);
    return true;
}

std::unordered_map<std::string, std::string> ParseKeyValuePairs(const std::string &input, char delimiter)
{
    std::unordered_map<std::string, std::string> result;
    std::istringstream ss(input);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        size_t equalPos = token.find('=');
        if (equalPos != std::string::npos) {
            std::string key = token.substr(0, equalPos);
            std::string value = token.substr(equalPos + 1);
            if (!key.empty()) {
                result[key] = value;
            }
        } else {
            std::string key = token;
            if (!key.empty()) {
                result[key] = "";
            }
        }
    }
    
    return result;
}

int32_t ReplaceAndCount(std::string &str, const std::string &target, const std::string &replacement)
{
    if (str.empty() || target.empty()) {
        return 0;
    }
    
    int32_t count = 0;
    size_t pos = 0;
    while ((pos = str.find(target, pos)) != std::string::npos) {
        str.replace(pos, target.length(), replacement);
        pos += replacement.length();
        count++;
    }
    
    return count;
}

bool ConvertStringToInt32(const std::string &context, int32_t &value)
{
    if (context.empty()) {
        return false;
    }
    std::regex pattern(R"(^([1-9]\d{0,9})$)");
    if (!std::regex_match(context, pattern)) {
        return false;
    }
    char *endptr;
    errno = 0;
    int64_t tollRes = strtoll(context.c_str(), &endptr, BASE_DECIMAL);
    if (errno != 0 || endptr != context.c_str() + context.size()) {
        return false;
    }
    if (tollRes <= 0 || tollRes >= INT32_MAX) {
        return false;
    }
    value = static_cast<int32_t>(tollRes);
    LOGD("[L8:StringUtils] ConvertStringToInt32: <<< EXIT SUCCESS <<< value=%{public}d", value);
    return true;
}

void IncreaseThreadPriority(const std::string &processName)
{
    HiAudit::GetInstance().WriteStart("IncreaseThreadPriority", "processName=" + processName);
    LOGI("IncreaseThreadPriority start");
    std::unordered_map<std::string, std::string> mapPayLoad;
    mapPayLoad["pid"] = std::to_string(getpid()); // 提升优先级的进程id
    mapPayLoad[std::to_string(gettid())] = std::to_string(THREAD_QOS_HIGH_LEVEL); // 提升优先级的线程id
    mapPayLoad["bundleName"] = processName;
    uint32_t type = OHOS::ResourceSchedule::ResType::RES_TYPE_THREAD_QOS_CHANGE;
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, mapPayLoad);
    LOGI("IncreaseThreadPriority end");
    HiAudit::GetInstance().WriteEnd("IncreaseThreadPriority", 0);
}

void DecreaseThreadPriority(const std::string &processName)
{
    HiAudit::GetInstance().WriteStart("DecreaseThreadPriority", "processName=" + processName);
    LOGI("DecreaseThreadPriority start");
    std::unordered_map<std::string, std::string> mapPayLoad;
    mapPayLoad["pid"] = std::to_string(getpid()); // 提升优先级的进程id
    mapPayLoad[std::to_string(gettid())] = std::to_string(THREAD_QOS_LOW_LEVEL); // 提升优先级的线程id
    mapPayLoad["bundleName"] = processName;
    uint32_t type = OHOS::ResourceSchedule::ResType::RES_TYPE_THREAD_QOS_CHANGE;
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(type, 0, mapPayLoad);
    LOGI("DecreaseThreadPriority end");
    HiAudit::GetInstance().WriteEnd("DecreaseThreadPriority", 0);
}

std::string AnonymizePath(const std::string &path)
{
    if (path.empty()) {
        return path;
    }

    // Find the last path separator (support both Unix '/' and Windows '\')
    size_t lastSlash = path.find_last_of("/\\");
    std::string dirPart;
    std::string fileName;

    if (lastSlash == std::string::npos) {
        // No directory part, only filename
        fileName = path;
    } else {
        // Split into directory and filename parts
        dirPart = path.substr(0, lastSlash + 1);
        fileName = path.substr(lastSlash + 1);
    }

    // Anonymize the filename
    if (fileName.length() <= FIVE_CHARACRER) {
        // For very short filenames (<=5 chars), replace middle characters except first and last
        if (fileName.length() >= TWO_CHARACRER) {
            size_t replaceCount = fileName.length() - TWO_CHARACRER;
            fileName = fileName[0] + std::string(replaceCount, '*') + fileName[fileName.length() - 1];
        } else {
            // For single character, just return as is
            // For empty string, return as is
        }
    } else {
        // For longer filenames, replace middle 4 characters
        // Calculate start position for middle 4 characters
        size_t startPos = (fileName.length() - FOUR_CHARACRER) / 2;
        fileName = fileName.substr(0, startPos) + "****" + fileName.substr(startPos + FOUR_CHARACRER);
    }

    return dirPart + fileName;
}
} // namespace StorageDaemon
} // namespace OHOS
