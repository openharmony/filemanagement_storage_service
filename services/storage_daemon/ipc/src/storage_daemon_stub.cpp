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

#include "ipc/storage_daemon_stub.h"

#include <cinttypes>
#include "ipc/storage_daemon_ipc_interface_code.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "securec.h"
#include "utils/storage_xcollie.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
constexpr unsigned int LOCAL_TIME_OUT_SECONDS = 10;
constexpr unsigned int INACTIVE_USER_KEY_OUT_SECONDS = 25;
constexpr unsigned int UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS = 300;
constexpr unsigned int RADAR_REPORT_STATISTIC_INTERVAL_MINUTES = 1440;
constexpr unsigned int USER0ID = 0;
constexpr unsigned int USER100ID = 100;
constexpr unsigned int RADAR_STATISTIC_THREAD_WAIT_SECONDS = 60;
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024;

static bool GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        LOGE("null data");
        return false;
    }
    if (size == 0 || size > MAX_IPC_RAW_DATA_SIZE) {
        LOGE("size invalid: %{public}zu", size);
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        LOGE("malloc buffer failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != E_OK) {
        free(buffer);
        LOGE("memcpy failed");
        return false;
    }
    return true;
}

static bool ReadBatchUriByRawData(MessageParcel &data, std::vector<std::string> &uriVec)
{
    size_t dataSize = static_cast<size_t>(data.ReadInt32());
    if (dataSize == 0) {
        LOGE("parcel no data");
        return false;
    }

    void *buffer = nullptr;
    if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
        LOGE("read raw data failed: %{public}zu", dataSize);
        return false;
    }

    MessageParcel tempParcel;
    if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        LOGE("failed to parseFrom");
        free(buffer);
        return false;
    }
    tempParcel.ReadStringVector(&uriVec);
    return true;
}

