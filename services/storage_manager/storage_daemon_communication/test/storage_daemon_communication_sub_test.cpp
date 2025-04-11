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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "storage_daemon_communication/storage_daemon_communication.h"

#include "if_system_ability_manager.h"
#include "storage_service_errno.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "storage_daemon_stub_mock.h"

namespace OHOS {
class ISystemAbilityBase {
public:
    virtual ~ISystemAbilityBase() = default;
    virtual sptr<ISystemAbilityManager> GetSystemAbilityManager() = 0;
public:
    static inline std::shared_ptr<ISystemAbilityBase> sab = nullptr;
};

class SystemAbilityMock : public ISystemAbilityBase {
public:
    MOCK_METHOD((sptr<ISystemAbilityManager>), GetSystemAbilityManager, ());
};

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    return ISystemAbilityBase::sab->GetSystemAbilityManager();
}

class SystemAbilityManagerMock : public ISystemAbilityManager {
public:
    MOCK_METHOD((sptr<IRemoteObject>), AsObject, ());
    MOCK_METHOD((std::vector<std::u16string>), ListSystemAbilities, (unsigned int));
    MOCK_METHOD((sptr<IRemoteObject>), GetSystemAbility, (int32_t));
    MOCK_METHOD((sptr<IRemoteObject>), CheckSystemAbility, (int32_t));
    MOCK_METHOD(int32_t, RemoveSystemAbility, (int32_t));
    MOCK_METHOD(int32_t, SubscribeSystemAbility, (int32_t, (const sptr<ISystemAbilityStatusChange>&)));
    MOCK_METHOD(int32_t, UnSubscribeSystemAbility, (int32_t, (const sptr<ISystemAbilityStatusChange>&)));
    MOCK_METHOD((sptr<IRemoteObject>), GetSystemAbility, (int32_t, const std::string&));
    MOCK_METHOD((sptr<IRemoteObject>), CheckSystemAbility, (int32_t, const std::string&));
    MOCK_METHOD(int32_t, AddOnDemandSystemAbilityInfo, (int32_t, const std::u16string&));
    MOCK_METHOD((sptr<IRemoteObject>), CheckSystemAbility, (int32_t, bool&));
    MOCK_METHOD(int32_t, AddSystemAbility, (int32_t, (const sptr<IRemoteObject>&), const SAExtraProp&));
    MOCK_METHOD(int32_t, AddSystemProcess, (const std::u16string&, (const sptr<IRemoteObject>&)));
    MOCK_METHOD((sptr<IRemoteObject>), LoadSystemAbility, (int32_t, int32_t));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t, (const sptr<ISystemAbilityLoadCallback>&)));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t, const std::string&, (const sptr<ISystemAbilityLoadCallback>&)));
    MOCK_METHOD(int32_t, UnloadSystemAbility, (int32_t));
    MOCK_METHOD(int32_t, CancelUnloadSystemAbility, (int32_t));
    MOCK_METHOD(int32_t, UnloadAllIdleSystemAbility, ());
    MOCK_METHOD(int32_t, GetSystemProcessInfo, (int32_t, SystemProcessInfo&));
    MOCK_METHOD(int32_t, GetRunningSystemProcess, (std::list<SystemProcessInfo>&));
    MOCK_METHOD(int32_t, SubscribeSystemProcess, (const sptr<ISystemProcessStatusChange>&));
    MOCK_METHOD(int32_t, SendStrategy, (int32_t, (std::vector<int32_t>&), int32_t, std::string&));
    MOCK_METHOD(int32_t, UnSubscribeSystemProcess, (const sptr<ISystemProcessStatusChange>&));
    MOCK_METHOD(int32_t, GetOnDemandReasonExtraData, (int64_t, MessageParcel&));
    MOCK_METHOD(int32_t, GetOnDemandPolicy, (int32_t, OnDemandPolicyType, (std::vector<SystemAbilityOnDemandEvent>&)));
    MOCK_METHOD(int32_t, UpdateOnDemandPolicy, (int32_t, OnDemandPolicyType,
        (const std::vector<SystemAbilityOnDemandEvent>&)));
    MOCK_METHOD(int32_t, GetOnDemandSystemAbilityIds, (std::vector<int32_t>&));
    MOCK_METHOD(int32_t, GetExtensionSaIds, (const std::string&, std::vector<int32_t>&));
    MOCK_METHOD(int32_t, GetExtensionRunningSaList, (const std::string&, (std::vector<sptr<IRemoteObject>>&)));
    MOCK_METHOD(int32_t, GetRunningSaExtensionInfoList, (const std::string&, (std::vector<SaExtensionInfo>&)));
    MOCK_METHOD(int32_t, GetCommonEventExtraDataIdlist, (int32_t, (std::vector<int64_t>&), const std::string&));
    MOCK_METHOD((sptr<IRemoteObject>), GetLocalAbilityManagerProxy, (int32_t));
};
}

