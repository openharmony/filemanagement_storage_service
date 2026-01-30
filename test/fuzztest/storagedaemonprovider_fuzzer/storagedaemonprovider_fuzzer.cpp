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
#include "storagedaemonprovider_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <securec.h>
#include <string>
#include <algorithm>
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>
#include "message_parcel.h"
#include "storage_daemon.h"
#include "storage_daemon_provider.h"
#include "storage_daemon_stub.h"
#include "user_manager.h"
#include "uece_activation_callback_stub.h"
#include <memory.h>

#include <fstream>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#ifdef EXTERNAL_STORAGE_MANAGER
#include "disk/disk_config.h"
#include "disk/disk_info.h"
#include "disk/disk_manager.h"
#include "netlink/netlink_manager.h"
#endif
#include "ipc/storage_daemon_provider.h"
#include "ipc/storage_daemon.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "storage_service_constant.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "system_ability_definition.h"
#include "user/user_manager.h"
#include "utils/string_utils.h"
#ifdef DFS_SERVICE
#include "cloud_daemon_manager.h"
#endif
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
#include "mtp/mtp_device_monitor.h"
#endif
using namespace OHOS;
#ifdef DFS_SERVICE
using namespace OHOS::FileManagement::CloudFile;
#endif
using CloudListener = StorageDaemon::StorageDaemonProvider::SystemAbilityStatusChangeListener;
#ifdef EXTERNAL_STORAGE_MANAGER
const int CONFIG_PARAM_NUM = 6;
static const std::string CONFIG_PATH = "/system/etc/storage_daemon/disk_config";

static bool ParasConfig(StorageDaemon::DiskManager &dm)
{
    std::ifstream infile;
    infile.open(CONFIG_PATH);
    if (!infile) {
        return false;
    }
    while (infile) {
        std::string line;
        std::getline(infile, line);
        if (line.empty()) {
            break;
        }
        std::string token = " ";
        auto split = StorageDaemon::SplitLine(line, token);
        if (split.size() != CONFIG_PARAM_NUM) {
            continue;
        }
        auto it = split.begin();
        if (*it != "sysPattern") {
            continue;
        }
        auto sysPattern = *(++it);
        if (*(++it) != "label") {
            continue;
        }
        auto label = *(++it);
        if (*(++it) != "flag") {
            continue;
        }
        it++;
        int flag = std::atoi((*it).c_str());
        auto diskConfig =  std::make_shared<StorageDaemon::DiskConfig>(sysPattern, label, flag);
        dm.AddDiskConfig(diskConfig);
    }
    infile.close();
    return true;
}
#endif
using namespace OHOS;
using namespace OHOS::StorageDaemon;
namespace OHOS {

sptr<OHOS::StorageDaemon::StorageDaemonProvider> storageDaemonProvider = NULL;
StorageDaemon::UserManager &userManager = StorageDaemon::UserManager::GetInstance();
};

// Mock implementation for IUeceActivationCallback
class UeceActivationCallbackStubImpl : public StorageManager::UeceActivationCallbackStub  {
public:
    OHOS::ErrCode OnEl5Activation(int32_t resultCode, int32_t userId, bool needGetAllAppKey,
        int32_t& funcResult) { return 0;}
};
sptr<StorageManager::IUeceActivationCallback> GetMockUeceActivationCallbackStub()
{
    sptr<UeceActivationCallbackStubImpl> impl =
        sptr<UeceActivationCallbackStubImpl>::MakeSptr();
    return impl;
}

static void SetPriority()
{
    int tid = syscall(SYS_gettid);
    setpriority(PRIO_PROCESS, tid, OHOS::StorageService::PRIORITY_LEVEL);
}

static const int32_t SLEEP_TIME_INTERVAL_3MS = 3 * 1000;
static const int32_t MAX_RETRY_COUNT = 10000;
static const int32_t MAX_TEST_COUNT = 100;
static const int32_t DEFAULT_USERID = 100;

