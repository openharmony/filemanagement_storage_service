/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "utils/storage_utils.h"

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "storage/bundle_manager_connector.h"

namespace OHOS {
namespace StorageManager {
const std::string FILEMGR_BUNDLE_NAME = "com.ohos.filemanager";
int64_t GetRoundSize(int64_t size)
{
    int64_t val = 1;
    int64_t multple = UNIT;
    while (val * multple < size) {
        auto tmpVal = static_cast<uint64_t>(val);
        tmpVal <<= 1;
        val = static_cast<int64_t>(tmpVal);
        if (val > THRESHOLD && multple < ONE_GB) {
            val = 1;
            multple *= UNIT;
        }
    }
    return val * multple;
}

std::string GetAnonyString(const std::string &value)
{
    constexpr size_t INT32_SHORT_ID_LENGTH = 20;
    constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
    constexpr size_t INT32_MIN_ID_LENGTH = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

bool IsPathStartWithFileMgr(int32_t userId, const std::string &path)
{
    const std::string prefix = "/mnt/data/" + std::to_string(userId) + "/userExternal/";
    if (path.size() <= prefix.size()) {
        return false;
    }
    return path.compare(0, prefix.length(), prefix) == 0;
}

bool IsCalledByFileMgr()
{
    int32_t uid = IPCSkeleton::GetCallingUid();
    auto bundleMgr = BundleMgrConnector::GetInstance().GetBundleMgrProxy();
    if (bundleMgr == nullptr) {
        LOGE("Connect bundle manager sa proxy failed.");
        return false;
    }
    std::string bundleName;
    if (!bundleMgr->GetBundleNameForUid(uid, bundleName)) {
        LOGE("Invoke bundleMgr interface to get bundle name failed.");
        return false;
    }
    if (bundleName != FILEMGR_BUNDLE_NAME) {
        LOGE("permissionCheck error, caller is %{public}s(%{public}d)", bundleName.c_str(), uid);
        return false;
    }
    return true;
}
} // namespace STORAGE_Manager
} // namespace OHOS