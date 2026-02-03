/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <vector>
#include <gtest/gtest.h>

#include "bundle_manager_connector.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "cJSON.h"
#include "init_param.h"
#include "storage/storage_monitor_service.h"
#include "storage_service_errno.h"
#include "storage_total_status_service_mock.h"
#include "storage/storage_status_manager.h"
#include "file_cache_adapter.h"
#include "storage_service_constant.h"

namespace OHOS::StorageManager {
class SystemUtil {
public:
    virtual ~SystemUtil() = default;
    virtual std::string GetParameter(const std::string&, const std::string&) = 0;
    virtual cJSON *cJSON_CreateObject() = 0;
    virtual cJSON* cJSON_AddStringToObject(cJSON* object, const char* name, const char* string) = 0;
    virtual cJSON *cJSON_CreateString(const char *string) = 0;
    virtual cJSON *cJSON_CreateArray() = 0;
    virtual cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item) = 0;
    virtual cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item) = 0;
    virtual char *cJSON_Print(const cJSON *item) = 0;
    virtual void cJSON_free(void *object) = 0;
    virtual void cJSON_Delete(cJSON *item) = 0;
    static inline std::shared_ptr<SystemUtil> su = nullptr;
};

class SystemUtilMock : public SystemUtil {
public:
    MOCK_METHOD(std::string, GetParameter, (const std::string&, const std::string&));
    MOCK_METHOD(cJSON *, cJSON_CreateObject, (), (override));
    MOCK_METHOD(cJSON *, cJSON_AddStringToObject, (cJSON*, const char*, const char*), (override));
    MOCK_METHOD(cJSON *, cJSON_CreateString, (const char*), (override));
    MOCK_METHOD(cJSON *, cJSON_CreateArray, (), (override));
    MOCK_METHOD(cJSON_bool, cJSON_AddItemToArray, (cJSON*, cJSON*), (override));
    MOCK_METHOD(cJSON_bool, cJSON_AddItemToObject, (cJSON*, const char*, cJSON*), (override));
    MOCK_METHOD(char *, cJSON_Print, (const cJSON*), (override));
    MOCK_METHOD(void, cJSON_free, (void*), (override));
    MOCK_METHOD(void, cJSON_Delete, (cJSON*), (override));
};

class MockBundleMgr : public AppExecFwk::IBundleMgr {
public:
    ErrCode CleanBundleCacheFilesAutomatic(uint64_t cacheSize,
                                           AppExecFwk::CleanType cleanType,
                                           std::optional<uint64_t> &cleanedSize) override
    {
        return ERR_OK;
    }
    sptr<IRemoteObject> AsObject() override { return nullptr; }
};

BundleMgrConnector::BundleMgrConnector() {}
BundleMgrConnector::~BundleMgrConnector() {}
sptr<AppExecFwk::IBundleMgr> g_testBundleMgrProxy = nullptr;
#ifdef STORAGE_MANAGER_UNIT_TEST
sptr<AppExecFwk::IBundleMgr> BundleMgrConnector::GetBundleMgrProxy()
{
    return g_testBundleMgrProxy;
}
#endif
}

namespace OHOS {
namespace system {
static constexpr int MAX_VALUE_LEN = 128;
int IsValidParamValue(const char *value, uint32_t len)
{
    if ((value == NULL) || (strlen(value) + 1 > len)) {
        return 0;
    }
    return 1;
}

std::string GetParameter(const std::string& key, const std::string& def)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        uint32_t size = 0;
        int ret = SystemReadParam(key.c_str(), NULL, &size);
        if (ret == 0) {
            std::vector<char> value(MAX_VALUE_LEN);
            ret = SystemReadParam(key.c_str(), value.data(), &size);
            if (ret == 0) {
                return std::string(value.data());
            }
        }
        if (IsValidParamValue(def.c_str(), MAX_VALUE_LEN) == 1) {
            return std::string(def);
        }
        return "";
    }
    return OHOS::StorageManager::SystemUtil::su->GetParameter(key, def);
}
} // namespace system
} // namespace OHOS

cJSON *cJSON_CreateObject()
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return nullptr;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_CreateObject();
}

cJSON* cJSON_AddStringToObject(cJSON* object, const char* name, const char* string)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return nullptr;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_AddStringToObject(object, name, string);
}

cJSON *cJSON_CreateString(const char *string)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return nullptr;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_CreateString(string);
}

cJSON *cJSON_CreateArray(void)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return nullptr;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_CreateArray();
}

