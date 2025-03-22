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

#include "storage_daemon_client.h"

#include "if_system_ability_manager.h"
#include "ipc/istorage_daemon.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "storage_service_errno.h"
#include "../../ipc/test/storage_daemon_stub_mock.h"

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
using namespace StorageDaemon;

constexpr uint32_t STORAGE_DAEMON_SFIFT = 1;
constexpr uint32_t STORAGE_SERVICE_FLAG = (1 << STORAGE_DAEMON_SFIFT);

class StorageDaemonClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
public:
    static inline shared_ptr<SystemAbilityMock> sa = nullptr;
    static inline sptr<SystemAbilityManagerMock> sam = nullptr;
    static inline sptr<StorageDaemonStubMock> sd = nullptr;
};

void StorageDaemonClientTest::SetUpTestCase()
{
    sa = make_shared<SystemAbilityMock>();
    SystemAbilityMock::sab = sa;
    sam = sptr(new SystemAbilityManagerMock());
    sd = sptr(new StorageDaemonStubMock());
}

void StorageDaemonClientTest::TearDownTestCase()
{
    sam = nullptr;
    sd = nullptr;
    SystemAbilityMock::sab = nullptr;
    sa = nullptr;
}


/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_GetStorageDaemonProxy_001
 * @tc.desc: Verify the GetStorageDaemonProxy function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_GetStorageDaemonProxy_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetStorageDaemonProxy_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    auto ptr = StorageDaemonClient::GetStorageDaemonProxy();
    EXPECT_TRUE(ptr == nullptr);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(nullptr));
    ptr = StorageDaemonClient::GetStorageDaemonProxy();
    EXPECT_TRUE(ptr == nullptr);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    ptr = StorageDaemonClient::GetStorageDaemonProxy();
    EXPECT_TRUE(ptr != nullptr);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GetStorageDaemonProxy_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_CheckServiceStatus_001
 * @tc.desc: Verify the CheckServiceStatus function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_CheckServiceStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_CheckServiceStatus_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    auto ret = StorageDaemonClient::CheckServiceStatus(0);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(nullptr)).WillOnce(Return(sam));
    ret = StorageDaemonClient::CheckServiceStatus(0);
    EXPECT_EQ(ret, E_OK);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillRepeatedly(Return(nullptr));
    ret = StorageDaemonClient::CheckServiceStatus(0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    ret = StorageDaemonClient::CheckServiceStatus(STORAGE_SERVICE_FLAG);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillRepeatedly(Return(nullptr));
    ret = StorageDaemonClient::CheckServiceStatus(STORAGE_SERVICE_FLAG);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(nullptr))
        .WillOnce(Return(nullptr)).WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::CheckServiceStatus(STORAGE_SERVICE_FLAG);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_CheckServiceStatus_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001
 * @tc.desc: Verify the PrepareUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::PrepareUserDirs(0, 0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::PrepareUserDirs(0, 0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, PrepareUserDirs(_, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::PrepareUserDirs(0, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserDirs_001 end";
}

/**
 * @tc.name: Storage_Service_StorageDaemonClientTest_DestroyUserDirs_001
 * @tc.desc: Verify the DestroyUserDirs function.
 * @tc.type: FUNC
 * @tc.require: AR000GK4HB
 */
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_DestroyUserDirs_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DestroyUserDirs_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::DestroyUserDirs(0, 0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::DestroyUserDirs(0, 0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, DestroyUserDirs(_, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::DestroyUserDirs(0, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DestroyUserDirs_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_StartUser_001
* @tc.desc: Verify the StartUser function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_StartUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::StartUser(0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::StartUser(0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, StartUser(_)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::StartUser(0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StartUser_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_StopUser_001
* @tc.desc: Verify the StopUser function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_StopUser_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StopUser_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::StopUser(0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::StopUser(0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, StopUser(_)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::StopUser(0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_StopUser_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001
* @tc.desc: Verify the PrepareUserSpace function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 start";
    std::string volumId;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::PrepareUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::PrepareUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, PrepareUserDirs(_, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::PrepareUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_PrepareUserSpace_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_DestroyUserSpace_001
* @tc.desc: Verify the DestroyUserSpace function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_DestroyUserSpace_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DestroyUserSpace_001 start";
    std::string volumId;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::DestroyUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::DestroyUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, DestroyUserDirs(_, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::DestroyUserSpace(0, volumId, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DestroyUserSpace_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_InitGlobalKey_001
* @tc.desc: Verify the InitGlobalKey function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_InitGlobalKey_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_InitGlobalKey_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::InitGlobalKey();
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::InitGlobalKey();
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, InitGlobalKey()).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::InitGlobalKey();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_InitGlobalKey_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_InitGlobalUserKeys_001
* @tc.desc: Verify the InitGlobalUserKeys function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_InitGlobalUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_InitGlobalUserKeys_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::InitGlobalUserKeys();
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::InitGlobalUserKeys();
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, InitGlobalUserKeys()).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::InitGlobalUserKeys();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_InitGlobalUserKeys_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_GenerateUserKeys_001
* @tc.desc: Verify the GenerateUserKeys function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_GenerateUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GenerateUserKeys_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::GenerateUserKeys(0, 0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::GenerateUserKeys(0, 0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, GenerateUserKeys(_, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::GenerateUserKeys(0, 0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_GenerateUserKeys_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_DeleteUserKeys_001
* @tc.desc: Verify the DeleteUserKeys function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_DeleteUserKeys_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DeleteUserKeys_001 start";
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::DeleteUserKeys(0);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::DeleteUserKeys(0);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, DeleteUserKeys(_)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::DeleteUserKeys(0);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_DeleteUserKeys_001 end";
}

/**
* @tc.name: Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001
* @tc.desc: Verify the UpdateUserAuth function.
* @tc.type: FUNC
* @tc.require: AR000GK4HB
*/
HWTEST_F(StorageDaemonClientTest, Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 start";
    vector<uint8_t> token;
    vector<uint8_t> oldSecret;
    vector<uint8_t> newSecret;
    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>())).WillOnce(Return(sd));
    auto ret = StorageDaemonClient::UpdateUserAuth(0, 0, token, oldSecret, newSecret);
    EXPECT_EQ(ret, E_SERVICE_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(nullptr));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    ret = StorageDaemonClient::UpdateUserAuth(0, 0, token, oldSecret, newSecret);
    EXPECT_EQ(ret, E_SA_IS_NULLPTR);

    EXPECT_CALL(*sa, GetSystemAbilityManager()).WillOnce(Return(sam)).WillOnce(Return(sam));
    EXPECT_CALL(*sam, CheckSystemAbility(An<int32_t>(), An<bool&>()))
        .WillOnce(DoAll(SetArgReferee<1>(true), Return(sd)));
    EXPECT_CALL(*sam, GetSystemAbility(_)).WillOnce(Return(sd));
    EXPECT_CALL(*sd, UpdateUserAuth(_, _, _, _, _)).WillOnce(Return(E_OK));
    ret = StorageDaemonClient::UpdateUserAuth(0, 0, token, oldSecret, newSecret);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "Storage_Service_StorageDaemonClientTest_UpdateUserAuth_001 end";
}
}