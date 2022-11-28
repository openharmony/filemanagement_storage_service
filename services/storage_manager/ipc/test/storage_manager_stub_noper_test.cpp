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

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ipc/storage_manager_proxy.h"
#include "ipc/storage_manager_stub.h"
#include "storage_manager_stub_mock.h"

#include "storage_service_errno.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
using namespace testing::ext;

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

    int code[] = {
        IStorageManager::PREPARE_ADD_USER,
        IStorageManager::REMOVE_USER,
        IStorageManager::PREPARE_START_USER,
        IStorageManager::STOP_USER,
        IStorageManager::NOTIFY_VOLUME_CREATED,
        IStorageManager::NOTIFY_VOLUME_MOUNTED,
        IStorageManager::NOTIFY_VOLUME_DESTROYED,
        IStorageManager::MOUNT,
        IStorageManager::UNMOUNT,
        IStorageManager::NOTIFY_DISK_CREATED,
        IStorageManager::NOTIFY_DISK_DESTROYED,
        IStorageManager::PARTITION,
        IStorageManager::CREATE_USER_KEYS,
        IStorageManager::DELETE_USER_KEYS,
        IStorageManager::UPDATE_USER_AUTH,
        IStorageManager::ACTIVE_USER_KEY,
        IStorageManager::INACTIVE_USER_KEY,
        IStorageManager::UPDATE_KEY_CONTEXT,
        IStorageManager::GET_VOL_BY_UUID,
        IStorageManager::GET_VOL_BY_ID,
        IStorageManager::SET_VOL_DESC,
        IStorageManager::FORMAT,
        IStorageManager::GET_DISK_BY_ID,
        IStorageManager::GET_TOTAL,
        IStorageManager::GET_FREE,
        IStorageManager::GET_SYSTEM_SIZE,
        IStorageManager::GET_TOTAL_SIZE,
        IStorageManager::GET_FREE_SIZE,
        IStorageManager::GET_BUNDLE_STATUS,
        IStorageManager::GET_CURR_USER_STATS,
        IStorageManager::GET_USER_STATS,
        IStorageManager::GET_ALL_VOLUMES,
        IStorageManager::GET_ALL_DISKS,
    };

    StorageManagerStubMock mock;

    for (auto c : code) {
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
