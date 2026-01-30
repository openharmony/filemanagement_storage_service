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

#ifndef STORAGE_MANAGER_SERVICE_BUNDLE_MANAGER_ADAPTER_PROXY_H
#define STORAGE_MANAGER_SERVICE_BUNDLE_MANAGER_ADAPTER_PROXY_H

#include <string>
#include <vector>

#include "bundle_framework_core_ipc_interface_code.h"
#include "bundle_mgr_interface.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace StorageManager {
using namespace AppExecFwk;
using namespace AAFwk;
class BundleManagerAdapterProxy : public IRemoteProxy<IBundleMgr> {
public:
    explicit BundleManagerAdapterProxy(const sptr<IRemoteObject> &impl);
    ~BundleManagerAdapterProxy() override;

    /**
     * @brief Obtains the bundle name of a specified application based on the given UID through the proxy object.
     * @param uid Indicates the uid.
     * @param bundleName Indicates the obtained bundle name.
     * @return Returns true if the bundle name is successfully obtained; returns false otherwise.
     */
    virtual bool GetBundleNameForUid(const int uid, std::string &bundleName) override;

    /**
     * @brief Clears cache data of a specified size through the proxy object.
     * @param cacheSize Indicates the size of the cache data is to be cleared.
     * @return Returns ERR_OK if this function is successfully called; returns other ErrCode otherwise.
     */
    virtual ErrCode CleanBundleCacheFilesAutomatic(uint64_t cacheSize) override;

    /**
     * @brief Clears cache data of a specified size.
     * @param cacheSize Indicates the size of the cache data is to be cleared.
	 * @param cleanType Indicates the type of cache data to be cleared.
     * @param cleanedSize Indicates the size of the cache data that is actually cleared.
     * @return Returns ERR_OK if this function is successfully called; returns other ErrCode otherwise.
     */
    virtual ErrCode CleanBundleCacheFilesAutomatic(uint64_t cacheSize,
                                                   CleanType cleanType,
                                                   std::optional<uint64_t>& cleanedSize) override;

    /**
     * @brief Obtains BundleInfo of all bundles available in the system through the proxy object.
     * @param flags Indicates the flag used to specify information contained in the BundleInfo that will be returned.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @param userId Indicates the user ID.
     * @return Returns ERR_OK if the BundleInfos is successfully obtained; returns error code otherwise.
     */
    virtual ErrCode GetBundleInfosV9(int32_t flags, std::vector<BundleInfo> &bundleInfos, int32_t userId) override;

    /**
     * @brief Obtains the formal name associated with the given UID.
     * @param uid Indicates the uid.
     * @param bundleName Indicates the obtained formal bundleName.
     * @param appIndex Indicates the obtained appIndex.
     * @return Returns ERR_OK if execute success; returns errCode otherwise.
     */
    virtual ErrCode GetNameAndIndexForUid(const int32_t uid, std::string &bundleName, int32_t &appIndex) override;

    virtual bool GetBundleStats(const std::string &bundleName,
                                int32_t userId,
                                std::vector<int64_t> &bundleStats,
                                int32_t appIndex = 0,
                                uint32_t statFlag = 0) override;

    virtual ErrCode CreateBundleDataDirWithEl(int32_t userId, DataDirEl dirEl) override;

    virtual bool GetAllBundleStats(int32_t userId, std::vector<int64_t> &bundleStats) override;

private:
    /**
     * @brief Send a command message from the proxy object.
     * @param code Indicates the message code to be sent.
     * @param data Indicates the objects to be sent.
     * @param reply Indicates the reply to be sent;
     * @return Returns true if message send successfully; returns false otherwise.
     */
    bool SendTransactCmd(BundleMgrInterfaceCode code, MessageParcel &data, MessageParcel &reply);

    template<typename T>
    ErrCode GetVectorFromParcelIntelligentWithErrCode(BundleMgrInterfaceCode code,
                                                      MessageParcel &data,
                                                      std::vector<T> &parcelableInfos);

    /**
     * @brief Send a command message from the proxy object and  printf log.
     * @param code Indicates the message code to be sent.
     * @param data Indicates the objects to be sent.
     * @param reply Indicates the reply to be sent;
     * @return Returns true if message send successfully; returns false otherwise.
     */
    bool SendTransactCmdWithLog(BundleMgrInterfaceCode code, MessageParcel &data, MessageParcel &reply);

    template<typename T>
    ErrCode InnerGetVectorFromParcelIntelligent(MessageParcel &reply, std::vector<T> &parcelableInfos);

    bool SendData(void *&buffer, size_t size, const void *data);
    bool GetData(void *&buffer, size_t size, const void *data);

    ErrCode GetParcelInfoFromAshMem(MessageParcel &reply, void *&data);

    static inline BrokerDelegator<BundleManagerAdapterProxy> delegator_;
};
} // namespace StorageManager
} // namespace OHOS
#endif // STORAGE_MANAGER_SERVICE_BUNDLE_MANAGER_ADAPTER_PROXY_H
