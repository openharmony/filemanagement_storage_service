/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H
#define OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H

#include <nocopyable.h>
#include <string>

namespace OHOS {
namespace StorageDaemon {
struct FileStat {
    std::string filePath;
    int64_t fileSize;
    int64_t lastUpdateTime;
    int32_t mode;
    bool isDir;
    bool isIncre;
};
struct BundleStatsParas {
    uint32_t userId;
    std::string &bundleName;
    int64_t lastBackupTime;
    int64_t fileSizeSum;
    int64_t incFileSizeSum;
};

struct UidSaInfo {
    int32_t uid;
    std::string saName;
    int64_t size;
};

uint32_t CheckOverLongPath(const std::string &path);
class QuotaManager final {
public:
    virtual ~QuotaManager() = default;
    static QuotaManager* GetInstance();

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb);
    int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size);
    int32_t SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit);
    void GetUidStorageStats();
private:
    QuotaManager() = default;
    DISALLOW_COPY_AND_MOVE(QuotaManager);
    int64_t GetOccupiedSpaceForUidList(std::vector<struct UidSaInfo> &vec);
    bool GetUid32FromEntry(const std::string &entry, int32_t &outUid32, std::string &saName);
    bool StringToInt32(const std::string &strUid, int32_t &outUid32);
    int32_t ParseConfigFile(const std::string &path, std::vector<struct UidSaInfo> &vec);
    double ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces);

    static QuotaManager* instance_;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H