static bool RegisterStorageDaemonProvider(sptr<ISystemAbilityManager> samgr)
{
    storageDaemonProvider = new OHOS::StorageDaemon::StorageDaemonProvider();
    if (storageDaemonProvider == nullptr) {
        return false;
    }
    
    int ret = samgr->AddSystemAbility(STORAGE_MANAGER_DAEMON_ID, storageDaemonProvider);
    return (ret == 0);
}

static bool SubscribeSystemAbilities(sptr<ISystemAbilityManager> samgr)
{
    sptr<CloudListener> listener(new CloudListener());
    if (listener == nullptr) {
        return false;
    }
    
    int ret = samgr->SubscribeSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, listener);
    
    ret = samgr->SubscribeSystemAbility(ACCESS_TOKEN_MANAGER_SERVICE_ID, listener);
    
    return true;
}

#ifdef EXTERNAL_STORAGE_MANAGER
static void InitializeExternalStorage()
{
    OHOS::StorageDaemon::NetlinkManager::Instance().Start();
    OHOS::StorageDaemon::DiskManager &dm = OHOS::StorageDaemon::DiskManager::Instance();
    ParasConfig(dm);
}
#endif
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
static void InitializeMtpDevice()
{
    OHOS::StorageDaemon::MtpDeviceMonitor::GetInstance().StartMonitor();
}
#endif

static void InitializeStorageComponents()
{
#ifdef EXTERNAL_STORAGE_MANAGER
    InitializeExternalStorage();
#endif
    
#ifdef SUPPORT_OPEN_SOURCE_MTP_DEVICE
    InitializeMtpDevice();
#endif
}

static bool InitializeSystemAbilityManager()
{
    for (int retry = 0; retry < MAX_RETRY_COUNT; ++retry) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgr == nullptr) {
            usleep(SLEEP_TIME_INTERVAL_3MS);
            continue;
        }
        
        if (!RegisterStorageDaemonProvider(samgr)) {
            return false;
        }
        
        if (!SubscribeSystemAbilities(samgr)) {
            return false;
        }
        
        return true;
    }
    return false;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (!InitializeSystemAbilityManager()) {
        return -1;
    }
    (void)SetPriority();
    InitializeStorageComponents();
    return 0;
}

enum StorageDaemonFunction {
    FUNC_SHUTDOWN = 1,
    FUNC_MOUNT = 2,
    FUNC_UMOUNT = 3,
    FUNC_CHECK = 4,
    FUNC_FORMAT = 5,
    FUNC_PARTITION = 6,
    FUNC_SET_VOLUME_DESCRIPTION = 7,
    FUNC_TRY_TO_FIX = 8,
    FUNC_QUERY_USB_IS_IN_USE = 9,
    
    FUNC_START_USER = 10,
    FUNC_STOP_USER = 11,
    FUNC_PREPARE_USER_DIRS = 12,
    FUNC_DESTROY_USER_DIRS = 13,
    FUNC_COMPLETE_ADD_USER = 14,
    FUNC_CREATE_USER_DIR = 15,
    
    FUNC_INIT_GLOBAL_KEY = 16,
    FUNC_INIT_GLOBAL_USER_KEYS = 17,
    FUNC_GENERATE_USER_KEYS = 18,
    FUNC_DELETE_USER_KEYS = 19,
    FUNC_UPDATE_USER_AUTH = 20,
    FUNC_ACTIVE_USER_KEY = 21,
    FUNC_INACTIVE_USER_KEY = 22,
    FUNC_UPDATE_KEY_CONTEXT = 23,
    FUNC_MOUNT_CRYPTO_PATH_AGAIN = 24,
    
    FUNC_LOCK_USER_SCREEN = 25,
    FUNC_UNLOCK_USER_SCREEN = 26,
    FUNC_GET_LOCK_SCREEN_STATUS = 27,
    
    FUNC_UPDATE_USE_AUTH_WITH_RECOVERY_KEY = 28,
    FUNC_CREATE_RECOVER_KEY = 29,
    FUNC_SET_RECOVER_KEY = 30,
    FUNC_RESET_SECRET_WITH_RECOVERY_KEY = 31,
    
    FUNC_GENERATE_APPKEY = 32,
    FUNC_DELETE_APPKEY = 33,
    FUNC_CREATE_SHARE_FILE = 34,
    FUNC_DELETE_SHARE_FILE = 35,
    
