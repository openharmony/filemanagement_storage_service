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

#include "storage_manager_impl.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "storage_service_errno.h"
#include "storage_service_log.h"

using namespace OHOS::StorageManager;

namespace OHOS {
namespace CJStorageManager {
CjStorageStatusService::CjStorageStatusService() {}
CjStorageStatusService::~CjStorageStatusService() {}

int32_t CjStorageStatusService::Connect()
{
    LOGD("CjStorageManagerConnect::Connect start");
    std::lock_guard<std::mutex> lock(mutex_);
    if (storageManager_ == nullptr) {
        auto sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (sam == nullptr) {
            LOGE("CjStorageManagerConnect::Connect samgr == nullptr");
            return E_SA_IS_NULLPTR;
        }
        auto object = sam->GetSystemAbility(STORAGE_MANAGER_MANAGER_ID);
        if (object == nullptr) {
            LOGE("CjStorageManagerConnect::Connect object == nullptr");
            return E_REMOTE_IS_NULLPTR;
        }
        storageManager_ = iface_cast<StorageManager::IStorageManager>(object);
        if (storageManager_ == nullptr) {
            LOGE("CjStorageManagerConnect::Connect service == nullptr");
            return E_SERVICE_IS_NULLPTR;
        }
        deathRecipient_ = new (std::nothrow) CjSmDeathRecipient();
        if (!deathRecipient_) {
            LOGE("CjStorageManagerConnect::Connect failed to create death recipient");
            storageManager_ = nullptr;
            return E_SERVICE_IS_NULLPTR;
        }
        if (storageManager_->AsObject() != nullptr) {
            storageManager_->AsObject()->AddDeathRecipient(deathRecipient_);
        }
    }
    LOGD("CjStorageManagerConnect::Connect end");
    return E_OK;
}

int32_t CjStorageStatusService::GetCurrentBundleStats(BundleStats &bundleStats, uint32_t statFlag)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("CjStorageManagerConnect::GetCurrentBundleStats:Connect error");
        return err;
    }
    if (storageManager_ == nullptr) {
        LOGE("CjStorageManagerConnect::GetCurrentBundleStats service == nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->GetCurrentBundleStats(bundleStats, statFlag);
}

int32_t CjStorageStatusService::GetTotalSize(int64_t &totalSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("CjStorageManagerConnect::GetTotalSize:Connect error");
        return err;
    }
    if (storageManager_ == nullptr) {
        LOGE("CjStorageManagerConnect::GetTotalSize service == nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->GetTotalSize(totalSize);
}

int32_t CjStorageStatusService::GetFreeSize(int64_t &freeSize)
{
    int32_t err = Connect();
    if (err != E_OK) {
        LOGE("CjStorageManagerConnect::GetFreeSize:Connect error");
        return err;
    }
    if (storageManager_ == nullptr) {
        LOGE("CjStorageManagerConnect::GetFreeSize service == nullptr");
        return E_SERVICE_IS_NULLPTR;
    }
    return storageManager_->GetFreeSize(freeSize);
}

int32_t CjStorageStatusService::ResetProxy()
{
    LOGI("CjStorageManagerConnect::ResetProxy start");
    std::lock_guard<std::mutex> lock(mutex_);
    if ((storageManager_ != nullptr) && (storageManager_->AsObject() != nullptr)) {
        storageManager_->AsObject()->RemoveDeathRecipient(deathRecipient_);
    }
    storageManager_ = nullptr;
    return E_OK;
}

void CjSmDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LOGI("SmDeathRecipient::OnRemoteDied reset proxy.");
    auto service = DelayedSingleton<CjStorageStatusService>::GetInstance();
    if (service == nullptr) {
        return;
    }
    service->ResetProxy();
}

int32_t Convert2CjErrNum(int32_t errNum)
{
    static std::unordered_map<int32_t, int32_t> errCodeTable {
        { E_PERMISSION_DENIED, E_PERMISSION },
        { E_WRITE_DESCRIPTOR_ERR, E_IPCSS},
        { E_NON_EXIST, E_NOOBJECT},
        { E_PREPARE_DIR, E_PREPARE_DIR},
        { E_DESTROY_DIR, E_DESTROY_DIR},
        { E_VOL_MOUNT_ERR, E_MOUNT_ERR},
        { E_VOL_UMOUNT_ERR, E_UNMOUNT},
        { E_SET_POLICY, E_SET_POLICY},
        { E_USERID_RANGE, E_OUTOFRANGE},
        { E_VOL_STATE, E_VOLUMESTATE},
        { E_UMOUNT_BUSY, E_VOLUMESTATE},
        { E_NOT_SUPPORT, E_SUPPORTEDFS},
        { E_SYS_KERNEL_ERR, E_UNMOUNT},
        { E_NO_CHILD, E_NO_CHILD},
        { E_WRITE_PARCEL_ERR, E_IPCSS},
        { E_WRITE_REPLY_ERR, E_IPCSS},
        { E_SA_IS_NULLPTR, E_IPCSS},
        { E_REMOTE_IS_NULLPTR, E_IPCSS},
        { E_SERVICE_IS_NULLPTR, E_IPCSS},
        { E_BUNDLEMGR_ERROR, E_IPCSS},
        { E_MEDIALIBRARY_ERROR, E_IPCSS},
    };

    if (errCodeTable.find(errNum) != errCodeTable.end()) {
        return errCodeTable.at(errNum);
    } else {
        return errNum;
    }
};

} // namespace CJStorageManager
} // namespace OHOS