int32_t ReadBatchUris(MessageParcel &data, std::vector<std::string> &uriVec)
{
    uint32_t size = data.ReadUint32();
    if (size == 0) {
        LOGE("out of range: %{public}u", size);
        return E_PARAMS_INVALID;
    }
    if (!ReadBatchUriByRawData(data, uriVec)) {
        LOGE("read uris failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

std::map<uint32_t, RadarStatisticInfo>::iterator StorageDaemonStub::GetUserStatistics(const uint32_t userId)
{
    auto it = opStatistics_.find(userId);
    if (it != opStatistics_.end()) {
        return it;
    }
    RadarStatisticInfo radarInfo = { 0 };
    return opStatistics_.insert(make_pair(userId, radarInfo)).first;
}

void StorageDaemonStub::GetTempStatistics(std::map<uint32_t, RadarStatisticInfo> &statistics)
{
    std::lock_guard<std::mutex> lock(mutex_);
    statistics.insert(opStatistics_.begin(), opStatistics_.end());
    opStatistics_.clear();
}

void StorageDaemonStub::StorageRadarThd(void)
{
    // report radar statistics when restart
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    if (execRadarReportCon_.wait_for(lock, std::chrono::seconds(RADAR_STATISTIC_THREAD_WAIT_SECONDS),
        [this] { return this->stopRadarReport_.load(); })) {
        LOGI("Storage statistic radar exit.");
        return;
    }
    lock.unlock();
    LOGI("Storage statistic thread start.");
    StorageStatisticRadar::GetInstance().CreateStatisticFile();
    std::map<uint32_t, RadarStatisticInfo> opStatisticsTemp;
    StorageStatisticRadar::GetInstance().ReadStatisticFile(opStatisticsTemp);
    if (!opStatisticsTemp.empty()) {
        for (auto ele : opStatisticsTemp) {
            StorageService::StorageRadar::ReportStatistics(ele.first, ele.second);
        }
        StorageStatisticRadar::GetInstance().CleanStatisticFile();
    }
    lastRadarReportTime_ = std::chrono::system_clock::now();
    while (!stopRadarReport_.load()) {
        std::unique_lock<std::mutex> lock(onRadarReportLock_);
        if (execRadarReportCon_.wait_for(lock, std::chrono::seconds(UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS),
            [this] { return this->stopRadarReport_.load(); })) {
            LOGI("Storage statistic radar exit.");
            return;
        }
        std::chrono::time_point<std::chrono::system_clock> nowTime = std::chrono::system_clock::now();
        int64_t intervalMinutes =
            std::chrono::duration_cast<std::chrono::minutes>(nowTime - lastRadarReportTime_).count();
        if ((intervalMinutes > RADAR_REPORT_STATISTIC_INTERVAL_MINUTES) && !opStatistics_.empty()) {
            LOGI("Storage statistic report, intervalMinutes:%{public}" PRId64, intervalMinutes);
            opStatisticsTemp.clear();
            GetTempStatistics(opStatisticsTemp);
            for (auto ele : opStatisticsTemp) {
                StorageService::StorageRadar::ReportStatistics(ele.first, ele.second);
            }
            lastRadarReportTime_ = std::chrono::system_clock::now();
            StorageStatisticRadar::GetInstance().CleanStatisticFile();
            continue;
        }
        if (!isNeedUpdateRadarFile_) {
            LOGI("Storage statistic not need update.");
            continue;
        }
        LOGI("Storage statistic update, intervalMinutes:%{public}" PRId64, intervalMinutes);
        isNeedUpdateRadarFile_ = false;
        StorageStatisticRadar::GetInstance().UpdateStatisticFile(opStatistics_);
    }
}

StorageDaemonStub::StorageDaemonStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SHUTDOWN)] =
        &StorageDaemonStub::HandleShutdown;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CHECK)] =
        &StorageDaemonStub::HandleCheck;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT)] =
        &StorageDaemonStub::HandleMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT)] =
        &StorageDaemonStub::HandleUMount;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::PARTITION)] =
        &StorageDaemonStub::HandlePartition;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::FORMAT)] =
        &StorageDaemonStub::HandleFormat;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC)] =
        &StorageDaemonStub::HandleSetVolDesc;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::QUERY_USB_IS_IN_USE)] =
        &StorageDaemonStub::HandleQueryUsbIsInUse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS)] =
        &StorageDaemonStub::HandlePrepareUserDirs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS)] =
        &StorageDaemonStub::HandleDestroyUserDirs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::START_USER)] =
        &StorageDaemonStub::HandleStartUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::STOP_USER)] =
        &StorageDaemonStub::HandleStopUser;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY)] =
        &StorageDaemonStub::HandleInitGlobalKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS)] =
        &StorageDaemonStub::HandleInitGlobalUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS)] =
        &StorageDaemonStub::HandleGenerateUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS)] =
        &StorageDaemonStub::HandleDeleteUserKeys;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH)] =
        &StorageDaemonStub::HandleUpdateUserAuth;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY)] =
        &StorageDaemonStub::HandleUpdateUseAuthWithRecoveryKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY)] =
        &StorageDaemonStub::HandleActiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY)] =
        &StorageDaemonStub::HandleInactiveUserKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN)] =
        &StorageDaemonStub::HandleLockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN)] =
        &StorageDaemonStub::HandleUnlockUserScreen;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_SCREEN_STATUS)] =
        &StorageDaemonStub::HandleGetLockScreenStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT)] =
        &StorageDaemonStub::HandleUpdateKeyContext;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_CRYPTO_PATH_AGAIN)] =
        &StorageDaemonStub::HandleMountCryptoPathAgain;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE)] =
        &StorageDaemonStub::HandleCreateShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE)] =
        &StorageDaemonStub::HandleDeleteShareFile;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_BUNDLE_QUOTA)] =
        &StorageDaemonStub::HandleSetBundleQuota;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_SPACE)] =
        &StorageDaemonStub::HandleGetOccupiedSpace;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_MEM_PARA)] =
        &StorageDaemonStub::HandleUpdateMemoryPara;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_BUNDLE_STATS_INCREASE)] =
        &StorageDaemonStub::HandleGetBundleStatsForIncrease;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY)] =
        &StorageDaemonStub::HandleGenerateAppkey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY)] =
        &StorageDaemonStub::HandleDeleteAppkey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS)] =
        &StorageDaemonStub::HandleMountDfsDocs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_DFS_DOCS)] =
        &StorageDaemonStub::HandleUMountDfsDocs;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_FILE_ENCRYPT_STATUS)] =
        &StorageDaemonStub::HandleGetFileEncryptStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_RECOVER_KEY)] =
        &StorageDaemonStub::HandleCreateRecoverKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_RECOVER_KEY)] =
        &StorageDaemonStub::HandleSetRecoverKey;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_MEDIA_FUSE)] =
        &StorageDaemonStub::HandleMountMediaFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_MEDIA_FUSE)] =
        &StorageDaemonStub::HandleUMountMediaFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_USER_NEED_ACTIVE_STATUS)] =
        &StorageDaemonStub::HandleGetUserNeedActiveStatus;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_FILE_MGR_FUSE)] =
            &StorageDaemonStub::HandleMountFileMgrFuse;
    opToInterfaceMap_[static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_FILE_MGR_FUSE)] =
            &StorageDaemonStub::HandleUMountFileMgrFuse;
    callRadarStatisticReportThread_ = std::thread([this]() { StorageRadarThd(); });
}

