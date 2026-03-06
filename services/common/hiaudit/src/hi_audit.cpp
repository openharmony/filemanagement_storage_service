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

#include "hi_audit.h"

#include <dirent.h>
#include <fcntl.h>
#include <iomanip>
#include <sstream>
#include <unistd.h>

#include "storage_service_log.h"
#include "zip_utils.h"

namespace OHOS {
struct HiAuditConfig {
    std::string logPath;
    std::string logName;
    uint32_t logSize; // kb
    uint32_t fileSize;
    uint32_t fileCount;
};

const HiAuditConfig HIAUDIT_CONFIG = {
    "/data/log/hiaudit/storagedaemon/", "storagedaemon", 2 * 1024, 3 * 1024 * 1024, 10};
constexpr int8_t MILLISECONDS_LENGTH = 3;
constexpr int64_t SEC_TO_MILLISEC = 1000;
constexpr int MAX_TIME_BUFF = 64; // 64 : for example 2026-12-25-10-43-
constexpr int32_t HIAUDIT_E_OK = 0;
const std::string HIAUDIT_LOG_NAME = HIAUDIT_CONFIG.logPath + HIAUDIT_CONFIG.logName + "_audit.csv";
constexpr const char *HIAUDIT_SUCC = "SUCC";
constexpr const char *HIAUDIT_STATUS_SUCC = "SUCCESS";
constexpr const char *HIAUDIT_STATUS_FAIL = "FAIL";
constexpr const char *HIAUDIT_OP_TYPE = "STORAGE_DAEMON";

HiAudit::HiAudit()
{
    Init();
}

HiAudit::~HiAudit()
{
    if (writeFd_ >= 0) {
        fdsan_close_with_tag(writeFd_, LOG_DOMAIN);
    }
}

HiAudit& HiAudit::GetInstance()
{
    static HiAudit instance;
    return instance;
}

void HiAudit::Init()
{
    if (access(HIAUDIT_CONFIG.logPath.c_str(), F_OK) != 0) {
        int32_t ret = mkdir(HIAUDIT_CONFIG.logPath.c_str(), S_IRWXU | S_IRWXG | S_IXOTH);
        if (ret != 0) {
            LOGE("Failed to create directory %{public}s.", HIAUDIT_CONFIG.logPath.c_str());
        }
    }

    std::lock_guard<std::mutex> lock(mutex_);
    writeFd_ = open(HIAUDIT_LOG_NAME.c_str(), O_CREAT | O_APPEND | O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP);
    if (writeFd_ < 0) {
        LOGE("writeFd_ open error, errno: %{public}d.", errno);
    } else {
        fdsan_exchange_owner_tag(writeFd_, 0, LOG_DOMAIN);
    }
    struct stat st;
    writeLogSize_ = stat(HIAUDIT_LOG_NAME.c_str(), &st) ? 0 : static_cast<uint64_t>(st.st_size);
    LOGI("writeLogSize: %{public}u", writeLogSize_.load());
}

uint64_t HiAudit::GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

std::string HiAudit::GetFormattedTimestamp(time_t timeStamp, const std::string& format)
{
    auto seconds = timeStamp / SEC_TO_MILLISEC;
    char date[MAX_TIME_BUFF] = {0};
    struct tm result {};
    if (localtime_r(&seconds, &result) != nullptr) {
        strftime(date, MAX_TIME_BUFF, format.c_str(), &result);
    }
    return std::string(date);
}

std::string HiAudit::GetFormattedTimestampEndWithMilli()
{
    uint64_t milliSeconds = GetMilliseconds();
    uint64_t seconds = milliSeconds / SEC_TO_MILLISEC;
    std::string formattedTimeStamp = GetFormattedTimestamp(seconds, "%Y%m%d%H%M%S");
    std::stringstream ss;
    ss << formattedTimeStamp;
    uint64_t milliseconds = milliSeconds % SEC_TO_MILLISEC;
    ss << std::setfill('0') << std::setw(MILLISECONDS_LENGTH) << milliseconds;
    return ss.str();
}

void HiAudit::Write(const AuditLog& auditLog)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (writeLogSize_ == 0) {
        WriteToFile(auditLog.TitleString());
    }
    std::string writeLog = GetFormattedTimestampEndWithMilli() + ", " +
        HIAUDIT_CONFIG.logName + ", NO, " + auditLog.ToString();
    LOGD("write %{public}s.", writeLog.c_str());
    writeLog = writeLog + "\n";

    if (writeLogSize_ + writeLog.length() > HIAUDIT_CONFIG.fileSize) {
        LOGW("Log file size will exceed limit after write, rotate log file.");
        GetWriteFilePath();
    }
    
    WriteToFile(writeLog);
}

