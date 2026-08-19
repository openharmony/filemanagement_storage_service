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

#ifndef OHOS_STORAGE_DAEMON_DM_DEVICE_MOCK_H
#define OHOS_STORAGE_DAEMON_DM_DEVICE_MOCK_H

#include <cstdint>

namespace OHOS {
namespace StorageDaemon {

struct MockConfig {
    bool mockEnabled = false;
    bool openControlFail = false;
    bool openSourceFail = false;
    uint64_t deviceBytes = 1024ULL * 1024 * 1024;
    bool blkGetSize64Fail = false;
    bool statusFail = true;
    bool createFail = false;
    bool loadTableFail = false;
    bool resumeFail = false;
    bool removeFail = false;
    bool memsetSFail = false;
    int strncpySFailAt = -1;      // 仅第 N 次 strncpy_s 调用失败（-1=不失败）
    int strcpySFailAt = -1;       // 同上
    bool dmRemoveCalled = false;
    int closeCount = 0;

    // 递增内部计数并判断当前调用是否应失败
    bool ShouldStrncpySFail()
    {
        strncpySCount_++;
        return strncpySFailAt >= 0 && strncpySCount_ == strncpySFailAt;
    }

    bool ShouldStrcpySFail()
    {
        strcpySCount_++;
        return strcpySFailAt >= 0 && strcpySCount_ == strcpySFailAt;
    }

    void SetUp(bool enableMock = false)
    {
        *this = MockConfig();
        mockEnabled = enableMock;
    }

    void TearDown()
    {
        *this = MockConfig();
    }

private:
    int strncpySCount_ = 0;
    int strcpySCount_ = 0;
};

extern MockConfig g_mock;

}  // namespace StorageDaemon
}  // namespace OHOS

#endif  // OHOS_STORAGE_DAEMON_DM_DEVICE_MOCK_H