StorageDaemonStub::~StorageDaemonStub()
{
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    stopRadarReport_ = true;
    execRadarReportCon_.notify_one();
    lock.unlock();
    if (callRadarStatisticReportThread_.joinable()) {
        callRadarStatisticReportThread_.join();
    }
}

int32_t StorageDaemonStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                           MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_PERMISSION_DENIED;
    }
    switch (code) {
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SHUTDOWN):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CHECK):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::PARTITION):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::FORMAT):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::QUERY_USB_IS_IN_USE):
            return OnRemoteRequestForBase(code, data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::START_USER):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::STOP_USER):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::COMPLETE_ADD_USER):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_RECOVER_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_RECOVER_KEY):
            return OnRemoteRequestForUser(code, data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_SCREEN_STATUS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_CRYPTO_PATH_AGAIN):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_BUNDLE_QUOTA):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_SPACE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_MEM_PARA):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_BUNDLE_STATS_INCREASE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_DFS_DOCS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_FILE_ENCRYPT_STATUS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_MEDIA_FUSE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_MEDIA_FUSE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_USER_NEED_ACTIVE_STATUS):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_FILE_MGR_FUSE):
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_FILE_MGR_FUSE):
            return OnRemoteRequestForApp(code, data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t StorageDaemonStub::OnRemoteRequestForBase(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    switch (code) {
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SHUTDOWN):
            return HandleShutdown(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CHECK):
            return HandleCheck(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT):
            return HandleMount(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT):
            return HandleUMount(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::PARTITION):
            return HandlePartition(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::FORMAT):
            return HandleFormat(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_VOL_DESC):
            return HandleSetVolDesc(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::QUERY_USB_IS_IN_USE):
            return HandleQueryUsbIsInUse(data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return E_SYS_KERNEL_ERR;
    }
}
int32_t StorageDaemonStub::OnRemoteRequestForUser(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    switch (code) {
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::PREPARE_USER_DIRS):
            return HandlePrepareUserDirs(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DESTROY_USER_DIRS):
            return HandleDestroyUserDirs(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::START_USER):
            return HandleStartUser(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::STOP_USER):
            return HandleStopUser(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::COMPLETE_ADD_USER):
            return HandleCompleteAddUser(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_KEY):
            return HandleInitGlobalKey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INIT_GLOBAL_USER_KEYS):
            return HandleInitGlobalUserKeys(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_USER_KEYS):
            return HandleGenerateUserKeys(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_USER_KEYS):
            return HandleDeleteUserKeys(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH):
            return HandleUpdateUserAuth(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_USER_AUTH_RECOVER_KEY):
            return HandleUpdateUseAuthWithRecoveryKey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::ACTIVE_USER_KEY):
            return HandleActiveUserKey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::INACTIVE_USER_KEY):
            return HandleInactiveUserKey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_USER_SCREEN):
            return HandleLockUserScreen(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UNLOCK_USER_SCREEN):
            return HandleUnlockUserScreen(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_RECOVER_KEY):
            return HandleCreateRecoverKey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_RECOVER_KEY):
            return HandleSetRecoverKey(data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return E_SYS_KERNEL_ERR;
    }
}
int32_t StorageDaemonStub::OnRemoteRequestForApp(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    switch (code) {
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::LOCK_SCREEN_STATUS):
            return HandleGetLockScreenStatus(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_KEY_CONTEXT):
            return HandleUpdateKeyContext(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_CRYPTO_PATH_AGAIN):
            return HandleMountCryptoPathAgain(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::CREATE_SHARE_FILE):
            return HandleCreateShareFile(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_SHARE_FILE):
            return HandleDeleteShareFile(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::SET_BUNDLE_QUOTA):
            return HandleSetBundleQuota(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_SPACE):
            return HandleGetOccupiedSpace(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UPDATE_MEM_PARA):
            return HandleUpdateMemoryPara(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_BUNDLE_STATS_INCREASE):
            return HandleGetBundleStatsForIncrease(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_DFS_DOCS):
            return HandleMountDfsDocs(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_DFS_DOCS):
            return HandleUMountDfsDocs(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GENERATE_APP_KEY):
            return HandleGenerateAppkey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::DELETE_APP_KEY):
            return HandleDeleteAppkey(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_FILE_ENCRYPT_STATUS):
            return HandleGetFileEncryptStatus(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_MEDIA_FUSE):
            return HandleMountMediaFuse(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_MEDIA_FUSE):
            return HandleUMountMediaFuse(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::GET_USER_NEED_ACTIVE_STATUS):
            return HandleGetUserNeedActiveStatus(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::MOUNT_FILE_MGR_FUSE):
            return HandleMountFileMgrFuse(data, reply);
        case static_cast<uint32_t>(StorageDaemonInterfaceCode::UMOUNT_FILE_MGR_FUSE):
            return HandleUMountFileMgrFuse(data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return E_SYS_KERNEL_ERR;
    }
}

