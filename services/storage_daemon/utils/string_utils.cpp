/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <vector>
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
    va_list ap_backup;
    va_start(ap, format);
    va_copy(ap_backup, ap);
    char buf[BUFF_SIZE] = {0};
    std::string result;

    int count = vsnprintf_s(buf, sizeof(buf), sizeof(buf), format, ap_backup);
    if (count < 0) {
        LOGE("vsnprintf_s error, errno %{public}d", errno);
    } else if (count >= 0 && count < BUFF_SIZE) {
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

    va_end(ap_backup);
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
    int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        LOGE("open %{public}s failed, errno %{public}d", path, errno);
        return false;
    }

    long len = write(fd, data, size);
    if (len < 0) {
        LOGE("write %{public}s failed, errno %{public}d", path, errno);
        (void)close(fd);
        return false;
    }
    if (static_cast<size_t>(len) != size) {
        LOGE("write return len %{public}ld, not equal to content length %{public}zu", len, size);
        (void)close(fd);
        return false;
    }

    if (fsync(fd) != 0) {
        LOGE("fsync %{public}s failed, errno %{public}d", path, errno);
        (void)close(fd);
        return false;
    }
    (void)close(fd);
    return true;
}

bool SaveStringToFileSync(const std::string &path, const std::string &data)
{
    if (path.empty() || data.empty()) {
        return false;
    }
    LOGD("enter %{public}s, size=%{public}zu", path.c_str(), data.length());
    return WriteFileSync(path.c_str(), reinterpret_cast<const uint8_t *>(data.c_str()), data.size());
}
} // namespace StorageDaemon
} // namespace OHOS