    FUNC_SET_BUNDLE_QUOTA = 36,
    FUNC_GET_OCCUPIED_SPACE = 37,
    FUNC_QUERY_OCCUPIED_SPACE_FOR_SA = 38,
    FUNC_UPDATE_MEMORY_PARA = 39,
    
    FUNC_MOUNT_DFS_DOCS = 40,
    FUNC_UMOUNT_DFS_DOCS = 41,
    FUNC_GET_FILE_ENCRYPT_STATUS = 42,
    FUNC_GET_USER_NEED_ACTIVE_STATUS = 43,
    
    FUNC_MOUNT_MEDIA_FUSE = 44,
    FUNC_UMOUNT_MEDIA_FUSE = 45,
    FUNC_MOUNT_FILE_MGR_FUSE = 46,
    FUNC_UMOUNT_FILE_MGR_FUSE = 47,
    FUNC_MOUNT_USB_FUSE = 48,
    
    FUNC_IS_FILE_OCCUPIED = 49,
    
    FUNC_PREPARE_ANCO_USER_DIRS = 50,
    FUNC_VERIFY_ANCO_USER_DIRS = 51,
    FUNC_GET_ANCO_SIZE_DATA = 52,
    
    FUNC_MOUNT_DIS_SHARE_FILE = 53,
    FUNC_UMOUNT_DIS_SHARE_FILE = 54,
    
    FUNC_REGISTER_UEEC_ACTIVATION_CALLBACK = 55,
    FUNC_UNREGISTER_UEEC_ACTIVATION_CALLBACK = 56,
    
    FUNC_INACTIVE_USER_PUBLIC_DIR_KEY = 57,
    FUNC_UPDATE_USER_PUBLIC_DIR_POLICY = 58,
    FUNC_SET_DIR_ENCRYPTION_POLICY = 59,
    
    FUNC_GET_DQ_BLK_SPACES_BY_UIDS = 60,
    FUNC_GET_DIR_LIST_SPACE = 61,
    FUNC_SET_STOP_SCAN_FLAG = 62,
    FUNC_GET_DATA_SIZE_BY_PATH = 63,
    FUNC_GET_RMG_RESOURCE_SIZE = 64
};

static const int IPC_CODES[] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 62, 63, 64
};

static void HandleBasicStorageOps(FuzzedDataProvider& provider,
                                  OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                  StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_SHUTDOWN: providerObj->Shutdown(); break;
        case FUNC_MOUNT: {
            std::string volId = provider.ConsumeRandomLengthString();
            uint32_t flags = provider.ConsumeIntegral<uint32_t>();
            providerObj->Mount(volId, flags);
            break;
        }
        case FUNC_UMOUNT: {
            std::string volId = provider.ConsumeRandomLengthString();
            providerObj->UMount(volId);
            break;
        }
        case FUNC_CHECK: {
            std::string volId = provider.ConsumeRandomLengthString();
            providerObj->Check(volId);
            break;
        }
        default: break;
    }
}

static void HandleDiskFormatOps(FuzzedDataProvider& provider,
                                OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_FORMAT: {
            std::string volId = provider.ConsumeRandomLengthString();
            std::string fsType = provider.ConsumeRandomLengthString();
            providerObj->Format(volId, fsType);
            break;
        }
        case FUNC_PARTITION: {
            std::string diskId = provider.ConsumeRandomLengthString();
            int type = provider.ConsumeIntegral<int>();
            providerObj->Partition(diskId, type);
            break;
        }
        case FUNC_SET_VOLUME_DESCRIPTION: {
            std::string volId = provider.ConsumeRandomLengthString();
            std::string description = provider.ConsumeRandomLengthString();
            providerObj->SetVolumeDescription(volId, description);
            break;
        }
        default: break;
    }
}

static void HandleDiskFixUsbOps(FuzzedDataProvider& provider,
                                OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_TRY_TO_FIX: {
            std::string volId = provider.ConsumeRandomLengthString();
            uint32_t flags = provider.ConsumeIntegral<uint32_t>();
            providerObj->TryToFix(volId, flags);
            break;
        }
        case FUNC_QUERY_USB_IS_IN_USE: {
            std::string diskPath = provider.ConsumeRandomLengthString();
            bool isInUse;
            providerObj->QueryUsbIsInUse(diskPath, isInUse);
            break;
        }
        default: break;
    }
}