namespace OHOS {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace StorageManager;
using namespace StorageDaemon;

class StorageDaemonCommunicationTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase();
    void SetUp() {};
    void TearDown() {};
public:
    static inline shared_ptr<StorageDaemonCommunication> sdCommunication = nullptr;
    static inline shared_ptr<SystemAbilityMock> sa = nullptr;
    static inline sptr<SystemAbilityManagerMock> sam = nullptr;
    static inline sptr<StorageDaemonStubMock> sd = nullptr;
};

void StorageDaemonCommunicationTest::SetUpTestCase()
{
    sdCommunication = make_shared<StorageDaemonCommunication>();
    sa = make_shared<SystemAbilityMock>();
    SystemAbilityMock::sab = sa;
    sam = sptr(new SystemAbilityManagerMock());
    sd = sptr(new StorageDaemonStubMock());
}

void StorageDaemonCommunicationTest::TearDownTestCase()
{
    sam = nullptr;
    sd = nullptr;
    SystemAbilityMock::sab = nullptr;
    sa = nullptr;
    sdCommunication->storageDaemon_ = nullptr;
    sdCommunication = nullptr;
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_Connect_0000
* @tc.name: Daemon_communication_Connect_0000
* @tc.desc: Test function of Connect interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Connect_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Connect_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Connect(), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Connect(), E_REMOTE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_EQ(sdCommunication->Connect(), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Connect_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_PrepareAddUser_0000
* @tc.name: Daemon_communication_PrepareAddUser_0000
* @tc.desc: Test function of PrepareAddUser interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_PrepareAddUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_PrepareAddUser_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->PrepareAddUser(0, 0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->PrepareAddUser(0, 0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, PrepareUserDirs(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->PrepareAddUser(0, 0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_PrepareAddUser_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_RemoveUser_0000
* @tc.name: Daemon_communication_RemoveUser_0000
* @tc.desc: Test function of RemoveUser interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_RemoveUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_RemoveUser_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->RemoveUser(0, 0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->RemoveUser(0, 0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, DestroyUserDirs(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->RemoveUser(0, 0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_RemoveUser_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_PrepareStartUser_0000
* @tc.name: Daemon_communication_PrepareStartUser_0000
* @tc.desc: Test function of PrepareStartUser interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_PrepareStartUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_PrepareStartUser_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->PrepareStartUser(0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->PrepareStartUser(0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, StartUser(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->PrepareStartUser(0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_PrepareStartUser_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_StopUser_0000
* @tc.name: Daemon_communication_StopUser_0000
* @tc.desc: Test function of StopUser interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_StopUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_StopUser_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->StopUser(0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->StopUser(0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, StopUser(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->StopUser(0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_StopUser_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_CompleteAddUser_0000
* @tc.name: Daemon_communication_CompleteAddUser_0000
* @tc.desc: Test function of CompleteAddUser interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_CompleteAddUser_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_CompleteAddUser_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->CompleteAddUser(0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->CompleteAddUser(0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, CompleteAddUser(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->CompleteAddUser(0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_CompleteAddUser_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_Mount_0000
* @tc.name: Daemon_communication_Mount_0000
* @tc.desc: Test function of Mount interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Mount_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Mount_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Mount("", 0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->Mount("", 0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, Mount(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->Mount("", 0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Mount_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_Unmount_0000
* @tc.name: Daemon_communication_Unmount_0000
* @tc.desc: Test function of Unmount interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Unmount_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Unmount_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Unmount(""), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->Unmount(""), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, UMount(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->Unmount(""), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Unmount_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_Check_0000
* @tc.name: Daemon_communication_Check_0000
* @tc.desc: Test function of Check interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Check_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Check_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Check(""), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->Check(""), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, Check(_)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->Check(""), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Check_0000";
}

/**
* @tc.number: SUB_STORAGE_Daemon_communication_Partition_0000
* @tc.name: Daemon_communication_Partition_0000
* @tc.desc: Test function of Partition interface for SUCCESS.
* @tc.size: MEDIUM
* @tc.type: FUNC
* @tc.level Level 1
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonCommunicationTest, Daemon_communication_Partition_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-begin Daemon_communication_Partition_0000";
    ASSERT_TRUE(sdCommunication != nullptr);

    sdCommunication->storageDaemon_ = nullptr;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    EXPECT_EQ(sdCommunication->Partition("", 0), E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(DoAll(Invoke([sdCommunication {sdCommunication}] () {
        sdCommunication->storageDaemon_ = nullptr;
    }), Return(true)));
    EXPECT_EQ(sdCommunication->Partition("", 0), E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, AddDeathRecipient(_)).WillOnce(Return(true));
    EXPECT_CALL(*sd, Partition(_, _)).WillOnce(Return(E_OK));
    EXPECT_EQ(sdCommunication->Partition("", 0), E_OK);
    GTEST_LOG_(INFO) << "StorageDaemonCommunicationTest-end Daemon_communication_Partition_0000";
}
}