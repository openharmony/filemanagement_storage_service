/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <fstream>
#include <map>
#include <nocopyable.h>
#include "statistic_info.h"
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <atomic>

#include "userdata_dir_info.h"

namespace OHOS {
namespace StorageDaemon {

struct UidSaInfo {
    int32_t uid;
    std::string saName;
    int64_t size;
    uint64_t iNodes;

    UidSaInfo(int32_t uid, const std::string& saName, int64_t size, uint64_t iNodes = 0)
        : uid(uid), saName(saName), size(size), iNodes(iNodes) {}

    UidSaInfo() : uid(0), saName(""), size(0), iNodes(0) {}
};

using NextDqBlk = OHOS::StorageManager::NextDqBlk;
using DirSpaceInfo = OHOS::StorageManager::DirSpaceInfo;

struct KernelNextDqBlk {
    uint64_t dqbHardLimit = 0;
    uint64_t dqbBSoftLimit = 0;
    uint64_t dqbCurSpace = 0;
    uint64_t dqbIHardLimit = 0;
    uint64_t dqbISoftLimit = 0;
    uint64_t dqbCurInodes = 0;
    uint64_t dqbBTime = 0;
    uint64_t dqbITime = 0;
    uint32_t dqbValid = 0;
    uint32_t dqbId = 0;
};

struct AllAppVec {
    std::vector<struct UidSaInfo> sysSaVec;
    std::vector<struct UidSaInfo> sysAppVec;
    std::vector<struct UidSaInfo> userAppVec;
    std::vector<struct UidSaInfo> otherAppVec;
};

class QuotaManager final {
public:
    virtual ~QuotaManager() = default;
    static QuotaManager &GetInstance();

    int32_t SetBundleQuota(int32_t uid, const std::string &bundleDataDirPath, int32_t limitSizeMb);
    int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size);
    int32_t SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit);
    void GetUidStorageStats(const std::string &storageStatus);
    void GetUidStorageStats(std::string &storageStatus, const std::map<int32_t, std::string> &bundleNameAndUid);
    int32_t GetFileData(const std::string &path, int64_t &size);
    int32_t GetDqBlkSpacesByUids(const std::vector<int32_t> &uids, std::vector<NextDqBlk> &dqBlks);
    int32_t GetDirListSpace(std::vector<DirSpaceInfo> &dirs);
    void SetStopScanFlag(bool stop);
    void GetAncoSizeData(std::string &extraData);
    int32_t ListUserdataDirInfo(std::vector<OHOS::StorageManager::UserdataDirInfo> &scanDirs);
private:
    QuotaManager() = default;
    DISALLOW_COPY_AND_MOVE(QuotaManager);
    void ProcessVecList(struct AllAppVec &allVec, const std::map<int32_t, std::string> &bundleNameAndUid);
    void GetOccupiedSpaceForUidList(struct AllAppVec &allVec, uint64_t &iNodes);
    void SortAndCutSaInfoVec(std::vector<struct UidSaInfo> &vec);
    void AssembleSaInfoVec(std::vector<struct UidSaInfo> &vec,
        const std::map<int32_t, std::string> &bundleNameAndUid);
    void WriteExtraData(const std::vector<UidSaInfo> &vec, std::ostringstream &extraData);
    bool GetUid32FromEntry(const std::string &entry, int32_t &outUid32, std::string &saName);
    bool StringToInt32(const std::string &strUid, int32_t &outUid32);
    int32_t ParseConfigFile(const std::string &path, std::vector<struct UidSaInfo> &vec);
    double ConvertBytesToMB(int64_t bytes, int32_t decimalPlaces);
    int32_t AddBlksRecurse(const std::string &path, int64_t &blks, uid_t uid);
    int32_t AddBlks(const std::string &path, int64_t &blks, uid_t uid);
    void GetMetaData(std::ostringstream &extraData);
    bool StringToInt64(const std::string& str, int64_t& out_value);
    void GetCurrentTime(std::ostringstream &extraData);
    void AssembleSysAppVec(int32_t dqUid, const KernelNextDqBlk &dq,
        std::map<int32_t, int64_t> &userAppSizeMap, std::vector<struct UidSaInfo> &sysAppVec);
    void GetSaOrOtherTotal(const std::vector<UidSaInfo> &vec, std::ostringstream &extraData, bool isSaVec);
    void ProcessSingleDir(const DirSpaceInfo &dirInfo, std::vector<DirSpaceInfo> &resultDirs);
    void ProcessDirWithUserId(const DirSpaceInfo &dirInfo, const std::vector<int32_t> &userIds,
        std::vector<DirSpaceInfo> &resultDirs);
    OHOS::StorageManager::UserdataDirInfo ScanDirRecurse(const std::string &path,
        std::vector<OHOS::StorageManager::UserdataDirInfo> &scanDirs);
    std::atomic<bool> stopScanFlag_{false};
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H