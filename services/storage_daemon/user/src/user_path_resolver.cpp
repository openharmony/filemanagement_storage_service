/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "user/user_path_resolver.h"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>

#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "storage_service_constant.h"
#include "utils/string_utils.h"
#include "utils/file_utils.h"
#include "user/mount_constant.h"
#include "quota/quota_manager.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageDaemon {
constexpr const char *STORAGE_ETC_PATH = "/etc/storage_daemon/";
constexpr const char *STORAGE_USER_PATH = "storage_user_path.json";
constexpr const char *STORAGE_MOUNT_INFO = "storage_mount_info.json";
#define MAX_JSON_FILE_LEN 102400

// 创建与挂载配置字段
constexpr const char *FIELDS_SRC_PATH = "src-path";
constexpr const char *FIELDS_DEST_PATH = "dest-path";
constexpr const char *FIELDS_FS = "file-system-type";
constexpr const char *FIELDS_MOUNT_FLAGS = "mount-flags";
constexpr const char *FIELDS_DATA = "data";
constexpr const char *FIELDS_PATH = "path";
constexpr const char *FIELDS_MODE = "mode";
constexpr const char *FIELDS_UID = "uid";
constexpr const char *FIELDS_GID = "gid";

// 可选属性
constexpr const char *FIELDS_OPTIONS = "options";
constexpr const char *FIELDS_CREATE_DEST_PATH = "create-dest-path";
constexpr const char *FIELDS_DEST_PATH_INFO = "dest-path-info";

// 可变参数
constexpr const char *USER_ID = "<userId>";

// 创建与挂载 扩展属性
constexpr const char *OPTIONS_UPDATE_UID = "update_uid";
constexpr const char *OPTIONS_CHECK_MOUNTED = "check_mounted";

// json文件配置路径
constexpr const char *JSON_KEY_USER_BASE = "user_base";
constexpr const char *JSON_KEY_USER_SERVICE = "user_service";
constexpr const char *JSON_KEY_APPDATA = "appdata";
constexpr const char *JSON_KEY_VIRTUAL = "virtual";
constexpr const char *JSON_KEY_APPDATA_MOUNT = "appdata-mount";
constexpr const char *JSON_KEY_SANDBOX_MOUNT = "sandbox-mount";
constexpr const char *JSON_KEY_HMDFS_MOUNT = "hmdfs-mount";

static const std::unordered_map<std::string, unsigned long> MOUNT_FLAGS_MAP {
    {"rec", MS_REC},    // 递归挂载
    {"bind", MS_BIND},  // 绑定挂载
    {"move", MS_MOVE},  // 移动挂载点
    {"slave", MS_SLAVE},    // 从属挂载
    {"rdonly", MS_RDONLY},  // 只读挂载
    {"shared", MS_SHARED},  // 共享挂载
    {"unbindable", MS_UNBINDABLE},  // 不可绑定挂载
    {"remount", MS_REMOUNT},    // 重新挂载
    {"nosuid", MS_NOSUID},  // 忽略SUID/SGID位
    {"nodev", MS_NODEV},    // 禁止访问设备文件
    {"noexec", MS_NOEXEC},  // 禁止执行程序
    {"noatime", MS_NOATIME},    // 不更新访问时间
    {"lazytime", MS_LAZYTIME},  // 延迟时间更新
};

void from_json(const nlohmann::json &j, DirInfo &dirInfo)
{
    if (j.contains(FIELDS_PATH) && j[FIELDS_PATH].is_string()) {
        dirInfo.path = j[FIELDS_PATH].get<std::string>();
    }

    if (j.contains(FIELDS_UID) && j[FIELDS_UID].is_number_integer()) {
        auto uid = j[FIELDS_UID].get<int32_t>();
        if (uid >= 0) {
            dirInfo.uid = static_cast<uid_t>(uid);
        }
    }

    if (j.contains(FIELDS_GID) && j[FIELDS_GID].is_number_integer()) {
        auto gid = j[FIELDS_GID].get<int32_t>();
        if (gid >= 0) {
            dirInfo.gid = static_cast<gid_t>(gid);
        }
    }

    int64_t mode = MODE_0711;
    if (j.contains(FIELDS_MODE) && j[FIELDS_MODE].is_string()) {
        ConvertStringToInt(j[FIELDS_MODE].get<std::string>(), mode, BASE_OCTAL);
    }
    dirInfo.mode = static_cast<mode_t>(mode);

    std::string optionsStr;
    if (j.contains(FIELDS_OPTIONS) && j[FIELDS_OPTIONS].is_string()) {
        optionsStr = j[FIELDS_OPTIONS].get<std::string>();
    }
    dirInfo.options = ParseKeyValuePairs(optionsStr, ',');
}

