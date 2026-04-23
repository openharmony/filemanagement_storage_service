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

#ifndef FILEMANAGEMENT_HI_AUDIT_H
#define FILEMANAGEMENT_HI_AUDIT_H
#include <cstdio>
#include <mutex>
#include <sys/stat.h>

#include "nocopyable.h"

namespace OHOS {
struct AuditLog {
    bool isUserBehavior = false;
    uint32_t operationCount = 0;
    std::string cause;
    std::string operationType;
    std::string operationScenario;
    std::string operationStatus;
    std::string extend;

    const std::string TitleString() const
    {
        return "happenTime, packageName, isForeground, cause, isUserBehavior, operationType, operationScenario, "
               "operationStatus, operationCount, extend\n";
    }

    const std::string ToString() const
    {
        return cause + ", " + std::to_string(isUserBehavior) + ", " + operationType + ", " + operationScenario +
            ", " + operationStatus + ", " + std::to_string(operationCount) + ", " + extend;
    }
};

class HiAudit : public NoCopyable {
public:
    static HiAudit& GetInstance();
    void Write(const AuditLog& auditLog);
    void WriteStart(const std::string &funcName, const std::string &extend = "");
    void WriteEnd(const std::string &funcName, int32_t ret, const std::string &extend = "");

private:
    HiAudit();
    ~HiAudit();

    void Init();
    void GetWriteFilePath();
    void WriteToFile(const std::string& log);
    uint64_t GetMilliseconds();
    std::string GetFormattedTimestamp(time_t timeStamp, const std::string& format);
    std::string GetFormattedTimestampEndWithMilli();
    void CleanOldAuditFile();
    void ZipAuditLog();

private:
    std::mutex mutex_;
    int writeFd_ = -1; // -1: init fd
    std::atomic<uint32_t> writeLogSize_ = 0;
};
} // namespace OHOS
#endif // FILEMANAGEMENT_HI_AUDIT_H