static void HandleUserDirOps(FuzzedDataProvider& provider,
                             OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                             StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_START_USER: {
            int userId = provider.ConsumeIntegral<int>();
            providerObj->StartUser(userId);
            break;
        }
        case FUNC_STOP_USER: {
            int userId = provider.ConsumeIntegral<int>();
            providerObj->StopUser(userId);
            break;
        }
        case FUNC_PREPARE_USER_DIRS: {
            int userId = provider.ConsumeIntegral<int>();
            uint32_t flags = provider.ConsumeIntegral<uint32_t>();
            providerObj->PrepareUserDirs(userId, flags);
            break;
        }
        case FUNC_DESTROY_USER_DIRS: {
            int userId = provider.ConsumeIntegral<int>();
            if (userId == DEFAULT_USERID) {
                break;
            }
            uint32_t flags = provider.ConsumeIntegral<uint32_t>();
            providerObj->DestroyUserDirs(userId, flags);
            break;
        }
        case FUNC_COMPLETE_ADD_USER: {
            int userId = provider.ConsumeIntegral<int>();
            providerObj->CompleteAddUser(userId);
            break;
        }
        case FUNC_CREATE_USER_DIR: {
            std::string path = provider.ConsumeRandomLengthString();
            uint32_t mode = provider.ConsumeIntegral<uint32_t>();
            uint32_t uid = provider.ConsumeIntegral<uint32_t>();
            uint32_t gid = provider.ConsumeIntegral<uint32_t>();
            providerObj->CreateUserDir(path, mode, uid, gid);
            break;
        }
        default: break;
    }
}

static void HandleGlobalKeyOps(FuzzedDataProvider& provider,
                               OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                               StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_INIT_GLOBAL_KEY: providerObj->InitGlobalKey(); break;
        case FUNC_INIT_GLOBAL_USER_KEYS: providerObj->InitGlobalUserKeys(); break;
        case FUNC_GENERATE_USER_KEYS: {
            break;
        }
        default: break;
    }
}

static void HandleUserAuthOps(FuzzedDataProvider& provider,
                              OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                              StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_UPDATE_USER_AUTH: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            uint64_t secureUid = provider.ConsumeIntegral<uint64_t>();
            std::vector<uint8_t> token =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> oldSecret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> newSecret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->UpdateUserAuth(userId, secureUid, token, oldSecret, newSecret);
            break;
        }
        case FUNC_ACTIVE_USER_KEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            std::vector<uint8_t> token =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> secret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->ActiveUserKey(userId, token, secret);
            break;
        }
        case FUNC_INACTIVE_USER_KEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            providerObj->InactiveUserKey(userId);
            break;
        }
        default: break;
    }
}

static void HandleKeyContextOps(FuzzedDataProvider& provider,
                                OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_UPDATE_KEY_CONTEXT: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            bool needRemoveTmpKey = provider.ConsumeBool();
            providerObj->UpdateKeyContext(userId, needRemoveTmpKey);
            break;
        }
        case FUNC_MOUNT_CRYPTO_PATH_AGAIN: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            providerObj->MountCryptoPathAgain(userId);
            break;
        }
        default: break;
    }
}

static void HandleScreenLockOps(FuzzedDataProvider& provider,
                                OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_LOCK_USER_SCREEN: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            providerObj->LockUserScreen(userId);
            break;
        }
        case FUNC_UNLOCK_USER_SCREEN: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            std::vector<uint8_t> token =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> secret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->UnlockUserScreen(userId, token, secret);
            break;
        }
        case FUNC_GET_LOCK_SCREEN_STATUS: {
            uint32_t user = provider.ConsumeIntegral<uint32_t>();
            bool lockScreenStatus;
            providerObj->GetLockScreenStatus(user, lockScreenStatus);
            break;
        }
        default: break;
    }
}