void from_json(const nlohmann::json &j, MountNodeInfo &mountNodeInfo)
{
    if (j.contains(FIELDS_SRC_PATH) && j[FIELDS_SRC_PATH].is_string()) {
        mountNodeInfo.srcPath = j[FIELDS_SRC_PATH].get<std::string>();
    }

    if (j.contains(FIELDS_DEST_PATH) && j[FIELDS_DEST_PATH].is_string()) {
        mountNodeInfo.dstPath = j[FIELDS_DEST_PATH].get<std::string>();
    }
    
    if (j.contains(FIELDS_FS) && j[FIELDS_FS].is_string()) {
        mountNodeInfo.fsType = j[FIELDS_FS].get<std::string>();
    }
    
    if (j.contains(FIELDS_DATA) && j[FIELDS_DATA].is_string()) {
        mountNodeInfo.data = j[FIELDS_DATA].get<std::string>();
    }

    mountNodeInfo.mountFlags = 0;
    if (j.contains(FIELDS_MOUNT_FLAGS) && j[FIELDS_MOUNT_FLAGS].is_array()) {
        for (const auto &flag : j[FIELDS_MOUNT_FLAGS]) {
            if (!flag.is_string()) {
                continue;
            }
            auto it = MOUNT_FLAGS_MAP.find(flag.get<std::string>());
            if (it != MOUNT_FLAGS_MAP.end()) {
                mountNodeInfo.mountFlags |= it->second;
            }
        }
    }

    if (j.contains(FIELDS_CREATE_DEST_PATH) && j[FIELDS_CREATE_DEST_PATH].is_boolean()) {
        mountNodeInfo.createDstPath = j[FIELDS_CREATE_DEST_PATH].get<bool>();
    }
    if (j.contains(FIELDS_DEST_PATH_INFO) && j[FIELDS_DEST_PATH_INFO].is_object()) {
        mountNodeInfo.dstPathInfo = std::make_shared<DirInfo>();
        from_json(j[FIELDS_DEST_PATH_INFO], *mountNodeInfo.dstPathInfo);
    }

    std::string optionsStr;
    if (j.contains(FIELDS_OPTIONS) && j[FIELDS_OPTIONS].is_string()) {
        optionsStr = j[FIELDS_OPTIONS].get<std::string>();
    }
    mountNodeInfo.options = ParseKeyValuePairs(optionsStr, ',');
}

int32_t DirInfo::MakeDir() const
{
    return MakeDir(path);
}

int32_t DirInfo::MakeDir(const std::string &createPath) const
{
    if (!PrepareDir(createPath, mode, uid, gid)) {
        LOGE("prepareDir failed, path=%{public}s, errno=%{public}d", createPath.c_str(), errno);
        return E_PREPARE_DIR;
    }
    return E_OK;
}

int32_t DirInfo::RemoveDir() const
{
    if (!RmDirRecurse(path)) {
        LOGE("RmDirRecurse failed, path=%{public}s, errno=%{public}d", path.c_str(), errno);
        return E_DESTROY_DIR;
    }
    return E_OK;
}

void DirInfo::UpdateDirUid(int32_t userId)
{
    if (userId >= 0 && options.find(OPTIONS_UPDATE_UID) != options.end()) {
        uid = USER_ID_BASE * static_cast<uid_t>(userId) + uid;
    }
}

int32_t MountNodeInfo::MountDir() const
{
    return MountDir(srcPath, dstPath);
}

