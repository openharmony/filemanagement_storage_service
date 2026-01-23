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

#include "bundle_manager_adapter_proxy.h"
#include "bundle_mgr_interface_mock.h"
#include "message_parcel_mock.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::StorageManager::Test {
using namespace OHOS::StorageManager;
using namespace testing;
using namespace testing::ext;
using namespace std;

class BundleManagerAdapterProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline shared_ptr<BundleManagerAdapterProxy> proxy_ = nullptr;
    static inline sptr<BundleMgrMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void BundleManagerAdapterProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void BundleManagerAdapterProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void BundleManagerAdapterProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    mock_ = sptr(new BundleMgrMock());
    proxy_ = make_shared<BundleManagerAdapterProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void BundleManagerAdapterProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    mock_ = nullptr;
    proxy_ = nullptr;
    messageParcelMock_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetBundleNameForUid_0000
 * @tc.desc: The execution of the ConnectDfs failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetBundleNameForUid_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleNameForUid_0000 Start";

    string bundleName = "default-bundleName";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetBundleNameForUid(0, bundleName);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleNameForUid(0, bundleName);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetBundleNameForUid(0, bundleName);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(false));
    ret = proxy_->GetBundleNameForUid(0, bundleName);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(true));
    ret = proxy_->GetBundleNameForUid(0, bundleName);
    EXPECT_EQ(ret, true);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_ConnectDfs_0100 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_CleanBundleCacheFilesAutomatic_0000
 * @tc.desc: The execution of the ConnectDfs failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_CleanBundleCacheFilesAutomatic_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_CleanBundleCacheFilesAutomatic_0000 Start";

    auto ret = proxy_->CleanBundleCacheFilesAutomatic(0);
    EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_INVALID_PARAMETER);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    ret = proxy_->CleanBundleCacheFilesAutomatic(100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(false));
    ret = proxy_->CleanBundleCacheFilesAutomatic(100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->CleanBundleCacheFilesAutomatic(100);
    EXPECT_EQ(ret, ERR_BUNDLE_MANAGER_IPC_TRANSACTION);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0));
    ret = proxy_->CleanBundleCacheFilesAutomatic(100);
    EXPECT_EQ(ret, 0);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_ConnectDfs_0100 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetBundleInfosV9_0000
 * @tc.desc: The execution of the ConnectDfs failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetBundleInfosV9_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleInfosV9_0000 Start";

    std::vector<BundleInfo> vector = {};

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0)).WillOnce(Return(0));
    ret = proxy_->GetBundleInfosV9(0, vector, 100);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_ConnectDfs_0100 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetNameAndIndexForUid_0000
 * @tc.desc: Test GetNameAndIndexForUid function with various scenarios
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetNameAndIndexForUid_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetNameAndIndexForUid_0000 Start";

    std::string bundleName;
    int32_t appIndex = 0;

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetNameAndIndexForUid(1000, bundleName, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetNameAndIndexForUid(1000, bundleName, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetNameAndIndexForUid(1000, bundleName, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(ERR_APPEXECFWK_SERVICE_NOT_READY));
    ret = proxy_->GetNameAndIndexForUid(1000, bundleName, appIndex);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SERVICE_NOT_READY);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(ERR_OK)).WillOnce(Return(1));
    EXPECT_CALL(*messageParcelMock_, ReadString()).WillOnce(Return("com.example.app"));
    ret = proxy_->GetNameAndIndexForUid(1000, bundleName, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(bundleName, "com.example.app");
    EXPECT_EQ(appIndex, 1);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetNameAndIndexForUid_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetBundleStats_0000
 * @tc.desc: Test GetBundleStats function with various scenarios
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetBundleStats_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleStats_0000 Start";

    std::vector<int64_t> bundleStats;
    std::string bundleName = "com.example.app";

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleStats_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetBundleStats_0001
 * @tc.desc: Test GetBundleStats function with various scenarios
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetBundleStats_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleStats_0001 Start";

    std::vector<int64_t> bundleStats;
    std::string bundleName = "com.example.app";

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(false));
    auto ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt64Vector(_)).WillOnce(Return(false));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt64Vector(_)).WillOnce(Return(true));
    ret = proxy_->GetBundleStats(bundleName, 100, bundleStats, 0, 1);
    EXPECT_EQ(ret, true);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetBundleStats_0001 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_CreateBundleDataDirWithEl_0000
 * @tc.desc: Test CreateBundleDataDirWithEl function with various scenarios
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_CreateBundleDataDirWithEl_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_CreateBundleDataDirWithEl_0000 Start";

    DataDirEl dirEl = DataDirEl::EL1;

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint8(_)).WillOnce(Return(false));
    ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint8(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint8(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(ERR_APPEXECFWK_SERVICE_NOT_READY));
    ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_APPEXECFWK_SERVICE_NOT_READY);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint8(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(ERR_OK));
    ret = proxy_->CreateBundleDataDirWithEl(100, dirEl);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_CreateBundleDataDirWithEl_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetAllBundleStats_0000
 * @tc.desc: Test GetAllBundleStats function with various scenarios
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetAllBundleStats_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetAllBundleStats_0000 Start";

    std::vector<int64_t> bundleStats;

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(ERR_APPEXECFWK_PARCEL_ERROR));
    ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(false));
    ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, false);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt64Vector(_)).WillOnce(Return(false));
    ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, false);

    std::vector<int64_t> expectedStats = {1024, 2048, 4096, 8192, 16384};
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(NO_ERROR));
    EXPECT_CALL(*messageParcelMock_, ReadBool()).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt64Vector(_)).WillOnce(DoAll(
        SetArgPointee<0>(expectedStats), Return(true)));
    ret = proxy_->GetAllBundleStats(100, bundleStats);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(bundleStats.size(), 5);
    EXPECT_EQ(bundleStats[0], 1024);
    EXPECT_EQ(bundleStats[1], 2048);
    EXPECT_EQ(bundleStats[2], 4096);
    EXPECT_EQ(bundleStats[3], 8192);
    EXPECT_EQ(bundleStats[4], 16384);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetAllBundleStats_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0000
 * @tc.desc: Test InnerGetVectorFromParcelIntelligent with zero data size
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, InnerGetVectorFromParcelIntelligent_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0000 Start";

    MessageParcel reply;
    std::vector<BundleInfo> parcelableInfos;

    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(0));

    auto ret = proxy_->InnerGetVectorFromParcelIntelligent(reply, parcelableInfos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(parcelableInfos.empty());

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0001
 * @tc.desc: Test InnerGetVectorFromParcelIntelligent with small data size
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, InnerGetVectorFromParcelIntelligent_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0001 Start";

    MessageParcel reply;
    std::vector<BundleInfo> parcelableInfos;
    const size_t dataSize = 1024; // Small data size

    // Simulate small data case
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(nullptr)); // Return null to test error path

    auto ret = proxy_->InnerGetVectorFromParcelIntelligent(reply, parcelableInfos);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0001 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0002
 * @tc.desc: Test InnerGetVectorFromParcelIntelligent with large data size using ashmem
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, InnerGetVectorFromParcelIntelligent_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0002 Start";

    MessageParcel reply;
    std::vector<BundleInfo> parcelableInfos;
    const size_t largeDataSize = MessageParcel::MAX_RAWDATA_SIZE + 1; // Force ashmem path

    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(largeDataSize));

    auto ret = proxy_->InnerGetVectorFromParcelIntelligent(reply, parcelableInfos);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0002 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0003
 * @tc.desc: Test InnerGetVectorFromParcelIntelligent with valid small data
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, InnerGetVectorFromParcelIntelligent_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0003 Start";

    MessageParcel reply;
    std::vector<BundleInfo> parcelableInfos;
    const size_t dataSize = 1024;
    char buffer[dataSize] = {0};

    // Create a valid message parcel structure
    // First int32_t: number of BundleInfo entries (0 in this case)
    *reinterpret_cast<int32_t*>(buffer) = 0;

    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize)).WillOnce(Return(0));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(buffer));

    // Mock ParseFrom to succeed
    EXPECT_CALL(*messageParcelMock_, ParseFrom(_, _)).WillOnce(Return(true));

    auto ret = proxy_->InnerGetVectorFromParcelIntelligent(reply, parcelableInfos);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(parcelableInfos.empty());

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0003 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0004
 * @tc.desc: Test InnerGetVectorFromParcelIntelligent with ParseFrom failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, InnerGetVectorFromParcelIntelligent_0004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0004 Start";

    MessageParcel reply;
    std::vector<BundleInfo> parcelableInfos;
    const size_t dataSize = 1024;
    char buffer[dataSize] = {0};

    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(dataSize));
    EXPECT_CALL(*messageParcelMock_, ReadRawData(_)).WillOnce(Return(buffer));
    EXPECT_CALL(*messageParcelMock_, ParseFrom(_, _)).WillOnce(Return(false));

    auto ret = proxy_->InnerGetVectorFromParcelIntelligent(reply, parcelableInfos);
    EXPECT_EQ(ret, ERR_APPEXECFWK_PARCEL_ERROR);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_InnerGetVectorFromParcelIntelligent_0004 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetData_0000
 * @tc.desc: Test GetData with null data pointer
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetData_0000, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0000 Start";

    void* buffer = nullptr;
    const size_t size = 1024;
    const void* data = nullptr; // Null data

    auto ret = proxy_->GetData(buffer, size, data);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(buffer, nullptr);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0000 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetData_0001
 * @tc.desc: Test GetData with zero size
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetData_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0001 Start";

    void* buffer = nullptr;
    const size_t size = 0; // Zero size
    char data[1024] = {0};

    auto ret = proxy_->GetData(buffer, size, data);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(buffer, nullptr);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0001 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetData_0002
 * @tc.desc: Test GetData with size exceeding max capacity
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetData_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0002 Start";

    void* buffer = nullptr;
    const size_t size = Constants::MAX_PARCEL_CAPACITY + 1; // Exceeds max
    char data[Constants::MAX_PARCEL_CAPACITY + 1] = {0};

    auto ret = proxy_->GetData(buffer, size, data);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(buffer, nullptr);

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0002 End";
}

/**
 * @tc.name: BundleManagerAdapterProxy_GetData_0003
 * @tc.desc: Test GetData with malloc failure
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(BundleManagerAdapterProxyTest, BundleManagerAdapterProxy_GetData_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0003 Start";

    void* buffer = nullptr;
    const size_t size = 1024;
    char data[size] = {0};

    auto ret = proxy_->GetData(buffer, size, data);
    EXPECT_EQ(ret, true);
    EXPECT_NE(buffer, nullptr);

    if (buffer != nullptr) {
        free(buffer);
    }

    GTEST_LOG_(INFO) << "BundleManagerAdapterProxy_GetData_0003 End";
}
} // namespace OHOS::StorageManager::Test
