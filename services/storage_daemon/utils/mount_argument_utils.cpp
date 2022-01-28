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

#include "utils/mount_argument_utils.h"

#include <sstream>
#include <sys/mount.h>

namespace OHOS {
namespace StorageDaemon {
namespace Utils {
using namespace std;
namespace {
static const std::string DATA_POINT = "/data/service/el2/";
static const std::string BASE_MOUNT_POINT = "/mnt/hmdfs/";
} // namespace

string MountArgument::GetFullSrc() const
{
    stringstream ss;
    if (sameAccount_ == true) {
        ss << DATA_POINT << userId_ << "/hmdfs/account/";
    } else {
        ss << DATA_POINT << userId_ << "/hmdfs/non_account/";
    }

    return ss.str();
}

string MountArgument::GetFullDst() const
{
    stringstream ss;
    if (sameAccount_ == true) {
        ss << BASE_MOUNT_POINT << userId_ << "/account/";
    } else {
        ss << BASE_MOUNT_POINT << userId_ << "/non_account/";
    }

    return ss.str();
}

string MountArgument::GetCachePath() const
{
    stringstream ss;
    if (sameAccount_ == true) {
        ss << DATA_POINT << userId_ << "/hmdfs/account/cache/";
    } else {
        ss << DATA_POINT << userId_ << "/hmdfs/non_account/cache/";
    }

    return ss.str();
}

string MountArgument::OptionsToString() const
{
    stringstream ss;
    ss << "local_dst=" << GetFullDst();
    if (useCache_) {
        ss << ",cache_dir=" << GetCachePath();
    }
    if (caseSensitive_) {
        ss << ",sensitive";
    }
    if (enableMergeView_) {
        ss << ",merge";
    }
    if (!enableOfflineStash_) {
        ss << ",no_offline_stash";
    }
    return ss.str();
}

unsigned long MountArgument::GetFlags() const
{
    return MS_NODEV;
}

MountArgument MountArgumentDescriptors::Alpha(int userId, bool sameAccount)
{
    MountArgument mountArgument = {
        .sameAccount_ = sameAccount,
        .userId_ = userId,
        .needInitDir_ = true,
        .useCache_ = true,
        .enableMergeView_ = true,
        .enableFixupOwnerShip_ = false,
        .enableOfflineStash_ = true,
        .externalFS_ = false,
    };
    return mountArgument;
}
} // namespace Utils
} // namespace StorageDaemon
} // namespace OHOS