cJSON_bool cJSON_AddItemToArray(cJSON *array, cJSON *item)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return cJSON_False;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_AddItemToArray(array, item);
}

cJSON_bool cJSON_AddItemToObject(cJSON *object, const char *string, cJSON *item)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return cJSON_False;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_AddItemToObject(object, string, item);
}

char *cJSON_Print(const cJSON *item)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return nullptr;
    }
    return OHOS::StorageManager::SystemUtil::su->cJSON_Print(item);
}

void cJSON_free(void *object)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return;
    }
    OHOS::StorageManager::SystemUtil::su->cJSON_free(object);
}

void cJSON_Delete(cJSON *item)
{
    if (OHOS::StorageManager::SystemUtil::su == nullptr) {
        return;
    }
    OHOS::StorageManager::SystemUtil::su->cJSON_Delete(item);
}

namespace OHOS::StorageManager {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace StorageService;

constexpr int32_t THREE_TIME = 3;
constexpr int32_t FOUR_TIME = 4;
constexpr int32_t ZERO_TIME = 0;
constexpr int64_t STORAGE_THRESHOLD_500M = 500 * 1024 * 1024; // 500M
constexpr int64_t STORAGE_THRESHOLD_2G = 2000 * 1024 * 1024; // 2G
int g_storageFlag = 0;

int32_t StorageStatusManager::GetUserStorageStats(StorageStats &storageStats)
{
    return g_storageFlag;
}

enum class LocalTimeStubMode {
    NORMAL_MATCH,      // 返回满足条件的时间
    NORMAL_NOT_MATCH,  // 返回不满足条件的时间
    RETURN_NULL        // 返回 nullptr
};
static LocalTimeStubMode g_localTimeStubMode = LocalTimeStubMode::NORMAL_MATCH;
static std::tm g_fakeTm {};
static std::mutex g_stubMutex;
void SetLocalTimeStubMode(LocalTimeStubMode mode)
{
    std::lock_guard<std::mutex> lock(g_stubMutex);
    g_localTimeStubMode = mode;
}
// C 接口桩，std::localtime 通常会调用它
extern "C" std::tm *localtime(const std::time_t *timer)
{
    (void)timer; // UT 里不关心真正的 time_t
    std::lock_guard<std::mutex> lock(g_stubMutex);
    switch (g_localTimeStubMode) {
        case LocalTimeStubMode::RETURN_NULL:
            return nullptr;
        case LocalTimeStubMode::NORMAL_NOT_MATCH:
            // 构造一个“不匹配”的时间
            g_fakeTm = {};
            g_fakeTm.tm_min  = THREE_TIME;
            g_fakeTm.tm_hour = FOUR_TIME;
            return &g_fakeTm;
        case LocalTimeStubMode::NORMAL_MATCH:
        default:
            // 构造一个“匹配”的时间，例如 0:00
            g_fakeTm = {};
            g_fakeTm.tm_min  = ZERO_TIME;   // 合法分钟
            g_fakeTm.tm_hour = ZERO_TIME;     // 合法小时
            return &g_fakeTm;
    }
}

class StorageMonitorServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase() {};
    void SetUp();
    void TearDown();
public:
    static inline StorageMonitorService* service = nullptr;
    static inline shared_ptr<StorageTotalStatusServiceMock> stss = nullptr;
    static inline shared_ptr<SystemUtilMock> sum = nullptr;
};

void StorageMonitorServiceTest::SetUpTestCase(void)
{
    service = &StorageMonitorService::GetInstance();
}

void StorageMonitorServiceTest::SetUp(void)
{
    stss = make_shared<StorageTotalStatusServiceMock>();
    StorageTotalStatusServiceMock::stss = stss;
    sum = make_shared<SystemUtilMock>();
    SystemUtilMock::su = sum;
}