void HiAudit::WriteStart(const std::string &funcName, const std::string &extend)
{
    AuditLog auditLog = {false, 1, HIAUDIT_SUCC, HIAUDIT_OP_TYPE, funcName, "START", extend};
    Write(auditLog);
}

void HiAudit::WriteEnd(const std::string &funcName, int32_t ret, const std::string &extend)
{
    AuditLog auditLog = {false, 1, HIAUDIT_SUCC, HIAUDIT_OP_TYPE, funcName, "SUCCESS", extend};
    auditLog.cause = (ret == HIAUDIT_E_OK) ? HIAUDIT_SUCC : std::to_string(ret);
    auditLog.operationStatus = (ret == HIAUDIT_E_OK) ? HIAUDIT_STATUS_SUCC : HIAUDIT_STATUS_FAIL;
    Write(auditLog);
}

void HiAudit::GetWriteFilePath()
{
    if (writeLogSize_ < HIAUDIT_CONFIG.fileSize) {
        return;
    }
    if (writeFd_ >= 0) {
        LOGW("file content full, close fd.");
        fdsan_close_with_tag(writeFd_, LOG_DOMAIN);
        writeFd_ = -1; // -1 : for close fd
    }

    ZipAuditLog();
    CleanOldAuditFile();
    writeFd_ = open(HIAUDIT_LOG_NAME.c_str(), O_CREAT | O_TRUNC | O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP);
    if (writeFd_ < 0) {
        LOGE("writeFd_ open error, errno: %{public}d.", errno);
    } else {
        fdsan_exchange_owner_tag(writeFd_, 0, LOG_DOMAIN);
    }

    writeLogSize_ = 0;
}

void HiAudit::CleanOldAuditFile()
{
    uint32_t zipFileCount = 0;
    std::string oldestAuditFile;
    DIR* dir = opendir(HIAUDIT_CONFIG.logPath.c_str());
    if (dir == nullptr) {
        LOGE("failed open dir, errno: %{public}d.", errno);
        return;
    }
    struct dirent *ptr = nullptr;
    while ((ptr = readdir(dir)) != nullptr) {
        std::string subName = std::string(ptr->d_name);
        std::string zipTag = ".zip";
        if (subName.find(HIAUDIT_CONFIG.logName) == 0 &&
            subName.find(zipTag) == subName.length() - zipTag.length()) {
            zipFileCount = zipFileCount + 1;
            if (oldestAuditFile.empty()) {
                oldestAuditFile = HIAUDIT_CONFIG.logPath + subName;
                continue;
            }
            struct stat st;
            if (stat((HIAUDIT_CONFIG.logPath + subName).c_str(), &st) != 0) {
                LOGE("stat failed, errno: %{public}d, file: %{public}s.", errno, subName.c_str());
                continue;
            }
            struct stat oldestSt;
            if (stat(oldestAuditFile.c_str(), &oldestSt) != 0) {
                LOGE("stat failed, errno: %{public}d, file: %{public}s.", errno, oldestAuditFile.c_str());
                continue;
            }
            if (st.st_mtime < oldestSt.st_mtime) {
                oldestAuditFile = HIAUDIT_CONFIG.logPath + subName;
            }
        }
    }
    closedir(dir);
    if (zipFileCount > HIAUDIT_CONFIG.fileCount) {
        remove(oldestAuditFile.c_str());
    }
}

void HiAudit::WriteToFile(const std::string& content)
{
    GetWriteFilePath();
    if (writeFd_ < 0) {
        LOGE("writeFd_ invalid.");
        return;
    }
    size_t len = content.length();
    size_t ret = write(writeFd_, content.c_str(), len);
    if (ret != len) {
        LOGE("write failed, len: %{public}zd, ret: %{public}zd, errno: %{public}d.", len, ret, errno);
    }
    len = (ret > 0) ? ret : 0;
    writeLogSize_ = writeLogSize_ + len;
}

void HiAudit::ZipAuditLog()
{
    std::string zipFileName = HIAUDIT_CONFIG.logPath + HIAUDIT_CONFIG.logName + "_audit_" +
        GetFormattedTimestampEndWithMilli();
    if (std::rename(HIAUDIT_LOG_NAME.c_str(), (zipFileName + ".csv").c_str()) != 0) {
        LOGW("rename audit log file failed, errno: %{public}d.", errno);
        return;
    }
    zipFile compressZip = Storage::DistributedFile::ZipUtil::CreateZipFile(zipFileName + ".zip");
    if (compressZip == nullptr) {
        LOGW("open zip file failed.");
        return;
    }
    if (Storage::DistributedFile::ZipUtil::AddFileInZip(compressZip, zipFileName + ".csv",
        Storage::DistributedFile::KeepStatus::KEEP_NONE_PARENT_PATH) == 0) {
        remove((zipFileName + ".csv").c_str());
    }
    Storage::DistributedFile::ZipUtil::CloseZipFile(compressZip);
}
} // namespace OHOS