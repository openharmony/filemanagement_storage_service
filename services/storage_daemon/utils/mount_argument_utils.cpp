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

#include "utils/mount_argument_utils.h"

#include <sstream>
#include <sys/mount.h>
#include <sys/stat.h>

#include "storage_service_log.h"

namespace OHOS {
namespace StorageDaemon {
namespace Utils {
using namespace std;
namespace {
    static const std::string DATA_POINT = "/data/service/el2/";
    static const std::string BASE_MOUNT_POINT = "/mnt/hmdfs/";
    static const std::string SYSFS_HMDFS_PATH = "/sys/fs/hmdfs/";
    static const std::string COMM_DATA_POINT = "/storage/media/";
    static const std::string COMM_CLOUD_POINT = "/storage/cloud/";
    static const std::string RELATIVE_DOCS_PATH = "/files/Docs";
    static const std::string SHAREFS_DATA_POINT = "/data/service/el2/";
    static const std::string SHAREFS_BASE_MOUNT_POINT = "/mnt/share/";
    static const std::string TMPFS_MNT_DATA = "/mnt/data/";
    static const std::string HMDFS_DEVICE_VIEW_LOCAL_DOCS_PATH = "/device_view/local" + RELATIVE_DOCS_PATH;
    static const std::string SANDBOX_PATH = "/mnt/sandbox/";
    static const std::string MNT_USER_PATH = "/mnt/user/";
    static const std::string NOSHAREFS_DOC_PATH = "/nosharefs/docs";
    static const std::string SHAREFS_DOC_PATH = "/sharefs/docs";
    static const std::string NOSHAREFS_DOC_CUR_PATH = "/nosharefs/docs/currentUser";
    static const std::string SHAREFS_DOC_CUR_PATH = "/sharefs/docs/currentUser";
    static const std::string LOCAL_FILE_DOCS_PATH = "/local/files/Docs";
    static const std::string CUR_OTHER_PATH = "/currentUser/other";
    static const std::string CUR_OTHER_APPDATA_PATH = "/currentUser/other/appdata";
    static const std::string CUR_FILEMGR_PATH = "/currentUser/filemgr";
    static const std::string CUR_FILEMGR_APPDATA_PATH = "/currentUser/filemgr/appdata";
    static const std::string NOSHAREFS_APPDATA_PATH = "/nosharefs/appdata";
} // namespace

string MountArgument::GetFullSrc() const
{
    stringstream ss;
    ss << DATA_POINT << userId_ << "/hmdfs/" << relativePath_;

    return ss.str();
}

string MountArgument::GetFullDst() const
{
    stringstream ss;
    ss << BASE_MOUNT_POINT << userId_ << "/" << relativePath_;

    return ss.str();
}

string MountArgument::GetFullMediaCloud() const
{
    stringstream ss;
    ss << TMPFS_MNT_DATA << userId_ << "/" << "cloud";

    return ss.str();
}

string MountArgument::GetFullCloud() const
{
    stringstream ss;
    ss << TMPFS_MNT_DATA << userId_ << "/" << "cloud_fuse";

    return ss.str();
}

string MountArgument::GetShareSrc() const
{
    stringstream ss;
    ss << SHAREFS_DATA_POINT << userId_ << "/share";

    return ss.str();
}

string MountArgument::GetUserIdPara() const
{
    stringstream ss;
    ss << "user_id=" << userId_;

    return ss.str();
}

string MountArgument::GetHmUserIdPara() const
{
    stringstream ss;
    ss << "override_support_delete,user_id=" << userId_;
    return ss.str();
}

string MountArgument::GetShareDst() const
{
    stringstream ss;
    ss << SHAREFS_BASE_MOUNT_POINT << userId_;

    return ss.str();
}

string MountArgument::GetCommFullPath() const
{
    stringstream ss;
    ss << COMM_DATA_POINT << userId_ << "/";

    return ss.str();
}

string MountArgument::GetCloudFullPath() const
{
    stringstream ss;
    ss << COMM_CLOUD_POINT << userId_ << "/";

    return ss.str();
}

string MountArgument::GetCloudDocsPath() const
{
    stringstream ss;
    ss << COMM_CLOUD_POINT << userId_ << RELATIVE_DOCS_PATH;

    return ss.str();
}

string MountArgument::GetLocalDocsPath() const
{
    stringstream ss;
    ss << BASE_MOUNT_POINT << userId_ << "/" << relativePath_ << HMDFS_DEVICE_VIEW_LOCAL_DOCS_PATH;

    return ss.str();
}

string MountArgument::GetCachePath() const
{
    stringstream ss;
    if (enableCloudDisk_) {
        ss << DATA_POINT << userId_ << "/hmdfs/cloud/";
    } else {
        ss << DATA_POINT << userId_ << "/hmdfs/cache/" << relativePath_ << "_cache/";
    }
    return ss.str();
}

static uint64_t MocklispHash(const string &str)
{
    struct stat statBuf;
    auto err = stat(str.c_str(), &statBuf);
    if (err != 0) {
        LOGE("stat failed err: %{public}d", err);
    }
    LOGI("statBuf dev id: %{public}lu", static_cast<unsigned long>(statBuf.st_dev));
    return statBuf.st_dev;
}

string MountArgument::GetCtrlPath() const
{
    auto dst = GetFullDst();
    auto res = MocklispHash(dst);

    stringstream ss;
    ss << SYSFS_HMDFS_PATH << res << "/cmd";
    return ss.str();
}

string MountArgument::GetMountPointPrefix() const
{
    stringstream ss;
    ss << DATA_POINT << userId_ << "/hmdfs";
    return ss.str();
}

std::string MountArgument::GetSandboxPath() const
{
    stringstream ss;
    ss << SANDBOX_PATH << userId_;
    return ss.str();
}

string MountArgument::OptionsToString() const
{
    stringstream ss;
    ss << "local_dst=" << GetFullDst() << ",user_id=" << userId_;
    ss << ",ra_pages=512";
    if (useCache_) {
        ss << ",cache_dir=" << GetCachePath();
    }
    if (useCloudDir_) {
        ss << ",cloud_dir=" << GetFullMediaCloud();
    }
    if (caseSensitive_) {
        ss << ",sensitive";
    }
    if (enableMergeView_) {
        ss << ",merge";
    }
    if (enableCloudDisk_) {
        ss << ",cloud_disk";
    }
    if (!enableOfflineStash_) {
        ss << ",no_offline_stash";
    }
    return ss.str();
}

string MountArgument::GetMediaDocsPath() const
{
    stringstream ss;
    ss << COMM_DATA_POINT << userId_ << LOCAL_FILE_DOCS_PATH;
    return ss.str();
}

string MountArgument::GetNoSharefsAppdataPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << NOSHAREFS_APPDATA_PATH;
    return ss.str();
}

string MountArgument::GetNoSharefsDocPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << NOSHAREFS_DOC_PATH;
    return ss.str();
}

string MountArgument::GetNoSharefsDocCurPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << NOSHAREFS_DOC_CUR_PATH;
    return ss.str();
}

