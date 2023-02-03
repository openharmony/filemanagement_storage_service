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

#include <cstdio>
#include <gtest/gtest.h>

#include "storage_daemon_communication/storage_daemon_communication.h"
#include "user/multi_user_manager_service.h"
#include "storage_service_errno.h"
#include "parameters.h"

namespace {
using namespace std;
using namespace OHOS;
using namespace StorageManager;
const string FSCRYPT_POLICY_KEY = "fscrypt.policy.config";
bool g_fscryptEnable = false;
class MultiUserManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        std::string res = system::GetParameter(FSCRYPT_POLICY_KEY, "");
        if (!res.empty()) {
            g_fscryptEnable = true;
        }
    };
    static void TearDownTestCase() {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareAddUser_0000
 * @tc.name: User_manager_service_PrepareAddUser_0000
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareAddUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareAddUser_0000";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 121;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        result = service->PrepareAddUser(userId, flag);
    }
    EXPECT_EQ(result, 0);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareAddUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareAddUser_0001
 * @tc.name: User_manager_service_PrepareAddUser_0001
 * @tc.desc: Test function of PrepareAddUser interface for Parameters ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareAddUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareAddUser_0001";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = -1;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        result = service->PrepareAddUser(userId, flag);
    }
    EXPECT_NE(result, 0);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareAddUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareAddUser_0002
 * @tc.name: User_manager_service_PrepareAddUser_0002
 * @tc.desc: Test function of PrepareAddUser interface for Parameters ERROR which userId not in [101, 1099].
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareAddUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareAddUser_0002";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 10000;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        result = service->PrepareAddUser(userId, flag);
    }
    EXPECT_NE(result, 0);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareAddUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareAddUser_0003
 * @tc.name: User_manager_service_PrepareAddUser_0003
 * @tc.desc: Test function of PrepareAddUser interface for SUCCESS which Repeated add.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareAddUser_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareAddUser_0003";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 102;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->PrepareAddUser(userId, flag);
    }
    if (g_fscryptEnable) {
        EXPECT_EQ(result, -EEXIST);
    } else {
        EXPECT_EQ(result, E_OK);
    }
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareAddUser_0003";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_RemoveUser_0000
 * @tc.name: User_manager_service_RemoveUser_0000
 * @tc.desc: Test function of RemoveUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_RemoveUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_RemoveUser_0000";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 103;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->RemoveUser(userId, flag);
    }
    EXPECT_EQ(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_RemoveUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_RemoveUser_0001
 * @tc.name: User_manager_service_RemoveUser_0001
 * @tc.desc: Test function of RemoveUser interface for SUCCESS which remove userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_RemoveUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_RemoveUser_0001";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 104;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        result = service->RemoveUser(userId, flag);
    }
    if (g_fscryptEnable) {
        EXPECT_EQ(result, -EFAULT);
    } else {
        EXPECT_EQ(result, E_OK);
    }
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_RemoveUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_RemoveUser_0002
 * @tc.name: User_manager_service_RemoveUser_0002
 * @tc.desc: Test function of RemoveUser interface for Parameters ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_RemoveUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_RemoveUser_0002";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = -2;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->RemoveUser(userId, flag);
    }
    EXPECT_NE(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_RemoveUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareStartUser_0000
 * @tc.name: User_manager_service_PrepareStartUser_0000
 * @tc.desc: Test function of PrepareStartUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareStartUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareStartUser_0000";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 105;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->PrepareStartUser(userId);
    }
    EXPECT_EQ(result, 0);
    service->StopUser(userId);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareStartUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareStartUser_0001
 * @tc.name: User_manager_service_PrepareStartUser_0001
 * @tc.desc: Test function of PrepareStartUser interface for Logic ERROR which start userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareStartUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareStartUser_0001";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 106;
    int32_t result;
    if (service != nullptr) {
        result = service->PrepareStartUser(userId);
    }
    EXPECT_NE(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareStartUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_PrepareStartUser_0002
 * @tc.name: User_manager_service_PrepareStartUser_0002
 * @tc.desc: Test function of PrepareStartUser interface for Parameters ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_PrepareStartUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_PrepareStartUser_0002";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = -4;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->PrepareStartUser(userId);
    }
    EXPECT_NE(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_PrepareStartUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_StopUser_0000
 * @tc.name: User_manager_service_StopUser_0000
 * @tc.desc: Test function of StopUser interface for SUCCESS.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_StopUser_0000, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_StopUser_0000";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 108;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        service->PrepareStartUser(userId);
        result = service->StopUser(userId);
    }
    EXPECT_EQ(result, 0);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_StopUser_0000";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_StopUser_0001
 * @tc.name: User_manager_service_StopUser_0001
 * @tc.desc: Test function of StopUser interface for Logic ERROR which start userId not exist.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_StopUser_0001, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_StopUser_0001";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 109;
    int32_t result;
    if (service != nullptr) {
        result = service->StopUser(userId);
    }
    EXPECT_NE(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_StopUser_0001";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_StopUser_0002
 * @tc.name: User_manager_service_StopUser_0002
 * @tc.desc: Test function of StopUser interface for Parameters ERROR which userId<0.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_StopUser_0002, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_StopUser_0002";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = -6;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        service->PrepareStartUser(userId);
        result = service->StopUser(userId);
    }
    EXPECT_NE(result, 0);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_StopUser_0002";
}

/**
 * @tc.number: SUB_STORAGE_User_manager_service_StopUser_0003
 * @tc.name: User_manager_service_StopUser_0003
 * @tc.desc: Test function of StopUser interface for Logic ERROR which stop userId not start.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: AR000GK4HB
 */
HWTEST_F(MultiUserManagerServiceTest, User_manager_service_StopUser_0003, testing::ext::TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-begin User_manager_service_StopUser_0003";
    std::shared_ptr<MultiUserManagerService> service = DelayedSingleton<MultiUserManagerService>::GetInstance();
    int32_t userId = 110;
    uint32_t flag = 3;
    int32_t result;
    if (service != nullptr) {
        service->PrepareAddUser(userId, flag);
        result = service->StopUser(userId);
    }
    EXPECT_NE(result, 0);
    service->RemoveUser(userId, flag);
    GTEST_LOG_(INFO) << "MultiUserManagerServiceTest-end User_manager_service_StopUser_0003";
}
} // namespace