static void HandleRecoveryKeyOps(FuzzedDataProvider& provider,
                                 OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                 StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_UPDATE_USE_AUTH_WITH_RECOVERY_KEY: {
            std::vector<uint8_t> authToken =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> newSecret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            uint64_t secureUid = provider.ConsumeIntegral<uint64_t>();
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            std::vector<std::vector<uint8_t>> plainText;
            int outerSize = provider.ConsumeIntegralInRange(0, 10);
            for (int i = 0; i < outerSize; ++i) {
                int innerSize = provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT);
                plainText.push_back(provider.ConsumeBytes<uint8_t>(innerSize));
            }
            providerObj->UpdateUseAuthWithRecoveryKey(authToken, newSecret, secureUid, userId, plainText);
            break;
        }
        case FUNC_CREATE_RECOVER_KEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            uint32_t userType = provider.ConsumeIntegral<uint32_t>();
            std::vector<uint8_t> token =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            std::vector<uint8_t> secret =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->CreateRecoverKey(userId, userType, token, secret);
            break;
        }
        case FUNC_SET_RECOVER_KEY: {
            std::vector<uint8_t> key =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->SetRecoverKey(key);
            break;
        }
        case FUNC_RESET_SECRET_WITH_RECOVERY_KEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            uint32_t rkType = provider.ConsumeIntegral<uint32_t>();
            std::vector<uint8_t> key =
                provider.ConsumeBytes<uint8_t>(provider.ConsumeIntegralInRange(0, MAX_TEST_COUNT));
            providerObj->ResetSecretWithRecoveryKey(userId, rkType, key);
            break;
        }
        default: break;
    }
}

static void HandleAppShareOps(FuzzedDataProvider& provider,
                              OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                              StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_GENERATE_APPKEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            uint32_t hashId = provider.ConsumeIntegral<uint32_t>();
            std::string keyId;
            bool needReSet = provider.ConsumeBool();
            providerObj->GenerateAppkey(userId, hashId, keyId, needReSet);
            break;
        }
        case FUNC_DELETE_APPKEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            std::string keyId = provider.ConsumeRandomLengthString();
            providerObj->DeleteAppkey(userId, keyId);
            break;
        }
        case FUNC_CREATE_SHARE_FILE: {
            StorageFileRawData rawData;
            uint32_t tokenId = provider.ConsumeIntegral<uint32_t>();
            uint32_t flag = provider.ConsumeIntegral<uint32_t>();
            std::vector<int32_t> res;
            providerObj->CreateShareFile(rawData, tokenId, flag, res);
            break;
        }
        case FUNC_DELETE_SHARE_FILE: {
            uint32_t tokenId = provider.ConsumeIntegral<uint32_t>();
            StorageFileRawData rawData;
            providerObj->DeleteShareFile(tokenId, rawData);
            break;
        }
        default: break;
    }
}

static void HandleQuotaMemoryOps(FuzzedDataProvider& provider,
                                 OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                 StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_SET_BUNDLE_QUOTA: {
            int uid = provider.ConsumeIntegral<int>();
            std::string bundleDataDirPath = provider.ConsumeRandomLengthString();
            int limitSizeMb = provider.ConsumeIntegral<int>();
            providerObj->SetBundleQuota(uid, bundleDataDirPath, limitSizeMb);
            break;
        }
        case FUNC_GET_OCCUPIED_SPACE: {
            int32_t idType = provider.ConsumeIntegral<int32_t>();
            int32_t id = provider.ConsumeIntegral<int32_t>();
            int64_t spaceSize;
            providerObj->GetOccupiedSpace(idType, id, spaceSize);
            break;
        }
        case FUNC_QUERY_OCCUPIED_SPACE_FOR_SA: {
            auto fuzzData = provider.ConsumeBytes<uint8_t>(provider.remaining_bytes());
            size_t fuzzDataSize = fuzzData.size();
            const uint8_t* dataPtr = fuzzData.data();
            if (dataPtr == nullptr || fuzzDataSize < sizeof(int32_t)) {
                break;
            }
            std::string storageStatus = provider.ConsumeRandomLengthString();
            int32_t key;
            int error = memcpy_s(&key, sizeof(key), dataPtr, sizeof(int32_t));
            if (error != 0) {
                break;
            }
            size_t valueOffset = std::min(sizeof(int32_t), fuzzDataSize);
            size_t valueLength = fuzzDataSize - valueOffset;
            if (valueLength == 0) {
                break;
            }
            std::string value(reinterpret_cast<const char*>(dataPtr + valueOffset), valueLength);
            std::map<int32_t, std::string> bundleNameAndUid{{key, value}};
            providerObj->QueryOccupiedSpaceForSa(storageStatus, bundleNameAndUid);
            break;
        }
        default: break;
    }
}