string MountArgument::GetSharefsDocPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << SHAREFS_DOC_PATH;
    return ss.str();
}

string MountArgument::GetSharefsDocCurPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << SHAREFS_DOC_CUR_PATH;
    return ss.str();
}

string MountArgument::GetCurOtherAppdataPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << CUR_OTHER_APPDATA_PATH;
    return ss.str();
}

string MountArgument::GetMntUserPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_;
    return ss.str();
}

string MountArgument::GetCurOtherPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << CUR_OTHER_PATH;
    return ss.str();
}

string MountArgument::GetCurFileMgrPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << CUR_FILEMGR_PATH;
    return ss.str();
}

string MountArgument::GetCurFileMgrAppdataPath() const
{
    stringstream ss;
    ss << MNT_USER_PATH << userId_ << CUR_FILEMGR_APPDATA_PATH;
    return ss.str();
}

unsigned long MountArgument::GetFlags() const
{
    return MS_NODEV;
}

MountArgument MountArgumentDescriptors::Alpha(int userId, string relativePath)
{
    MountArgument mountArgument = {
        .userId_ = userId,
        .needInitDir_ = true,
        .useCache_ = true,
        .useCloudDir_ = true,
        .enableMergeView_ = true,
        .enableCloudDisk_ = false,
        .enableFixupOwnerShip_ = false,
        .enableOfflineStash_ = true,
        .relativePath_ = relativePath,
    };
    return mountArgument;
}
} // namespace Utils
} // namespace StorageDaemon
} // namespace OHOS
