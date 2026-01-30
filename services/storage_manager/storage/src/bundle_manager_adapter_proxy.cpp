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
#include "securec.h"
#include "storage_service_log.h"

namespace OHOS {
namespace StorageManager {
namespace {
constexpr size_t MAX_PARCEL_CAPACITY_OF_ASHMEM = 1024 * 1024 * 1024; // allow max 1GB resource size
constexpr size_t MAX_IPC_REWDATA_SIZE = 120 * 1024 * 1024;           // max ipc size 120MB
} // namespace

BundleManagerAdapterProxy::BundleManagerAdapterProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IBundleMgr>(impl)
{}

BundleManagerAdapterProxy::~BundleManagerAdapterProxy()
{}

bool BundleManagerAdapterProxy::GetBundleNameForUid(const int uid, std::string &bundleName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("fail to GetBundleNameForUid due to write InterfaceToken fail");
        return false;
    }
    if (!data.WriteInt32(uid)) {
        LOGE("fail to GetBundleNameForUid due to write uid fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::GET_BUNDLE_NAME_FOR_UID, data, reply)) {
        LOGE("fail to GetBundleNameForUid from server");
        return false;
    }
    if (!reply.ReadBool()) {
        if (uid > Constants::BASE_APP_UID) {
            LOGD("reply result false");
        }
        return false;
    }
    bundleName = reply.ReadString();
    return true;
}

ErrCode BundleManagerAdapterProxy::CleanBundleCacheFilesAutomatic(uint64_t cacheSize)
{
    if (cacheSize == 0) {
        LOGE("parameter error, cache size must be greater than 0");
        return ERR_BUNDLE_MANAGER_INVALID_PARAMETER;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("fail to CleanBundleCacheFilesAutomatic due to write InterfaceToken fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteUint64(cacheSize)) {
        LOGE("fail to CleanBundleCacheFilesAutomatic due to write cache size fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::AUTO_CLEAN_CACHE_BY_SIZE, data, reply)) {
        LOGE("fail to CleanBundleCacheFilesAutomatic from server");
        return ERR_BUNDLE_MANAGER_IPC_TRANSACTION;
    }
    return reply.ReadInt32();
}

ErrCode BundleManagerAdapterProxy::CleanBundleCacheFilesAutomatic(uint64_t cacheSize, CleanType cleanType,
                                                                  std::optional<uint64_t>& cleanedSize)
{
    if (cacheSize == 0) {
        LOGE("parameter error, cache size must be greater than 0");
        return ERR_BUNDLE_MANAGER_INVALID_PARAMETER;
    }

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("fail to CleanBundleCacheFilesAutomatic write InterfaceToken fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteUint64(cacheSize)) {
        LOGE("fail to CleanBundleCacheFilesAutomatic write cache size fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt8(static_cast<int8_t>(cleanType))) {
        LOGE("fail to CleanBundleCacheFilesAutomatic write clean type fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::AUTO_CLEAN_CACHE_BY_INODE, data, reply)) {
        LOGE("fail CleanBundleCacheFilesAutomatic from CBCFA server");
        return ERR_BUNDLE_MANAGER_IPC_TRANSACTION;
    }

    ErrCode result = reply.ReadInt32();
    bool hasValue = reply.ReadBool();
    if (hasValue) {
        cleanedSize = reply.ReadUint64();
    }
    return result;
}

ErrCode BundleManagerAdapterProxy::GetBundleInfosV9(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("fail to GetBundleInfosV9 due to write InterfaceToken fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt32(flags)) {
        LOGE("fail to GetBundleInfosV9 due to write flag fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("fail to GetBundleInfosV9 due to write userId fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    LOGI("get bundleinfos end");
    return GetVectorFromParcelIntelligentWithErrCode<BundleInfo>(
        BundleMgrInterfaceCode::GET_BUNDLE_INFOS_WITH_INT_FLAGS_V9, data, bundleInfos);
}

ErrCode BundleManagerAdapterProxy::GetNameAndIndexForUid(const int32_t uid, std::string &bundleName, int32_t &appIndex)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("fail to GetNameAndIndexForUid due to write InterfaceToken fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt32(uid)) {
        LOGE("fail to GetNameAndIndexForUid due to write uid fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    if (!SendTransactCmdWithLog(BundleMgrInterfaceCode::GET_NAME_AND_APPINDEX_FOR_UID, data, reply)) {
        LOGE("fail to GetNameAndIndexForUid from server");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    ErrCode ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        return ret;
    }
    bundleName = reply.ReadString();
    appIndex = reply.ReadInt32();
    return ERR_OK;
}

bool BundleManagerAdapterProxy::GetBundleStats(const std::string &bundleName,
                                               int32_t userId,
                                               std::vector<int64_t> &bundleStats,
                                               int32_t appIndex,
                                               uint32_t statFlag)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("failed to GetBundleStats due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteString(bundleName)) {
        LOGE("fail to GetBundleStats due to write bundleName fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("fail to GetBundleStats due to write userId fail");
        return false;
    }
    if (!data.WriteInt32(appIndex)) {
        LOGE("fail to GetBundleStats due to write appIndex fail");
        return false;
    }
    if (!data.WriteUint32(statFlag)) {
        LOGE("fail to GetBundleStats due to write statFlag fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::GET_BUNDLE_STATS, data, reply)) {
        LOGE("fail to GetBundleStats from server");
        return false;
    }
    if (!reply.ReadBool()) {
        LOGE("reply result false");
        return false;
    }
    if (!reply.ReadInt64Vector(&bundleStats)) {
        LOGE("fail to GetBundleStats from reply");
        return false;
    }
    LOGI("end %{public}s", bundleName.c_str());
    return true;
}

ErrCode BundleManagerAdapterProxy::CreateBundleDataDirWithEl(int32_t userId, DataDirEl dirEl)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Write interfaceToken failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("fail to CreateBundleDataDirWithEl due to write userId fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (!data.WriteUint8(static_cast<uint8_t>(dirEl))) {
        LOGE("fail to CreateBundleDataDirWithEl due to write dirEl fail");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::CREATE_BUNDLE_DATA_DIR_WITH_EL, data, reply)) {
        LOGE("Call failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return reply.ReadInt32();
}

bool BundleManagerAdapterProxy::GetAllBundleStats(int32_t userId, std::vector<int64_t> &bundleStats)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("failed to GetAllBundleStats due to write MessageParcel fail");
        return false;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("fail to GetAllBundleStats due to write userId fail");
        return false;
    }

    MessageParcel reply;
    if (!SendTransactCmd(BundleMgrInterfaceCode::GET_ALL_BUNDLE_STATS, data, reply)) {
        LOGE("fail to GetAllBundleStats from server");
        return false;
    }
    if (!reply.ReadBool()) {
        LOGE("reply result false");
        return false;
    }
    if (!reply.ReadInt64Vector(&bundleStats)) {
        LOGE("fail to GetAllBundleStats from reply");
        return false;
    }
    LOGI("GetAllBundleStats end");
    return true;
}

bool BundleManagerAdapterProxy::SendTransactCmd(BundleMgrInterfaceCode code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("fail to send transact cmd %{public}d due to remote object", code);
        return false;
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

template<typename T>
ErrCode BundleManagerAdapterProxy::GetVectorFromParcelIntelligentWithErrCode(BundleMgrInterfaceCode code,
                                                                             MessageParcel &data,
                                                                             std::vector<T> &parcelableInfos)
{
    MessageParcel reply;
    if (!SendTransactCmd(code, data, reply)) {
        LOGE("SendTransactCmd failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    ErrCode res = reply.ReadInt32();
    if (res != ERR_OK) {
        return res;
    }

    return InnerGetVectorFromParcelIntelligent<T>(reply, parcelableInfos);
}

template<typename T>
ErrCode BundleManagerAdapterProxy::InnerGetVectorFromParcelIntelligent(MessageParcel &reply,
                                                                       std::vector<T> &parcelableInfos)
{
    size_t dataSize = static_cast<size_t>(reply.ReadInt32());
    if (dataSize == 0) {
        LOGW("Parcel no data");
        return ERR_OK;
    }

    void *buffer = nullptr;
    if (dataSize > MAX_IPC_REWDATA_SIZE) {
        LOGI("dataSize is too large, use ashmem");
        if (GetParcelInfoFromAshMem(reply, buffer) != ERR_OK) {
            LOGE("read data from ashmem fail, length %{public}zu", dataSize);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    } else {
        if (!GetData(buffer, dataSize, reply.ReadRawData(dataSize))) {
            LOGE("Fail read raw data length %{public}zu", dataSize);
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
    }

    MessageParcel tempParcel;
    if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        LOGE("Fail to ParseFrom");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    int32_t infoSize = tempParcel.ReadInt32();
    for (int32_t i = 0; i < infoSize; i++) {
        std::unique_ptr<T> info(tempParcel.ReadParcelable<T>());
        if (info == nullptr) {
            LOGE("Read Parcelable infos failed");
            return ERR_APPEXECFWK_PARCEL_ERROR;
        }
        parcelableInfos.emplace_back(*info);
    }

    return ERR_OK;
}

bool BundleManagerAdapterProxy::SendTransactCmdWithLog(BundleMgrInterfaceCode code,
                                                       MessageParcel &data,
                                                       MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("fail send transact cmd %{public}d due remote object", code);
        return false;
    }
    int32_t sptrRefCount = remote->GetSptrRefCount();
    int32_t wptrRefCount = remote->GetWptrRefCount();
    if (sptrRefCount <= 0 || wptrRefCount <= 0) {
        LOGI("SendRequest before sptrRefCount: %{public}d wptrRefCount: %{public}d", sptrRefCount, wptrRefCount);
    }
    int32_t result = remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
    if (result != NO_ERROR) {
        LOGE("receive error transact code %{public}d in transact cmd %{public}d", result, code);
        return false;
    }
    return true;
}

ErrCode BundleManagerAdapterProxy::GetParcelInfoFromAshMem(MessageParcel &reply, void *&data)
{
    sptr<Ashmem> ashMem = reply.ReadAshmem();
    if (ashMem == nullptr) {
        LOGE("Ashmem is nullptr");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }

    if (!ashMem->MapReadOnlyAshmem()) {
        LOGE("MapReadOnlyAshmem failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    int32_t ashMemSize = ashMem->GetAshmemSize();
    int32_t offset = 0;
    const void *ashDataPtr = ashMem->ReadFromAshmem(ashMemSize, offset);
    if (ashDataPtr == nullptr) {
        LOGE("ashDataPtr is nullptr");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if ((ashMemSize == 0) || ashMemSize > static_cast<int32_t>(MAX_PARCEL_CAPACITY_OF_ASHMEM)) {
        LOGE("failed due to wrong size");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    data = malloc(ashMemSize);
    if (data == nullptr) {
        LOGE("failed due to malloc data failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    if (memcpy_s(data, ashMemSize, ashDataPtr, ashMemSize) != EOK) {
        free(data);
        LOGE("failed due to memcpy_s failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    return ERR_OK;
}

bool BundleManagerAdapterProxy::GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        LOGE("GetData failed due to null data");
        return false;
    }
    if (size == 0 || size > Constants::MAX_PARCEL_CAPACITY) {
        LOGE("GetData failed due to zero size");
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        LOGE("GetData failed due to malloc buffer failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != EOK) {
        free(buffer);
        LOGE("GetData failed due to memcpy_s failed");
        return false;
    }
    return true;
}
} // namespace StorageManager
} // namespace OHOS