static void HandleDfsEncryptOps(FuzzedDataProvider& provider,
                                OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_MOUNT_DFS_DOCS: {
            int userId = provider.ConsumeIntegral<int>();
            std::string relativePath = provider.ConsumeRandomLengthString();
            std::string networkId = provider.ConsumeRandomLengthString();
            std::string deviceId = provider.ConsumeRandomLengthString();
            providerObj->MountDfsDocs(userId, relativePath, networkId, deviceId);
            break;
        }
        case FUNC_UMOUNT_DFS_DOCS: {
            int userId = provider.ConsumeIntegral<int>();
            std::string relativePath = provider.ConsumeRandomLengthString();
            std::string networkId = provider.ConsumeRandomLengthString();
            std::string deviceId = provider.ConsumeRandomLengthString();
            providerObj->UMountDfsDocs(userId, relativePath, networkId, deviceId);
            break;
        }
        case FUNC_GET_FILE_ENCRYPT_STATUS: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            bool isEncrypted;
            bool needCheckDirMount = provider.ConsumeBool();
            providerObj->GetFileEncryptStatus(userId, isEncrypted, needCheckDirMount);
            break;
        }
        case FUNC_GET_USER_NEED_ACTIVE_STATUS: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            bool needActive;
            providerObj->GetUserNeedActiveStatus(userId, needActive);
            break;
        }
        default: break;
    }
}

static void HandleFuseMountOps(FuzzedDataProvider& provider,
                               OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                               StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_MOUNT_MEDIA_FUSE: {
            int userId = provider.ConsumeIntegral<int>();
            int devFd = -1;
            providerObj->MountMediaFuse(userId, devFd);
            break;
        }
        case FUNC_UMOUNT_MEDIA_FUSE: {
            int userId = provider.ConsumeIntegral<int>();
            if (userId == DEFAULT_USERID) {
                break;
            }
            providerObj->UMountMediaFuse(userId);
            break;
        }
        case FUNC_MOUNT_FILE_MGR_FUSE: {
            int userId = provider.ConsumeIntegral<int>();
            std::string path = provider.ConsumeRandomLengthString();
            int fuseFd = -1;
            providerObj->MountFileMgrFuse(userId, path, fuseFd);
            break;
        }
        case FUNC_UMOUNT_FILE_MGR_FUSE: {
            int userId = provider.ConsumeIntegral<int>();
            if (userId == DEFAULT_USERID) {
                break;
            }
            std::string path = provider.ConsumeRandomLengthString();
            providerObj->UMountFileMgrFuse(userId, path);
            break;
        }
        case FUNC_MOUNT_USB_FUSE: {
            std::string volumeId = provider.ConsumeRandomLengthString();
            std::string fsUuid = provider.ConsumeRandomLengthString();
            int fuseFd = -1;
            providerObj->MountUsbFuse(volumeId, fsUuid, fuseFd);
            break;
        }
        default: break;
    }
}

static void HandleFileAncoOps(FuzzedDataProvider& provider,
                              OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                              StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_IS_FILE_OCCUPIED: {
            std::string path = provider.ConsumeRandomLengthString();
            std::vector<std::string> inputList;
            int inputSize = provider.ConsumeIntegralInRange(0, 10);
            for (int i = 0; i < inputSize; ++i) {
                inputList.push_back(provider.ConsumeRandomLengthString());
            }
            std::vector<std::string> outputList;
            bool isOccupy;
            providerObj->IsFileOccupied(path, inputList, outputList, isOccupy);
            break;
        }
        case FUNC_PREPARE_ANCO_USER_DIRS: {
            break;
        }
        case FUNC_VERIFY_ANCO_USER_DIRS: {
            break;
        }
        case FUNC_GET_ANCO_SIZE_DATA: {
            std::string outExtraData;
            providerObj->GetAncoSizeData(outExtraData);
            break;
        }
        default: break;
    }
}