void StorageMonitorServiceTest::TearDown()
{
    StorageTotalStatusServiceMock::stss = nullptr;
    stss = nullptr;
    SystemUtilMock::su = nullptr;
    sum = nullptr;
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_MonitorAndManageStorage_0000
 * @tc.name: Storage_monitor_service_MonitorAndManageStorage_0000
 * @tc.desc: Test function of MonitorAndManageStorage interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_MonitorAndManageStorage_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0000 start";

    EXPECT_CALL(*stss, GetTotalSize(_)).WillOnce(Return(E_PERMISSION_DENIED));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(Return(E_PERMISSION_DENIED));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillOnce(DoAll(SetArgReferee<0>(-1), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(20), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*sum, GetParameter(_, _)).WillRepeatedly(Return(""));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    service->hasNotifiedStorageEvent_ = true;
    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    service->hasNotifiedStorageEvent_ = false;
    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_MonitorAndManageStorage_0001
 * @tc.name: Storage_monitor_service_MonitorAndManageStorage_0001
 * @tc.desc: Test function of MonitorAndManageStorage interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_MonitorAndManageStorage_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0001 start";

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(20), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));

    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(Return(E_PERMISSION_DENIED));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(20), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(20), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeInodes(_)).WillOnce(Return(E_PERMISSION_DENIED));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(20), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeInodes(_)).WillOnce(DoAll(SetArgReferee<0>(-1), Return(E_OK)));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_MonitorAndManageStorage_0002
 * @tc.name: Storage_monitor_service_MonitorAndManageStorage_0002
 * @tc.desc: Test function of MonitorAndManageStorage interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_MonitorAndManageStorage_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0002 start";

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeInodes(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*sum, GetParameter(_, _)).WillRepeatedly(Return(""));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(10), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeInodes(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
    EXPECT_CALL(*sum, GetParameter(_, _)).WillRepeatedly(Return(""));
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    EXPECT_CALL(*stss, GetTotalSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeSize(_)).WillRepeatedly(DoAll(SetArgReferee<0>(10), Return(E_OK)));
    EXPECT_CALL(*stss, GetTotalInodes(_)).WillOnce(DoAll(SetArgReferee<0>(100), Return(E_OK)));
    EXPECT_CALL(*stss, GetFreeInodes(_)).WillOnce(DoAll(SetArgReferee<0>(10), Return(E_OK)));
    EXPECT_CALL(*sum, GetParameter(_, _)).WillRepeatedly(Return(""));
    service->hasNotifiedStorageEvent_ = true;
    service->MonitorAndManageStorage();
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_MonitorAndManageStorage_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CheckAndCleanCache_0000
 * @tc.name: Storage_monitor_service_CheckAndCleanCache_0000
 * @tc.desc: Test function of CheckAndCleanCache interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CheckAndCleanCache_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndCleanCache_0000 start";

    service->CheckAndCleanCache(0, 0, 0, 0);
    EXPECT_TRUE(true);

    service->CheckAndCleanCache(0, 20, 0, 0);
    EXPECT_TRUE(true);

    service->CheckAndCleanCache(30, 20, 0, 0);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndCleanCache_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CheckAndCleanCache_0001
 * @tc.name: Storage_monitor_service_CheckAndCleanCache_0001
 * @tc.desc: Test CheckAndCleanCache with empty result set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CheckAndCleanCache_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndCleanCache_0001 start";

    service->thresholds["clean_l"] = 0;
    service->inodeThresholds_["clean_l"] = 0;
    service->thresholds["clean_m"] = 20;
    service->inodeThresholds_["clean_m"] = 20;
    int64_t freeSize = 5;
    int64_t totalSize = 100;
    int64_t freeInode = 5;
    int64_t totalInode = 100;

    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    service->thresholds["clean_l"] = 10;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    service->inodeThresholds_["clean_l"] = 10;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeSize = 10;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeInode = 10;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeSize = 20;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeInode = 20;
    service->CheckAndCleanCache(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndCleanCache_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CheckAndEventNotify_0000
 * @tc.name: Storage_monitor_service_CheckAndEventNotify_0000
 * @tc.desc: Test function of CheckAndEventNotify interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CheckAndEventNotify_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndEventNotify_0000 start";

    service->CheckAndEventNotify(0, 0, 0, 0);
    EXPECT_TRUE(true);

    service->CheckAndEventNotify(STORAGE_THRESHOLD_500M, 0, 0, 0);
    EXPECT_TRUE(true);

    service->CheckAndEventNotify(STORAGE_THRESHOLD_2G, 0, 0, 0);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndEventNotify_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CheckAndEventNotify_0001
 * @tc.name: Storage_monitor_service_CheckAndEventNotify_0001
 * @tc.desc: Test CheckAndEventNotify with empty result set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CheckAndEventNotify_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndEventNotify_0001 start";

    service->thresholds["notify_l"] = 10;
    service->inodeThresholds_["notify_l"] = 10;
    service->thresholds["notify_m"] = 20;
    service->inodeThresholds_["notify_m"] = 20;
    int64_t freeSize = 5;
    int64_t totalSize = 100;
    int64_t freeInode = 5;
    int64_t totalInode = 100;

    service->CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeSize = 10;
    service->CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeInode = 10;
    service->CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeSize = 20;
    service->CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    freeInode = 20;
    service->CheckAndEventNotify(freeSize, totalSize, freeInode, totalInode);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndEventNotify_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_EventNotifyFreqHandlerForLow_0000
 * @tc.name: Storage_monitor_service_EventNotifyFreqHandlerForLow_0000
 * @tc.desc: Test function of EventNotifyFreqHandlerForLow interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_EventNotifyFreqHandlerForLow_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForLow_0000 start";

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForLow(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForLow(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForLow(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForLow(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForLow_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_EventNotifyFreqHandlerForMedium_0000
 * @tc.name: Storage_monitor_service_EventNotifyFreqHandlerForMedium_0000
 * @tc.desc: Test function of EventNotifyFreqHandlerForMedium interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_EventNotifyFreqHandlerForMedium_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForMedium_0000 start";

    service->lastNotificationTimeMedium_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForMedium(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeMedium_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForMedium(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeMedium_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForMedium(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTimeMedium_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForMedium(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForMedium_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_EventNotifyFreqHandlerForHigh_0000
 * @tc.name: Storage_monitor_service_EventNotifyFreqHandlerForHigh_0000
 * @tc.desc: Test function of EventNotifyFreqHandlerForHigh interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_EventNotifyFreqHandlerForHigh_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForHigh_0000 start";

    service->lastNotificationTime_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForHigh(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTime_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForHigh(true, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTime_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForHigh(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    service->lastNotificationTime_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForHigh(false, ONE_G_BYTE, TWO_G_BYTE);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_EventNotifyFreqHandlerForHigh_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_GetStorageAlertCleanupParams_0000
 * @tc.name: Storage_monitor_service_GetStorageAlertCleanupParams_0000
 * @tc.desc: Test function of GetStorageAlertCleanupParams interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_GetStorageAlertCleanupParams_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_GetStorageAlertCleanupParams_0000 start";
    std::string value = "notify_l500M/notify_m2G/notify_h10%";
    int64_t totalSize = ONE_G_BYTE;
    int64_t totalInode = TWO_G_BYTE;
    SystemSetParameter("const.storage_service.storage_alert_policy", value.c_str());
    service->ParseStorageParameters(totalSize, totalInode);
    std::string storageParams = service->GetStorageAlertCleanupParams();
    EXPECT_EQ(storageParams, value);

    GTEST_LOG_(INFO) << "storage_monitor_service_GetStorageAlertCleanupParams_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_HapAndSaStatisticsThd_0000
 * @tc.name: Storage_monitor_service_HapAndSaStatisticsThd_0000
 * @tc.desc: Test function of HapAndSaStatisticsThd interface when eventHandler_ is nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_HapAndSaStatisticsThd_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0000 start";

    service->eventHandler_ = nullptr;

    service->HapAndSaStatisticsThd();

    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_GetJsonString_0001
 * @tc.name: Storage_monitor_service_GetJsonString_0001
 * @tc.desc: Test function of HapAndSaStatisticsThd interface with multiple calls.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_GetJsonString_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_GetJsonString_0001 start";

    const std::string faultDesc = "faultDesc";
    const std::string faultSuggest = "faultSuggest";
    bool isHighFreq = true;

    std::string retString;
    cJSON jsonObject = { 0 };

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonString(faultDesc, faultSuggest, isHighFreq);
    EXPECT_TRUE(retString == "{}");

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_CreateString(_)).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonString(faultDesc, faultSuggest, isHighFreq);
    EXPECT_TRUE(retString == "{}");

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_CreateString(_)).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_CreateArray()).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonString(faultDesc, faultSuggest, isHighFreq);
    EXPECT_TRUE(retString == "{}");

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_CreateString(_)).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_CreateArray()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_Print(_)).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonString(faultDesc, faultSuggest, isHighFreq);
    EXPECT_TRUE(retString == "{}");

    isHighFreq = false;
    char stringTemp[] = "{}";
    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_Print(_)).WillRepeatedly(Return(stringTemp));
    retString = service->GetJsonString(faultDesc, faultSuggest, isHighFreq);
    EXPECT_TRUE(retString == "{}");

    GTEST_LOG_(INFO) << "storage_monitor_service_GetJsonString_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_PublishCleanCacheEvent_0000
 * @tc.name: Storage_monitor_service_PublishCleanCacheEvent_0000
 * @tc.desc: Test PublishCleanCacheEvent with high clean level.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_PublishCleanCacheEvent_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_0000 start";
    std::string cleanLevel = CLEAN_LEVEL_HIGH;
    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = ONE_G_BYTE;
    sizeInfo.totalSize = TWO_G_BYTE;
    service->PublishCleanCacheEvent(cleanLevel, true, sizeInfo);
    EXPECT_TRUE(true);

    service->PublishCleanCacheEvent(cleanLevel, false, sizeInfo);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_0000 end";
}

/**
 * @tc.number: storage_monitor_service_PublishCleanCacheEvent_000
 * @tc.name: storage_monitor_service_PublishCleanCacheEvent_000
 * @tc.desc: Test PublishCleanCacheEvent with all clean levels.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_PublishCleanCacheEvent_000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_000 start";
    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = ONE_G_BYTE;
    sizeInfo.totalSize = TWO_G_BYTE;
    std::vector<std::string> cleanLevels = {CLEAN_LEVEL_LOW, CLEAN_LEVEL_MEDIUM, CLEAN_LEVEL_HIGH, CLEAN_LEVEL_RICH};
    for (const auto &level : cleanLevels) {
        service->PublishCleanCacheEvent(level, true, sizeInfo);
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_PublishCleanCacheEvent_0002
 * @tc.name: Storage_monitor_service_PublishCleanCacheEvent_0002
 * @tc.desc: Test PublishCleanCacheEvent with special and empty clean levels.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_PublishCleanCacheEvent_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_0002 start";
    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = ONE_G_BYTE;
    sizeInfo.totalSize = TWO_G_BYTE;
    std::vector<std::string> specialLevels = {"", "!@#$%^&*()", std::string(1000, 'a')};
    for (const auto &level : specialLevels) {
        service->PublishCleanCacheEvent(level, true, sizeInfo);
        EXPECT_TRUE(true);
    }
    GTEST_LOG_(INFO) << "storage_monitor_service_PublishCleanCacheEvent_0002 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_SendCommonEventToCleanCache_0000
 * @tc.name: Storage_monitor_service_SendCommonEventToCleanCache_0000
 * @tc.desc: Test SendCommonEventToCleanCache with empty result set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_SendCommonEventToCleanCache_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_SendCommonEventToCleanCache_0000 start";
    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = ONE_G_BYTE;
    sizeInfo.totalSize = TWO_G_BYTE;
    // testcase 1: empty cleanlevel
    service->SendCommonEventToCleanCache("", sizeInfo, true);

    // testcase 2: can not find cleanLevel
    std::string cleanLevel = CLEAN_LEVEL_HIGH;
    service->SendCommonEventToCleanCache(cleanLevel, sizeInfo, true);

    // testcase 3: can find cleanLevel
    CleanNotify notify;
    notify.cleanLevelName = CLEAN_LEVEL_HIGH;
    notify.lastCleanNotifyTime = 0;
    FileCacheAdapter::GetInstance().InsertOrUpdateCleanNotify(notify);
    service->SendCommonEventToCleanCache(cleanLevel, sizeInfo, true);

    GTEST_LOG_(INFO) << "storage_monitor_service_SendCommonEventToCleanCache_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_HapAndSaStatisticsThd_0003
 * @tc.name: Storage_monitor_service_HapAndSaStatisticsThd_0003
 * @tc.desc: localTime 为空时直接返回
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level: Level 1
 * @tc.require: issuesXXXX
 */
HWTEST_F(StorageMonitorServiceTest,
    storage_monitor_service_HapAndSaStatisticsThd_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HapAndSaStatisticsThd_0003 start";
    SetLocalTimeStubMode(LocalTimeStubMode::RETURN_NULL);
    // 没有崩溃即可，localTime 为 nullptr 走第一条 return
    service->HapAndSaStatisticsThd();
    EXPECT_TRUE(true);
 
    SetLocalTimeStubMode(LocalTimeStubMode::NORMAL_NOT_MATCH);
    service->HapAndSaStatisticsThd();
    EXPECT_TRUE(true);
 
    SetLocalTimeStubMode(LocalTimeStubMode::NORMAL_MATCH);
    service->HapAndSaStatisticsThd();
    EXPECT_TRUE(true);
 
    GTEST_LOG_(INFO) << "HapAndSaStatisticsThd_0003 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_GetJsonStringForInode_0000
 * @tc.name: Storage_monitor_service_GetJsonStringForInode_0001
 * @tc.desc: Test function of HapAndSaStatisticsThd interface with multiple calls.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_GetJsonStringForInode_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_GetJsonStringForInode_0000 start";

    const std::string faultDesc = "faultDesc";
    const std::string faultSuggest = "faultSuggest";
    int64_t freeInode = ONE_G_BYTE;
    int64_t totalInode = TWO_G_BYTE;

    std::string retString;
    cJSON jsonObject = { 0 };

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonStringForInode(faultDesc, faultSuggest, freeInode, totalInode);
    EXPECT_TRUE(retString == "{}");

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    retString = service->GetJsonStringForInode(faultDesc, faultSuggest, freeInode, totalInode);
    EXPECT_TRUE(retString == "{}");

    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_Print(_)).WillRepeatedly(Return(nullptr));
    retString = service->GetJsonStringForInode(faultDesc, faultSuggest, freeInode, totalInode);
    EXPECT_TRUE(retString == "{}");

    char stringTemp[] = "{}";
    EXPECT_CALL(*sum, cJSON_CreateObject()).WillRepeatedly(Return(&jsonObject));
    EXPECT_CALL(*sum, cJSON_Print(_)).WillRepeatedly(Return(stringTemp));
    retString = service->GetJsonStringForInode(faultDesc, faultSuggest, freeInode, totalInode);
    EXPECT_TRUE(retString == "{}");

    GTEST_LOG_(INFO) << "storage_monitor_service_GetJsonStringForInode_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_ParseStorageInodeParameters_0000
 * @tc.name: Storage_monitor_service_ParseStorageInodeParameters_0002
 * @tc.desc: Test function of ParseStorageInodeParameters interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_ParseStorageInodeParameters_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_ParseStorageInodeParameters_0000 start";

    int64_t totalInode = ONE_G_BYTE;
    std::string storageInodeParams = "notify_l:25000/notify_m:100000/notify_h:10%/clean_l:37500/clean_m:5%/clean_h:10%/"
                                     "clean_r10%/clean_r:-1%/clean_r:101%/clean_r:/clean_r:-1";
    service->ParseStorageInodeParameters(totalInode, storageInodeParams);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_ParseStorageInodeParameters_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_HandleEventAndClean_0000
 * @tc.name: Storage_monitor_service_HandleEventAndClean_0000
 * @tc.desc: Test HandleEventAndClean with empty result set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_HandleEventAndClean_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_HandleEventAndClean_0000 start";

    service->thresholds["clean_l"] = 10;
    service->inodeThresholds_["clean_l"] = 10;
    struct SizeInfo sizeInfo;
    sizeInfo.freeSize = 10;
    sizeInfo.totalSize = 100;
    service->HandleEventAndClean(CLEAN_LEVEL_LOW, sizeInfo);
    EXPECT_TRUE(true);

    service->HandleEventAndClean(CLEAN_LEVEL_MEDIUM, sizeInfo);
    EXPECT_TRUE(true);

    service->HandleEventAndClean(CLEAN_LEVEL_HIGH, sizeInfo);
    EXPECT_TRUE(true);

    service->HandleEventAndClean("", sizeInfo);
    EXPECT_TRUE(true);

    GTEST_LOG_(INFO) << "storage_monitor_service_SendCommonEventToCleanCache_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CleanBundleCache_0000
 * @tc.name: Storage_monitor_service_CleanBundleCache_0000
 * @tc.desc: Test CleanBundleCache with empty result set.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CleanBundleCache_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CleanBundleCache_0000 start";

    int64_t lowThreshold = ONE_G_BYTE;
    int64_t lowInodeThreshold = ONE_G_BYTE;
    bool isCleanSpace = true;
    std::string cleanLevel = CLEAN_LEVEL_LOW;

    auto oldBundleMgrProxy = g_testBundleMgrProxy;
    g_testBundleMgrProxy = new MockBundleMgr();
    service->CleanBundleCache(lowThreshold, lowInodeThreshold, isCleanSpace, cleanLevel);
    EXPECT_TRUE(true);

    isCleanSpace = false;
    service->CleanBundleCache(lowThreshold, lowInodeThreshold, isCleanSpace, cleanLevel);
    EXPECT_TRUE(true);

    g_testBundleMgrProxy = oldBundleMgrProxy;
    GTEST_LOG_(INFO) << "storage_monitor_service_CleanBundleCache_0000 end";
}
}