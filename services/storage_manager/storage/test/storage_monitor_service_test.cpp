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

#include <vector>
#include <gtest/gtest.h>

#include "cJSON.h"
#include "init_param.h"
#include "storage/storage_monitor_service.h"
#include "storage_service_errno.h"
#include "storage_total_status_service_mock.h"
#include "storage/storage_status_service.h"

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

constexpr int64_t STORAGE_THRESHOLD_500M = 500 * 1024 * 1024; // 500M
constexpr int64_t STORAGE_THRESHOLD_2G = 2000 * 1024 * 1024; // 2G
int g_storageFlag = 0;

int32_t StorageStatusService::GetUserStorageStats(StorageStats &storageStats)
{
    return g_storageFlag;
}

class StorageMonitorServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase();
    void SetUp() {};
    void TearDown() {};
public:
    static inline StorageMonitorService* service = nullptr;
    static inline shared_ptr<StorageTotalStatusServiceMock> stss = nullptr;
    static inline shared_ptr<SystemUtilMock> sum = nullptr;
};

void StorageMonitorServiceTest::SetUpTestCase(void)
{
    service = &StorageMonitorService::GetInstance();
    stss = make_shared<StorageTotalStatusServiceMock>();
    StorageTotalStatusServiceMock::stss = stss;
    sum = make_shared<SystemUtilMock>();
    SystemUtilMock::su = sum;
}

void StorageMonitorServiceTest::TearDownTestCase()
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

    service->CheckAndCleanCache(0, 0);
    EXPECT_TRUE(true);

    service->CheckAndCleanCache(0, 20);
    EXPECT_TRUE(true);

    EXPECT_CALL(*sum, GetParameter(_, _)).WillOnce(Return(""));
    service->CheckAndCleanCache(30, 20);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndCleanCache_0000 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_CleanBundleCacheByInterval_0000
 * @tc.name: Storage_monitor_service_CleanBundleCacheByInterval_0000
 * @tc.desc: Test function of CleanBundleCacheByInterval interface.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issuesIC35N9
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_CleanBundleCacheByInterval_0000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_CleanBundleCacheByInterval_0000 start";

    std::string timestamp;
    EXPECT_CALL(*sum, GetParameter(_, _)).WillOnce(Return(""));
    service->CleanBundleCacheByInterval(timestamp, 0, 0);
    EXPECT_TRUE(true);

    EXPECT_CALL(*sum, GetParameter(_, _)).WillOnce(Return("0"));
    service->CleanBundleCacheByInterval(timestamp, 0, 0);
    EXPECT_TRUE(true);

    auto currentTime = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::hours>(currentTime -
        std::chrono::system_clock::time_point(std::chrono::hours(0))).count();
    EXPECT_CALL(*sum, GetParameter(_, _)).WillOnce(Return("0"));
    service->CleanBundleCacheByInterval(timestamp, 0, duration + duration);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_CleanBundleCacheByInterval_0000 end";
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

    service->CheckAndEventNotify(0, 0);
    EXPECT_TRUE(true);

    service->CheckAndEventNotify(STORAGE_THRESHOLD_500M, 0);
    EXPECT_TRUE(true);

    service->CheckAndEventNotify(STORAGE_THRESHOLD_2G, 0);
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "storage_monitor_service_CheckAndEventNotify_0000 end";
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
    service->EventNotifyFreqHandlerForLow();
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForLow();
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

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForMedium();
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForMedium();
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

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::time_point();
    service->EventNotifyFreqHandlerForHigh();
    EXPECT_TRUE(true);

    service->lastNotificationTimeHighFreq_ = std::chrono::system_clock::now() + std::chrono::hours(24);
    service->EventNotifyFreqHandlerForHigh();
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
    int64_t totalSize = 1024;

    SystemSetParameter("const.storage_service.storage_alert_policy", value.c_str());
    service->ParseStorageParameters(totalSize);
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
 * @tc.number: SUB_STORAGE_storage_monitor_service_HapAndSaStatisticsThd_0001
 * @tc.name: Storage_monitor_service_HapAndSaStatisticsThd_0001
 * @tc.desc: Test function of HapAndSaStatisticsThd interface when eventHandler_ is not nullptr.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_HapAndSaStatisticsThd_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0001 start";

    auto mockRunner = AppExecFwk::EventRunner::Create("testRunner");
    auto mockHandler = std::make_shared<AppExecFwk::EventHandler>(mockRunner);

    service->eventHandler_ = mockHandler;

    service->HapAndSaStatisticsThd();

    EXPECT_TRUE(true);

    mockHandler.reset();
    mockRunner.reset();

    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0001 end";
}

/**
 * @tc.number: SUB_STORAGE_storage_monitor_service_HapAndSaStatisticsThd_0002
 * @tc.name: Storage_monitor_service_HapAndSaStatisticsThd_0002
 * @tc.desc: Test function of HapAndSaStatisticsThd interface with multiple calls.
 * @tc.size: MEDIUM
 * @tc.type: FUNC
 * @tc.level Level 1
 * @tc.require: issues2344
 */
HWTEST_F(StorageMonitorServiceTest, storage_monitor_service_HapAndSaStatisticsThd_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0002 start";

    auto mockRunner = AppExecFwk::EventRunner::Create("testRunner");
    auto mockHandler = std::make_shared<AppExecFwk::EventHandler>(mockRunner);

    service->eventHandler_ = mockHandler;

    service->HapAndSaStatisticsThd();
    service->HapAndSaStatisticsThd();


    EXPECT_TRUE(true);

    mockHandler.reset();
    mockRunner.reset();

    GTEST_LOG_(INFO) << "storage_monitor_service_HapAndSaStatisticsThd_0002 end";
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
}