static void HandleDisShareUeceOps(FuzzedDataProvider& provider,
                                  OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                  StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_MOUNT_DIS_SHARE_FILE: {
            int userId = provider.ConsumeIntegral<int>();
            if (userId == DEFAULT_USERID) {
                break;
            }
            std::map<std::string, std::string> shareFiles;
            int mapSize = provider.ConsumeIntegralInRange(0, 5);
            for (int i = 0; i < mapSize; ++i) {
                std::string key = provider.ConsumeRandomLengthString();
                std::string value = provider.ConsumeRandomLengthString();
                shareFiles[key] = value;
            }
            providerObj->MountDisShareFile(userId, shareFiles);
            break;
        }
        case FUNC_UMOUNT_DIS_SHARE_FILE: {
            int userId = provider.ConsumeIntegral<int>();
            if (userId == DEFAULT_USERID) {
                break;
            }
            std::string networkId = provider.ConsumeRandomLengthString();
            providerObj->UMountDisShareFile(userId, networkId);
            break;
        }
        case FUNC_REGISTER_UEEC_ACTIVATION_CALLBACK: {
            sptr<IUeceActivationCallback> callback = GetMockUeceActivationCallbackStub();
            providerObj->RegisterUeceActivationCallback(callback);
            break;
        }
        case FUNC_UNREGISTER_UEEC_ACTIVATION_CALLBACK:
            providerObj->UnregisterUeceActivationCallback();
            break;
        default: break;
    }
}

static void HandlePublicDirEncryptOps(FuzzedDataProvider& provider,
                                      OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                      StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_INACTIVE_USER_PUBLIC_DIR_KEY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            providerObj->InactiveUserPublicDirKey(userId);
            break;
        }
        case FUNC_UPDATE_USER_PUBLIC_DIR_POLICY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            providerObj->UpdateUserPublicDirPolicy(userId);
            break;
        }
        case FUNC_SET_DIR_ENCRYPTION_POLICY: {
            uint32_t userId = provider.ConsumeIntegral<uint32_t>();
            std::string dirPath = provider.ConsumeRandomLengthString();
            uint32_t level = provider.ConsumeIntegral<uint32_t>();
            providerObj->SetDirEncryptionPolicy(userId, dirPath, level);
            break;
        }
        default: break;
    }
}

static void HandleSpaceScanOps(FuzzedDataProvider& provider,
                               OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                               StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_GET_DQ_BLK_SPACES_BY_UIDS: {
            size_t uidCount = provider.ConsumeIntegralInRange<size_t>(0, 10);
            std::vector<int32_t> uids;
            uids.reserve(uidCount);
            for (size_t i = 0; i < uidCount; i++) {
                uids.push_back(provider.ConsumeIntegral<int32_t>());
            }
            std::vector<NextDqBlk> dqBlks;
            providerObj->GetDqBlkSpacesByUids(uids, dqBlks);
            break;
        }
        case FUNC_GET_DIR_LIST_SPACE: {
            size_t dirCount = provider.ConsumeIntegralInRange<size_t>(0, 5);
            std::vector<DirSpaceInfo> inDirs;
            for (size_t i = 0; i < dirCount; i++) {
                DirSpaceInfo dirInfo;
                dirInfo.path = provider.ConsumeRandomLengthString();
                inDirs.push_back(dirInfo);
            }
            std::vector<DirSpaceInfo> outDirs;
            providerObj->GetDirListSpace(inDirs, outDirs);
            break;
        }
        case FUNC_SET_STOP_SCAN_FLAG: {
            bool stop = provider.ConsumeBool();
            providerObj->SetStopScanFlag(stop);
            break;
        }
        default: break;
    }
}

