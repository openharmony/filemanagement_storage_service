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
#include <string>
#include <sys/stat.h>
#include <sys/statvfs.h>

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
    uint64_t iNodes;

    UidSaInfo(int32_t uid, const std::string& saName, int64_t size, uint64_t iNodes = 0)
        : uid(uid), saName(saName), size(size), iNodes(iNodes) {}

    UidSaInfo() : uid(0), saName(""), size(0), iNodes(0) {}
};

struct DirSpaceInfo {
    std::string path;
    uid_t uid;
    int64_t size;
};

struct NextDqBlk {
    /* Absolute limit on disk quota blocks alloc */
    uint64_t dqbHardLimit;
    /* Preferred limit on disk quota blocks */
    uint64_t dqbBSoftLimit;
    /* Current occupied space(in bytes) */
    uint64_t dqbCurSpace;
    /* Maximum number of allocated inodes */
    uint64_t dqbIHardLimit;
    /* Preferred inode limit */
    uint64_t dqbISoftLimit;
    /* Current number of allocated inodes */
    uint64_t dqbCurInodes;
    /* Time limit for excessive disk use */
    uint64_t dqbBTime;
    /* Time limit for excessive files */
    uint64_t dqbITime;
    /* Bit mask of QIF_* constants */
    uint32_t dqbValid;
    /* the next ID greater than or equal to id that has a quota set */
    uint32_t dqbId;
};

struct AllAppVec {
    std::vector<struct UidSaInfo> sysSaVec;
    std::vector<struct UidSaInfo> sysAppVec;
    std::vector<struct UidSaInfo> userAppVec;
    std::vector<struct UidSaInfo> otherAppVec;
};

uint32_t CheckOverLongPath(const std::string &path);
class QuotaManager final {
public:
    virtual ~QuotaManager() = default;
    static QuotaManager &GetInstance();

    int32_t SetBundleQuota(const std::string &bundleName, int32_t uid,
        const std::string &bundleDataDirPath, int32_t limitSizeMb);
    int32_t GetOccupiedSpace(int32_t idType, int32_t id, int64_t &size);
    int32_t SetQuotaPrjId(const std::string &path, int32_t prjId, bool inherit);
    void GetUidStorageStats(const std::string &storageStatus);
    int32_t StatisticSysDirSpace();
    void GetUidStorageStats(std::string &storageStatus, const std::map<int32_t, std::string> &bundleNameAndUid);
    int32_t GetFileData(const std::string &path, int64_t &size);
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
    std::string AddDirSpace(const std::vector<DirSpaceInfo> &dirInfos, const std::vector<int32_t> &userIds);
    int32_t AddBlksRecurse(const std::string &path, int64_t &blks, uid_t uid);
    int32_t AddBlks(const std::string &path, int64_t &blks, uid_t uid);
    bool IsNeedScan();
    void GetMetaData(std::ostringstream &extraData);
    bool StringToInt64(const std::string& str, int64_t& out_value);
    void GetCurrentTime(std::ostringstream &extraData);
    int32_t CheckOccupation(std::ostringstream &extraData);
    void AssembleSysAppVec(int32_t dqUid, const struct NextDqBlk &dq,
        std::map<int32_t, int64_t> &userAppSizeMap, std::vector<struct UidSaInfo> &sysAppVec);
    void GetSaOrOtherTotal(const std::vector<UidSaInfo> &vec, std::ostringstream &extraData, bool isSaVec);
    int64_t oldChangeSizeCache_ = 0;
};
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_QUOTA_MANAGER_H