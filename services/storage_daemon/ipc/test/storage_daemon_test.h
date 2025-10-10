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
#ifndef STORAGE_DAEMON_TEST_H
#define STORAGE_DAEMON_TEST_H

#include "ipc/storage_daemon.h"

#include <gtest/gtest.h>

#include "anco_file_util_mock.h"
#include "file_utils_mock.h"
#include "iam_client_mock.h"
#include "key_manager_ext_mock.h"
#include "key_manager_mock.h"
#include "mount_manager_mock.h"
#include "user_manager_mock.h"

namespace OHOS {
namespace StorageDaemon {
namespace Test {
class StorageDaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    
    static void PrepareConfigDir();
    static void DestroyConfigDir();
    void RefreshConfigDir();
    void CreateNeedRestoreFile(int32_t userId, KeyType type);
    void DeleteNeedRestoreFile(int32_t userId, KeyType type);
    std::string GetUserRootPath(KeyType type);
    
    static const int32_t userId_ = 115;
    std::vector<uint8_t> token_;
    std::vector<uint8_t> secret_;

    std::shared_ptr<AncoFileUtilMoc> ancoMock_ = nullptr;
    std::shared_ptr<KeyManagerMock> keyManagerMock_ = nullptr;
    std::shared_ptr<KeyManagerExtMock> keyManagerExtMock_ = nullptr;
    std::shared_ptr<UserManagerMock> userManagerMock_ = nullptr;
    std::shared_ptr<FileUtilMoc> fileUtilMoc_ = nullptr;
    std::shared_ptr<MountManagerMoc> mountManagerMock_ = nullptr;
    std::shared_ptr<IamClientMoc> iamClientMock_ = nullptr;
    std::shared_ptr<StorageDaemon> storageDaemon_ = nullptr;
};
} // Test
} // STORAGE_DAEMON
} // OHOS
#endif /* STORAGE_DAEMON_TEST_H */
