/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "storage_manager_ipc_interface_code.h"
#include "storage_manager_proxy.h"
#include "ipc/storage_manager_stub.h"
#include "storage_manager_stub_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

namespace {
    int32_t g_code[] = {
        static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_ADD_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::REMOVE_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::PREPARE_START_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::STOP_USER),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_CREATED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_MOUNTED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_VOLUME_STATE_CHANGED),
        static_cast<int32_t>(StorageManagerInterfaceCode::MOUNT),
        static_cast<int32_t>(StorageManagerInterfaceCode::UNMOUNT),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_CREATED),
        static_cast<int32_t>(StorageManagerInterfaceCode::NOTIFY_DISK_DESTROYED),
        static_cast<int32_t>(StorageManagerInterfaceCode::PARTITION),
        static_cast<int32_t>(StorageManagerInterfaceCode::CREATE_USER_KEYS),
        static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_USER_KEYS),
        static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_USER_AUTH),
        static_cast<int32_t>(StorageManagerInterfaceCode::ACTIVE_USER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::INACTIVE_USER_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::UPDATE_KEY_CONTEXT),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_UUID),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_VOL_BY_ID),
        static_cast<int32_t>(StorageManagerInterfaceCode::SET_VOL_DESC),
        static_cast<int32_t>(StorageManagerInterfaceCode::FORMAT),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_DISK_BY_ID),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_SYSTEM_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_TOTAL_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_FREE_SIZE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_BUNDLE_STATUS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_CURR_USER_STATS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_USER_STATS_BY_TYPE),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_VOLUMES),
        static_cast<int32_t>(StorageManagerInterfaceCode::GET_ALL_DISKS),
        static_cast<int32_t>(StorageManagerInterfaceCode::LOCK_USER_SCREEN),
        static_cast<int32_t>(StorageManagerInterfaceCode::UNLOCK_USER_SCREEN),
        static_cast<int32_t>(StorageManagerInterfaceCode::GENERATE_APP_KEY),
        static_cast<int32_t>(StorageManagerInterfaceCode::DELETE_APP_KEY),
    };

} // namespace

class StorageManagerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageManagerStubTest, Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 start";

    StorageManagerStubMock mock;

    for (auto c : g_code) {
        MessageParcel data;
        MessageParcel reply;
        MessageOption option(MessageOption::TF_SYNC);
        bool bRet = data.WriteInterfaceToken(StorageManagerProxy::GetDescriptor());
        EXPECT_TRUE(bRet) << "write token error";
        int32_t ret = mock.OnRemoteRequest(c, data, reply, option);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    }

    GTEST_LOG_(INFO) << "Storage_Manager_StorageManagerStubTest_OnRemoteRequest_001 end";
}
} // STORAGE_MANAGER
} // OHOS
