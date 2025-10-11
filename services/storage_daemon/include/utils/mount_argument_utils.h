/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef MOUNT_ARGUMENT_UTILS_H
#define MOUNT_ARGUMENT_UTILS_H

#include <string>

namespace OHOS {
namespace StorageDaemon {
namespace Utils {
struct MountArgument final {
    int userId_{ 0 };
    bool needInitDir_{ false };
    bool useCache_{ false };
    bool useCloudDir_ { false };
    bool caseSensitive_{ false };
    bool enableMergeView_{ false };
    bool enableCloudDisk_{ false };
    bool enableFixupOwnerShip_{ false };
    bool enableOfflineStash_{ true };
    bool isSecurityMode_{ false };
    std::string relativePath_;

    std::string GetFullSrc() const;
    std::string GetFullDst() const;
    std::string GetShareSrc() const;
    std::string GetShareDst() const;
    std::string GetUserIdPara() const;
    std::string GetHmUserIdPara() const;
    std::string GetCommFullPath() const;
    std::string GetCloudFullPath() const;
    std::string GetCachePath() const;
    std::string GetCtrlPath() const;
    std::string OptionsToString() const;
    std::string GetFullCloud() const;
    std::string GetFullMediaCloud() const;
    std::string GetCloudDocsPath() const;
    std::string GetLocalDocsPath() const;
    std::string GetMountPointPrefix() const;
    std::string GetSandboxPath() const;
    std::string GetMntUserPath() const;
    std::string GetMediaDocsPath() const;
    std::string GetNoSharefsDocPath() const;
    std::string GetNoSharefsDocCurPath() const;
    std::string GetSharefsDocPath() const;
    std::string GetSharefsDocCurPath() const;
    std::string GetCurOtherPath() const;
    std::string GetCurOtherAppdataPath() const;
    std::string GetCurFileMgrPath() const;
    std::string GetCurFileMgrAppdataPath() const;
    std::string GetNoSharefsAppdataPath() const;
    unsigned long GetFlags() const;
    std::string GetFullMediaFuse() const;
};

struct MountArgumentDescriptors final {
public:
    static MountArgument Alpha(int userId, std::string relativePath);
};
} // namespace Utils
} // namespace StorageDaemon
} // namespace OHOS
#endif // UTILS_MOUNT_ARGUMENT_H
