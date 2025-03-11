/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <fcntl.h>
#include <unistd.h>

#include "securec.h"
#include "storage_service_log.h"

using namespace std;

namespace OHOS {
namespace StorageDaemon {
static constexpr int32_t BUFF_SIZE = 1024;
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
        LOGE("vsnprintf_s error, errno %{public}d", errno);
    } else if (count < BUFF_SIZE) {
        result.append(buf, count);
    } else {
        LOGI("allocate larger buffer, len = %{public}d", count + 1);

        char *newBuf = new char[count + 1];
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

bool WriteFileSync(const char *path, const uint8_t *data, size_t size)
{
    FILE *f = fopen(path, "w");
    if (f == nullptr) {
        LOGE("open %{public}s failed, errno %{public}d", path, errno);
        return false;
    }
    ChMod(path, S_IRUSR | S_IWUSR);
    int fd = fileno(f);
    if (fd == -1) {
        LOGE("open %{public}s failed, errno %{public}d", path, errno);
        (void)fclose(f);
        return false;
    }

    long len = write(fd, data, size);
    if (len < 0) {
        LOGE("write %{public}s failed, errno %{public}d", path, errno);
        (void)fclose(f);
        return false;
    }
    if (static_cast<size_t>(len) != size) {
        LOGE("write return len %{public}ld, not equal to content length %{public}zu", len, size);
        (void)fclose(f);
        return false;
    }

    if (fsync(fd) != 0) {
        LOGE("fsync %{public}s failed, errno %{public}d", path, errno);
        (void)fclose(f);
        return false;
    }
    (void)fclose(f);
    return true;
}

bool SaveStringToFileSync(const std::string &path, const std::string &data)
{
    if (path.empty() || data.empty()) {
        return false;
    }
    LOGI("enter %{public}s, size=%{public}zu", path.c_str(), data.length());
    return WriteFileSync(path.c_str(), reinterpret_cast<const uint8_t *>(data.c_str()), data.size());
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
} // namespace StorageDaemon
} // namespace OHOS