int32_t StorageDaemonStub::HandleShutdown(MessageParcel &data, MessageParcel &reply)
{
    Shutdown();
    return E_OK;
}

int32_t StorageDaemonStub::HandleMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    uint32_t flags = data.ReadUint32();

    int err = Mount(volId, flags);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUMount(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = UMount(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleCheck(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();

    int err = Check(volId);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleFormat(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string fsType = data.ReadString();

    int err = Format(volId, fsType);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandlePartition(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    int32_t type = data.ReadInt32();

    int err = Partition(volId, type);
    if (!reply.WriteInt32(err)) {
        return  E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleSetVolDesc(MessageParcel &data, MessageParcel &reply)
{
    std::string volId = data.ReadString();
    std::string description = data.ReadString();

    int err = SetVolumeDescription(volId, description);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleQueryUsbIsInUse(MessageParcel &data, MessageParcel &reply)
{
    std::string diskPath = data.ReadString();
    bool isInUse = true;
    int32_t res = QueryUsbIsInUse(diskPath, isInUse);
    if (!reply.WriteBool(isInUse)) {
        LOGE("Write bool error");
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(res)) {
        LOGE("Write res failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandlePrepareUserDirs(MessageParcel &data, MessageParcel &reply)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = PrepareUserDirs(userId, flags);
    if (err != E_OK) {
        it->second.userAddFailCount++;
    } else {
        it->second.userAddSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleDestroyUserDirs(MessageParcel &data, MessageParcel &reply)
{
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t userId = data.ReadInt32();
    uint32_t flags = data.ReadUint32();
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = DestroyUserDirs(userId, flags);
    if (err != E_OK) {
        it->second.userRemoveFailCount++;
    } else {
        it->second.userRemoveSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleStartUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = StartUser(userId);
    if (err != E_OK) {
        it->second.userStartFailCount++;
    } else {
        it->second.userStartSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    LOGI("SD_DURATION: HANDLE START USER: delay time = %{public}s",
        StorageService::StorageRadar::RecordDuration(startTime).c_str());
    return E_OK;
}

int32_t StorageDaemonStub::HandleStopUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int32_t err = StopUser(userId);
    if (err != E_OK) {
        it->second.userStopFailCount++;
    } else {
        it->second.userStopSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleCompleteAddUser(MessageParcel &data, MessageParcel &reply)
{
    int32_t userId = data.ReadInt32();

    int32_t err = CompleteAddUser(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalKey(MessageParcel &data, MessageParcel &reply)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(USER0ID);
    isNeedUpdateRadarFile_ = true;
    int err = InitGlobalKey();
    if (err != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleInitGlobalUserKeys(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Stub_InitGlobalUserKeys start.");
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(USER100ID);
    isNeedUpdateRadarFile_ = true;
    int err = InitGlobalUserKeys();
    if (err != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGenerateUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t flags = data.ReadUint32();
    int timerId = StorageXCollie::SetTimer("storage:GenerateUserKeys", LOCAL_TIME_OUT_SECONDS);
    int err = GenerateUserKeys(userId, flags);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteUserKeys(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int timerId = StorageXCollie::SetTimer("storage:DeleteUserKeys", LOCAL_TIME_OUT_SECONDS);
    int err = DeleteUserKeys(userId);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateUserAuth(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint64_t secureUid = data.ReadUint64();

    std::vector<uint8_t> token;
    std::vector<uint8_t> oldSecret;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&oldSecret);
    data.ReadUInt8Vector(&newSecret);

    int timerId = StorageXCollie::SetTimer("storage:UpdateUserAuth", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err = UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateUseAuthWithRecoveryKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint64_t secureUid = data.ReadUint64();

    std::vector<uint8_t> token;
    std::vector<uint8_t> newSecret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&newSecret);
    std::vector<std::vector<uint8_t>> plainText;
    const int CKEY_NUMS = 6;
    for (uint32_t i = 0; i < CKEY_NUMS; i++) {
        std::vector<uint8_t> iv;
        data.ReadUInt8Vector(&iv);
        plainText.push_back(iv);
    }

    int err = UpdateUseAuthWithRecoveryKey(token, newSecret, secureUid, userId, plainText);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleActiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int timerId = StorageXCollie::SetTimer("storage:ActiveUserKey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = ActiveUserKey(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    if ((err == E_OK) || ((err == E_ACTIVE_EL2_FAILED) && token.empty() && secret.empty())) {
        it->second.keyLoadSuccCount++;
    } else {
        it->second.keyLoadFailCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    LOGI("SD_DURATION: READ KEY FILE: delay time = %{public}s",
        StorageService::StorageRadar::RecordDuration(startTime).c_str());
    return E_OK;
}

int32_t StorageDaemonStub::HandleInactiveUserKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int timerId = StorageXCollie::SetTimer("storage:InactiveUserKey", INACTIVE_USER_KEY_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = InactiveUserKey(userId);
    StorageXCollie::CancelTimer(timerId);
    if (err != E_OK) {
        it->second.keyUnloadFailCount++;
    } else {
        it->second.keyUnloadSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleLockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    int timerId = StorageXCollie::SetTimer("storage:LockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = LockUserScreen(userId);
    StorageXCollie::CancelTimer(timerId);
    if (err != E_OK) {
        it->second.keyUnloadFailCount++;
    } else {
        it->second.keyUnloadSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleUnlockUserScreen(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);

    int timerId = StorageXCollie::SetTimer("storage:UnlockUserScreen", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = GetUserStatistics(userId);
    isNeedUpdateRadarFile_ = true;
    int err = UnlockUserScreen(userId, token, secret);
    StorageXCollie::CancelTimer(timerId);
    if (err != E_OK) {
        it->second.keyLoadFailCount++;
    } else {
        it->second.keyLoadSuccCount++;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetLockScreenStatus(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    bool lockScreenStatus = false;
    int timerId = StorageXCollie::SetTimer("storage:GetLockScreenStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err = GetLockScreenStatus(userId, lockScreenStatus);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteBool(lockScreenStatus)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleGenerateAppkey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t hashId = data.ReadUint32();
    std::string keyId;
    int timerId = StorageXCollie::SetTimer("storage:GenerateAppkey", LOCAL_TIME_OUT_SECONDS);
    int err = GenerateAppkey(userId, hashId, keyId);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteString(keyId)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteAppkey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::string keyId = data.ReadString();
    int timerId = StorageXCollie::SetTimer("storage:DeleteAppkey", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err = DeleteAppkey(userId, keyId);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleCreateRecoverKey(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    uint32_t userType = data.ReadUint32();

    std::vector<uint8_t> token;
    std::vector<uint8_t> secret;
    data.ReadUInt8Vector(&token);
    data.ReadUInt8Vector(&secret);
    int err = CreateRecoverKey(userId, userType, token, secret);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleSetRecoverKey(MessageParcel &data, MessageParcel &reply)
{
    std::vector<uint8_t> key;
    data.ReadUInt8Vector(&key);
    int err = SetRecoverKey(key);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleUpdateKeyContext(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    bool needRemoveTmpKey = data.ReadBool();

    int timerId = StorageXCollie::SetTimer("storage:UpdateKeyContext", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int err = UpdateKeyContext(userId, needRemoveTmpKey);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }

    return E_OK;
}

int32_t StorageDaemonStub::HandleMountCryptoPathAgain(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    int32_t err = MountCryptoPathAgain(userId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleCreateShareFile(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> uriList;
    auto ret = ReadBatchUris(data, uriList);
    if (ret != E_OK) {
        return ret;
    }
    uint32_t tokenId = data.ReadUint32();
    uint32_t flag = data.ReadUint32();
    std::vector<int32_t> retList = CreateShareFile(uriList, tokenId, flag);
    if (!reply.WriteInt32Vector(retList)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleDeleteShareFile(MessageParcel &data, MessageParcel &reply)
{
    uint32_t tokenId = data.ReadUint32();
    std::vector<std::string> uriList;
    auto ret = ReadBatchUris(data, uriList);
    if (ret != E_OK) {
        return ret;
    }
    int err = DeleteShareFile(tokenId, uriList);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleSetBundleQuota(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int32_t uid = data.ReadInt32();
    std::string bundleDataDirPath = data.ReadString();
    int32_t limitSizeMb = data.ReadInt32();
    int err = SetBundleQuota(bundleName, uid, bundleDataDirPath, limitSizeMb);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetOccupiedSpace(MessageParcel &data, MessageParcel &reply)
{
    int32_t idType = data.ReadInt32();
    int32_t id = data.ReadInt32();
    int64_t size = 0;
    int err = GetOccupiedSpace(idType, id, size);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64(size)) {
        LOGE("StorageManagerStub::HandleGetFree call GetTotalSize failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}
int32_t StorageDaemonStub::HandleUpdateMemoryPara(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    int32_t oldSize = 0;
    int err = UpdateMemoryPara(size, oldSize);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(oldSize)) {
    LOGE("StorageManagerStub::HandleUpdateMemoryPara call Write oldSize failed");
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetBundleStatsForIncrease(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    std::vector<std::string> bundleNames;
    if (!data.ReadStringVector(&bundleNames)) {
        return E_WRITE_REPLY_ERR;
    }
    std::vector<int64_t> incrementalBackTimes;
    if (!data.ReadInt64Vector(&incrementalBackTimes)) {
        return E_WRITE_REPLY_ERR;
    }

    std::vector<int64_t> pkgFileSizes;
    std::vector<int64_t> incPkgFileSizes;
    int32_t err = GetBundleStatsForIncrease(userId, bundleNames, incrementalBackTimes, pkgFileSizes, incPkgFileSizes);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64Vector(pkgFileSizes)) {
        LOGE("StorageDaemonStub::HandleGetBundleStatsForIncrease call GetBundleStatsForIncrease failed");
        return  E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt64Vector(incPkgFileSizes)) {
        LOGE("StorageDaemonStub::HandleGetBundleStatsForIncrease call GetBundleStatsForIncrease failed");
        return  E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleMountDfsDocs(MessageParcel &data, MessageParcel &reply)
{
    LOGI("StorageDaemonStub::HandleMountDfsDocs start.");
    int32_t userId = data.ReadInt32();
    std::string relativePath = data.ReadString();
    std::string networkId = data.ReadString();
    std::string deviceId = data.ReadString();

    int32_t err = MountDfsDocs(userId, relativePath, networkId, deviceId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleUMountDfsDocs(MessageParcel &data, MessageParcel &reply)
{
    LOGI("StorageDaemonStub::HandleUMountDfsDocs start.");
    int32_t userId = data.ReadInt32();
    std::string relativePath = data.ReadString();
    std::string networkId = data.ReadString();
    std::string deviceId = data.ReadString();

    int32_t err = UMountDfsDocs(userId, relativePath, networkId, deviceId);
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetFileEncryptStatus(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    bool needCheckDirMount = data.ReadBool();
    bool isEncrypted = true;
    int timerId = StorageXCollie::SetTimer("storage:GetFileEncryptStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteBool(isEncrypted)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleMountMediaFuse(MessageParcel &data, MessageParcel &reply)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonStub::HandleMountMediaFuse start.");

    int32_t userId = data.ReadInt32();
    int32_t fd = -1;
    int32_t ret = MountMediaFuse(userId, fd);
    if (!reply.WriteInt32(ret)) {
        LOGE("Write reply error code failed");
        if (ret == E_OK) {
            close(fd);
        }
        return E_WRITE_REPLY_ERR;
    }
    if (ret == E_OK && fd > 0) {
        if (!reply.WriteFileDescriptor(fd)) {
            LOGE("Write reply fd failed");
            close(fd);
            return E_WRITE_REPLY_ERR;
        }
        close(fd);
    }
#endif
    return E_OK;
}

int32_t StorageDaemonStub::HandleUMountMediaFuse(MessageParcel &data, MessageParcel &reply)
{
#ifdef STORAGE_SERVICE_MEDIA_FUSE
    LOGI("StorageDaemonStub::HandleUMountMediaFuse start.");

    int32_t userId = data.ReadInt32();
    int32_t ret = UMountMediaFuse(userId);
    if (!reply.WriteInt32(ret)) {
        return E_WRITE_REPLY_ERR;
    }
#endif
    return E_OK;
}

int32_t StorageDaemonStub::HandleGetUserNeedActiveStatus(MessageParcel &data, MessageParcel &reply)
{
    uint32_t userId = data.ReadUint32();
    bool needActive = false;
    int timerId = StorageXCollie::SetTimer("storage:GetUserNeedActiveStatus", LOCAL_TIME_OUT_SECONDS);
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t err = GetUserNeedActiveStatus(userId, needActive);
    StorageXCollie::CancelTimer(timerId);
    if (!reply.WriteBool(needActive)) {
        return E_WRITE_REPLY_ERR;
    }
    if (!reply.WriteInt32(err)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleMountFileMgrFuse(MessageParcel &data, MessageParcel &reply)
{
    LOGI("StorageDaemonStub::HandleMountFileMgrFuse start.");
    int32_t userId = data.ReadInt32();
    std::string path = data.ReadString();
    int32_t fuseFd = -1;
    int32_t ret = MountFileMgrFuse(userId, path, fuseFd);
    if (!reply.WriteInt32(ret)) {
        LOGE("Write reply error code failed");
        if (ret == E_OK) {
            close(fuseFd);
        }
        return E_WRITE_REPLY_ERR;
    }
    if (ret == E_OK && fuseFd > 0) {
        if (!reply.WriteFileDescriptor(fuseFd)) {
            LOGE("Write reply fuseFd failed");
            close(fuseFd);
            return E_WRITE_REPLY_ERR;
        }
        close(fuseFd);
    }
    return E_OK;
}

int32_t StorageDaemonStub::HandleUMountFileMgrFuse(MessageParcel &data, MessageParcel &reply)
{
    LOGI("StorageDaemonStub::HandleUMountFileMgrFuse start.");
    int32_t userId = data.ReadInt32();
    std::string path = data.ReadString();
    int32_t ret = UMountFileMgrFuse(userId, path);
    if (!reply.WriteInt32(ret)) {
        return E_WRITE_REPLY_ERR;
    }
    return E_OK;
}
} // StorageDaemon
} // OHOS