static void HandleDataStatsOps(FuzzedDataProvider& provider,
                               OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                               StorageDaemonFunction code)
{
    switch (code) {
        case FUNC_GET_DATA_SIZE_BY_PATH: {
            std::string path = provider.ConsumeRandomLengthString();
            int64_t dataSize = 0;
            providerObj->GetDataSizeByPath(path, dataSize);
            break;
        }
        case FUNC_GET_RMG_RESOURCE_SIZE: {
            std::string rgmName = provider.ConsumeRandomLengthString();
            uint64_t totalSize = 0;
            providerObj->GetRmgResourceSize(rgmName, totalSize);
            break;
        }
        default: break;
    }
}

static void DispatchStorageDaemonFunction(FuzzedDataProvider& provider,
                                          OHOS::StorageDaemon::StorageDaemonProvider* providerObj,
                                          int code)
{
    StorageDaemonFunction funcCode = static_cast<StorageDaemonFunction>(code);
    
    struct {
        StorageDaemonFunction start;
        StorageDaemonFunction end;
        void (*handler)(FuzzedDataProvider&, StorageDaemonProvider*, StorageDaemonFunction);
    } ranges[] = {
        {FUNC_SHUTDOWN, FUNC_CHECK, HandleBasicStorageOps},
        {FUNC_FORMAT, FUNC_SET_VOLUME_DESCRIPTION, HandleDiskFormatOps},
        {FUNC_TRY_TO_FIX, FUNC_QUERY_USB_IS_IN_USE, HandleDiskFixUsbOps},
        {FUNC_START_USER, FUNC_CREATE_USER_DIR, HandleUserDirOps},
        {FUNC_INIT_GLOBAL_KEY, FUNC_DELETE_USER_KEYS, HandleGlobalKeyOps},
        {FUNC_UPDATE_USER_AUTH, FUNC_INACTIVE_USER_KEY, HandleUserAuthOps},
        {FUNC_UPDATE_KEY_CONTEXT, FUNC_MOUNT_CRYPTO_PATH_AGAIN, HandleKeyContextOps},
        {FUNC_LOCK_USER_SCREEN, FUNC_GET_LOCK_SCREEN_STATUS, HandleScreenLockOps},
        {FUNC_UPDATE_USE_AUTH_WITH_RECOVERY_KEY, FUNC_RESET_SECRET_WITH_RECOVERY_KEY, HandleRecoveryKeyOps},
        {FUNC_GENERATE_APPKEY, FUNC_DELETE_SHARE_FILE, HandleAppShareOps},
        {FUNC_SET_BUNDLE_QUOTA, FUNC_UPDATE_MEMORY_PARA, HandleQuotaMemoryOps},
        {FUNC_MOUNT_DFS_DOCS, FUNC_GET_USER_NEED_ACTIVE_STATUS, HandleDfsEncryptOps},
        {FUNC_MOUNT_MEDIA_FUSE, FUNC_MOUNT_USB_FUSE, HandleFuseMountOps},
        {FUNC_IS_FILE_OCCUPIED, FUNC_GET_ANCO_SIZE_DATA, HandleFileAncoOps},
        {FUNC_MOUNT_DIS_SHARE_FILE, FUNC_UNREGISTER_UEEC_ACTIVATION_CALLBACK, HandleDisShareUeceOps},
        {FUNC_INACTIVE_USER_PUBLIC_DIR_KEY, FUNC_SET_DIR_ENCRYPTION_POLICY, HandlePublicDirEncryptOps},
        {FUNC_GET_DQ_BLK_SPACES_BY_UIDS, FUNC_SET_STOP_SCAN_FLAG, HandleSpaceScanOps},
        {FUNC_GET_DATA_SIZE_BY_PATH, FUNC_GET_RMG_RESOURCE_SIZE, HandleDataStatsOps}
    };

    for (const auto& range : ranges) {
        if (funcCode >= range.start && funcCode <= range.end) {
            range.handler(provider, providerObj, funcCode);
            return;
        }
    }
    return;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    FuzzedDataProvider provider(data, size);
    
    if (storageDaemonProvider == nullptr) {
        return 0;
    }
    int code = provider.PickValueInArray(IPC_CODES);

    DispatchStorageDaemonFunction(provider, storageDaemonProvider, code);
    return 0;
}