int32_t MountNodeInfo::MountDir(const std::string &src, const std::string &dst) const
{
    std::string currentDst = dst;
    if (createDstPath && dstPathInfo != nullptr) {
        dstPathInfo->MakeDir(currentDst);
    }

    if (!src.empty() && !IsDir(src)) {
        LOGE("path invalid, %{public}s", src.c_str());
        return E_NON_EXIST;
    }

    if (currentDst.empty() || !IsDir(currentDst)) {
        LOGE("path invalid, %{public}s", currentDst.c_str());
        return E_NON_EXIST;
    }

    if (options.find(OPTIONS_CHECK_MOUNTED) != options.end()) {
        if (IsPathMounted(currentDst)) {
            LOGI("path has mounted, %{public}s", currentDst.c_str());
            return E_OK;
        }
    }
    
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto ret = Mount(src, currentDst, fsType.empty() ? nullptr : fsType.c_str(),
        mountFlags, data.empty() ? nullptr : data.c_str());
    auto delay = StorageService::StorageRadar::ReportDuration("MountDir",
        startTime, StorageService::DEFAULT_DELAY_TIME_THRESH, StorageService::DEFAULT_USER_ID);
    LOGI("SD_DURATION: MountDir, delayTime = %{public}s", delay.c_str());
    if (ret != E_OK && errno != EEXIST && errno != EBUSY) {
        LOGE("mount failed, path=%{public}s, errno=%{public}d.", currentDst.c_str(), errno);
        return ret;
    }
    return E_OK;
}

int32_t UserPathResolver::GetUserBasePath(int32_t userId, uint32_t flags, std::vector<DirInfo> &dirInfoList)
{
    auto ret = GetUserPath(flags, JSON_KEY_USER_BASE, dirInfoList);
    if (ret != E_OK) {
        return ret;
    }
    return ReplaceUserId(userId, dirInfoList);
}

int32_t UserPathResolver::GetUserServicePath(int32_t userId, uint32_t flags, std::vector<DirInfo> &dirInfoList)
{
    auto ret = GetUserPath(flags, JSON_KEY_USER_SERVICE, dirInfoList);
    if (ret != E_OK) {
        return ret;
    }
    return ReplaceUserId(userId, dirInfoList);
}

int32_t UserPathResolver::GetAppdataPath(int32_t userId, std::vector<DirInfo> &dirInfoList)
{
    nlohmann::json j;
    auto ret = OpenJsonFile(STORAGE_USER_PATH, j);
    if (ret != E_OK) {
        return ret;
    }

    ret = GetTListFromJson<DirInfo>(j, {JSON_KEY_APPDATA}, dirInfoList);
    if (ret != E_OK) {
        return ret;
    }
    return ReplaceUserId(userId, dirInfoList);
}

int32_t UserPathResolver::GetVirtualPath(int32_t userId, std::vector<DirInfo> &dirInfoList)
{
    nlohmann::json j;
    auto ret = OpenJsonFile(STORAGE_USER_PATH, j);
    if (ret != E_OK) {
        return ret;
    }

    ret = GetTListFromJson<DirInfo>(j, {JSON_KEY_VIRTUAL}, dirInfoList);
    if (ret != E_OK) {
        return ret;
    }
    return ReplaceUserId(userId, dirInfoList);
}

int32_t UserPathResolver::GetAppDataMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList)
{
    return GetMountNodeList(userId, {JSON_KEY_APPDATA_MOUNT}, mountNodeList);
}

int32_t UserPathResolver::GetHmdfsMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList)
{
    return GetMountNodeList(userId, {JSON_KEY_HMDFS_MOUNT}, mountNodeList);
}

int32_t UserPathResolver::GetSandboxMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList)
{
    return GetMountNodeList(userId, {JSON_KEY_SANDBOX_MOUNT}, mountNodeList);
}

int32_t UserPathResolver::GetMountNodeList(int32_t userId, const std::vector<std::string> &pathKeys,
    std::vector<MountNodeInfo> &mountNodeList)
{
    nlohmann::json j;
    auto ret = OpenJsonFile(STORAGE_MOUNT_INFO, j);
    if (ret != E_OK) {
        return ret;
    }

    ret = GetTListFromJson<MountNodeInfo>(j, pathKeys, mountNodeList);
    if (ret != E_OK) {
        return ret;
    }
    return ReplaceUserId(userId, mountNodeList);
}

