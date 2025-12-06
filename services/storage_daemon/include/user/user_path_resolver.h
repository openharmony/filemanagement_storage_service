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

#ifndef OHOS_STORAGE_DAEMON_USER_PATH_RESOLVER_H
#define OHOS_STORAGE_DAEMON_USER_PATH_RESOLVER_H

#include <string>
#include <vector>
#include <sys/stat.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <memory>

namespace OHOS {
namespace StorageDaemon {
struct DirInfo {
    std::string path;
    mode_t mode;
    uid_t uid = 0;
    gid_t gid = 0;

    std::unordered_map<std::string, std::string> options;

    int32_t MakeDir() const;
    int32_t MakeDir(const std::string &createPath) const;
    int32_t RemoveDir() const;
    void UpdateDirUid(int32_t userId);
};
void from_json(const nlohmann::json &j, DirInfo &dirInfo);

struct MountNodeInfo {
    std::string srcPath;
    std::string dstPath;
    std::string fsType;
    unsigned long mountFlags;
    std::string data;

    // 可选参数
    bool createDstPath = false;
    std::shared_ptr<DirInfo> dstPathInfo = nullptr;
    std::unordered_map<std::string, std::string> options;

    int32_t MountDir() const;
    int32_t MountDir(const std::string &src, const std::string &dst) const;
};
void from_json(const nlohmann::json &j, MountNodeInfo &mountNodeInfo);

template <typename T>
class InfoList final {
public:
    ~InfoList()
    {
        std::vector<T>().swap(data);
    }
    std::vector<T> data;
};

class UserPathResolver final {
public:
    static int32_t GetUserBasePath(int32_t userId, uint32_t flags, std::vector<DirInfo> &dirInfoList);
    static int32_t GetUserServicePath(int32_t userId, uint32_t flags, std::vector<DirInfo> &dirInfoList);
    static int32_t GetAppdataPath(int32_t userId, std::vector<DirInfo> &dirInfoList);
    static int32_t GetVirtualPath(int32_t userId, std::vector<DirInfo> &dirInfoList);
    static int32_t GetAppDataMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList);
    static int32_t GetHmdfsMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList);
    static int32_t GetSandboxMountNodeList(int32_t userId, std::vector<MountNodeInfo> &mountNodeList);

private:
    template <typename T>
    static int32_t GetTListFromJson(const nlohmann::json &j, const std::vector<std::string> &pathKeys,
        std::vector<T> &tList);
    static int32_t GetMountNodeList(int32_t userId, const std::vector<std::string> &pathKeys,
        std::vector<MountNodeInfo> &mountNodeList);
    static int32_t ReplaceUserId(int32_t userId, std::vector<DirInfo> &dirInfoList);
    static int32_t ReplaceUserId(int32_t userId, std::vector<MountNodeInfo> &mountNodeList);
    static int32_t GetUserPath(uint32_t flags, const std::string &userType, std::vector<DirInfo> &dirInfoList);
    static int32_t OpenJsonFile(const std::string &filename, nlohmann::json &j);
};

extern template int32_t UserPathResolver::GetTListFromJson<DirInfo>(const nlohmann::json&,
    const std::vector<std::string>&, std::vector<DirInfo>&);
extern template int32_t UserPathResolver::GetTListFromJson<MountNodeInfo>(const nlohmann::json&,
    const std::vector<std::string>&, std::vector<MountNodeInfo>&);
} // STORAGE_DAEMON
} // OHOS

#endif // OHOS_STORAGE_DAEMON_USER_PATH_RESOLVER_H