int32_t UserPathResolver::GetUserPath(uint32_t flags, const std::string &userType, std::vector<DirInfo> &dirInfoList)
{
    std::map<IStorageDaemonEnum, std::string> expectFlags {
        {CRYPTO_FLAG_EL1, EL1},
        {CRYPTO_FLAG_EL2, EL2},
        {CRYPTO_FLAG_EL3, EL3},
        {CRYPTO_FLAG_EL4, EL4},
        {CRYPTO_FLAG_EL5, EL5},
    };

    nlohmann::json j;
    auto ret = OpenJsonFile(STORAGE_USER_PATH, j);
    if (ret != E_OK) {
        return ret;
    }

    for (auto &expectFlag : expectFlags) {
        int32_t ret = E_OK;
        if (flags & expectFlag.first) {
            ret = GetTListFromJson<DirInfo>(j, {userType, expectFlag.second}, dirInfoList);
        }
        if (ret != E_OK) {
            return ret;
        }
    }
    return E_OK;
}

int32_t UserPathResolver::OpenJsonFile(const std::string &filename, nlohmann::json &j)
{
    std::string filePath = STORAGE_ETC_PATH + filename;
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0 ||
        fileStat.st_size <= 0 || fileStat.st_size > MAX_JSON_FILE_LEN) {
        return E_OPEN_JSON_FILE_ERROR;
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGE("json file: %{public}s open failed, error=%{public}d", filename.c_str(), errno);
        return E_OPEN_JSON_FILE_ERROR;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    if (content.empty() || !nlohmann::json::accept(content)) {
        LOGE("parseFromJson: jsonStr is empty or invalid");
        return E_JSON_PARSE_ERROR;
    }

    j = nlohmann::json::parse(content, nullptr, false);
    return E_OK;
}

template <typename T>
int32_t UserPathResolver::GetTListFromJson(const nlohmann::json &j, const std::vector<std::string> &pathKeys,
    std::vector<T> &tList)
{
    if (j.is_discarded()) {
        LOGE("parseFromJson: jsonStr is discarded");
        return E_JSON_PARSE_ERROR;
    }

    const nlohmann::json* current = &j;
    for (const auto &key : pathKeys) {
        if (!(*current).contains(key)) {
            LOGE("json parse error, key=%{public}s not exist", key.c_str());
            return E_JSON_PARSE_ERROR;
        }
        if (!(*current).at(key).is_object() && !(*current).at(key).is_array()) {
            LOGE("json parse error, key=%{public}s invalid", key.c_str());
            return E_JSON_PARSE_ERROR;
        }
        current = &(*current).at(key);
    }

    if (!(*current).is_array()) {
        LOGE("json parse error, not array");
        return E_JSON_PARSE_ERROR;
    }

    for (const auto &item : *current) {
        if (!item.is_object()) {
            LOGE("json parse error, item not object");
            return E_JSON_PARSE_ERROR;
        }
        tList.emplace_back(item.get<T>());
    }
    return E_OK;
}

template int32_t UserPathResolver::GetTListFromJson<DirInfo>(const nlohmann::json&,
    const std::vector<std::string>&, std::vector<DirInfo>&);

template int32_t UserPathResolver::GetTListFromJson<MountNodeInfo>(const nlohmann::json&,
    const std::vector<std::string>&, std::vector<MountNodeInfo>&);

int32_t UserPathResolver::ReplaceUserId(int32_t userId, std::vector<DirInfo> &dirInfoList)
{
    for (auto &dirInfo : dirInfoList) {
        ReplaceAndCount(dirInfo.path, USER_ID, std::to_string(userId));
    }
    return E_OK;
}

int32_t UserPathResolver::ReplaceUserId(int32_t userId, std::vector<MountNodeInfo> &mountNodeList)
{
    for (auto &mountNode : mountNodeList) {
        ReplaceAndCount(mountNode.srcPath, USER_ID, std::to_string(userId));
        ReplaceAndCount(mountNode.dstPath, USER_ID, std::to_string(userId));
        ReplaceAndCount(mountNode.data, USER_ID, std::to_string(userId));
    }
    return E_OK;
}
} // namespace StorageDaemon
} // namespace OHOS
