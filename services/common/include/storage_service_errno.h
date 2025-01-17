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
#ifndef STORAGE_SERVICE_UTILS_ERRNO_H
#define STORAGE_SERVICE_UTILS_ERRNO_H

#include <sys/types.h>
#include <unordered_map>

namespace OHOS {
constexpr int32_t E_ERR = -1;
constexpr int32_t STORAGE_SERVICE_SYS_CAP_TAG = 13600000;

enum ErrNo {
    // 历史错误码（为保证兼容，禁止修改或新增）
    E_OK = 0,
    E_ACTIVE_EL2_FAILED = 30,

    // 通用错误码 13600001 ~ 13600200
    E_PERMISSION_DENIED = STORAGE_SERVICE_SYS_CAP_TAG + 1,
    E_PARAMS_INVALID = STORAGE_SERVICE_SYS_CAP_TAG + 2,
    E_PARAMS_NULLPTR_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 3,
    E_NON_EXIST = STORAGE_SERVICE_SYS_CAP_TAG + 4,
    E_PREPARE_DIR = STORAGE_SERVICE_SYS_CAP_TAG + 5,
    E_DESTROY_DIR = STORAGE_SERVICE_SYS_CAP_TAG + 6,
    E_NOT_SUPPORT = STORAGE_SERVICE_SYS_CAP_TAG + 7,
    E_SYS_KERNEL_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 8,
    E_WRITE_PARCEL_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 9,
    E_WRITE_REPLY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 10,
    E_WRITE_DESCRIPTOR_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 11,
    E_SA_IS_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 12,
    E_REMOTE_IS_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 13,
    E_SERVICE_IS_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 14,
    E_MEMORY_OPERATION_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 15,
    E_DEATHRECIPIENT_IS_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 16,
    E_CREATE_PIPE = STORAGE_SERVICE_SYS_CAP_TAG + 17,
    E_FORK = STORAGE_SERVICE_SYS_CAP_TAG + 18,
    E_WIFEXITED = STORAGE_SERVICE_SYS_CAP_TAG + 19,
    E_WEXITSTATUS = STORAGE_SERVICE_SYS_CAP_TAG + 20,

    // 密钥管理 13600201 ~ 13600700
    E_SET_POLICY = STORAGE_SERVICE_SYS_CAP_TAG + 201,
    E_USERID_RANGE = STORAGE_SERVICE_SYS_CAP_TAG + 202,
    E_KEY_TYPE_INVALID = STORAGE_SERVICE_SYS_CAP_TAG + 203,
    E_MIGRETE_ELX_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 204,
    E_GLOBAL_KEY_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 205,
    E_GLOBAL_KEY_INIT_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 206,
    E_GLOBAL_KEY_STORE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 207,
    E_GLOBAL_KEY_ACTIVE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 208,
    E_GLOBAL_KEY_UPDATE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 209,
    E_ELX_KEY_INIT_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 210,
    E_ELX_KEY_STORE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 211,
    E_ELX_KEY_ACTIVE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 212,
    E_ELX_KEY_UPDATE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 213,
    E_EL5_ADD_CLASS_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 214,
    E_EL5_ENCRYPT_CLASS_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 215,
    E_EL5_UPDATE_CLASS_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 216,
    E_EL5_DELETE_CLASS_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 217,
    E_ELX_KEY_INACTIVE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 218,
    E_CLEAR_KEY_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 219,
    E_KEY_NOT_ACTIVED = STORAGE_SERVICE_SYS_CAP_TAG + 220,
    E_RESTORE_KEY_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 221,
    E_UNLOCK_SCREEN_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 222,
    E_UNLOCK_APP_KEY2_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 223,
    E_JSON_PARSE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 224,
    E_OPEN_JSON_FILE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 225,
    E_FILE_VERIFY_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 226,
    E_HCF_OPERATION_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 227,
    E_CREATE_DIR_RECURSIVE_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 228,
    E_CHECK_USER_PIN_PROTECT_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 229,
    E_TRY_TO_FIX_USER_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 230,
    E_EL5_GENERATE_APP_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 231,
    E_EL5_DELETE_APP_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 232,
    E_RECOVERY_KEY_OPEN_SESSION_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 233,
    E_RECOVERY_KEY_GEN_KEY_DESC_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 234,
    E_RECOVERY_KEY_INS_KEY_DESC_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 235,
    E_TEEC_GEN_RECOVERY_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 236,
    E_TEEC_DECRYPT_CLASS_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 237,
    E_SET_RECOVERY_KEY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 238,
    E_LOAD_AND_SET_POLICY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 239,
    E_LOAD_AND_SET_ECE_POLICY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 240,

    E_DIR_INTEGRITY_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 350,
    E_KEY_SIZE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 351,
    E_ADD_SESSION_KEYRING_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 352,
    E_SAVE_KEY_BLOB_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 353,
    E_SEARCH_SESSION_KEYING_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 354,
    E_KEY_EMPTY_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 355,
    E_KEY_LOAD_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 356,
    E_KEY_CTX_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 357,
    E_KEY_BLOB_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 358,
    E_GEN_HUKS_PARAM_ERROR =  STORAGE_SERVICE_SYS_CAP_TAG + 359,
    E_INSTALL_KEYDESC_KEYRING_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 360,
    E_SHIELD_OPERATION_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 361,
    E_LOAD_KEY_BLOB_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 362,
    E_GENERATE_DISCARD_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 363,
    E_SAVE_KEY_TYPE_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 364,

    // 用户管理 13600701 ~ 13601200
    E_USER_MOUNT_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 701,
    E_USER_UMOUNT_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 702,
    E_DIFF_UID_GID = STORAGE_SERVICE_SYS_CAP_TAG + 703,
    E_UMOUNT_CLOUD_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 704,
    E_UMOUNT_CLOUD = STORAGE_SERVICE_SYS_CAP_TAG + 705,
    E_UMOUNT_MEDIA_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 706,
    E_UMOUNT_SHAREFS = STORAGE_SERVICE_SYS_CAP_TAG + 707,
    E_UMOUNT_HMFS = STORAGE_SERVICE_SYS_CAP_TAG + 708,
    E_UMOUNT_HMDFS = STORAGE_SERVICE_SYS_CAP_TAG + 709,
    E_UMOUNT_LOCAL = STORAGE_SERVICE_SYS_CAP_TAG + 710,
    E_UMOUNT_PROC_MOUNTS_OPEN = STORAGE_SERVICE_SYS_CAP_TAG + 711,
    E_UMOUNT_ALL_PATH = STORAGE_SERVICE_SYS_CAP_TAG + 712,
    E_UMOUNT_PROC_OPEN = STORAGE_SERVICE_SYS_CAP_TAG + 713,
    E_UMOUNT_DETACH = STORAGE_SERVICE_SYS_CAP_TAG + 714,
    E_UMOUNT_NO_PROCESS_FIND = STORAGE_SERVICE_SYS_CAP_TAG + 715,
    E_UMOUNT_PROCESS_KILL = STORAGE_SERVICE_SYS_CAP_TAG + 716,
    E_UMOUNT_FIND_FD = STORAGE_SERVICE_SYS_CAP_TAG + 717,
    E_NOT_EMPTY_TO_UMOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 718,
    E_MOUNT_LOCAL_MEDIA = STORAGE_SERVICE_SYS_CAP_TAG + 719,
    E_MOUNT_LOCAL_CLOUD = STORAGE_SERVICE_SYS_CAP_TAG + 720,
    E_MOUNT_HMDFS = STORAGE_SERVICE_SYS_CAP_TAG + 721,
    E_MOUNT_HMDFS_MEDIA = STORAGE_SERVICE_SYS_CAP_TAG + 722,
    E_MOUNT_HMDFS_CLOUD = STORAGE_SERVICE_SYS_CAP_TAG + 723,
    E_MOUNT_HMDFS_CLOUD_DOCS = STORAGE_SERVICE_SYS_CAP_TAG + 724,
    E_MOUNT_CLOUD_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 725,
    E_MOUNT_CLOUD = STORAGE_SERVICE_SYS_CAP_TAG + 726,
    E_MOUNT_SHAREFS = STORAGE_SERVICE_SYS_CAP_TAG + 727,
    E_MOUNT_HM_SHAREFS = STORAGE_SERVICE_SYS_CAP_TAG + 728,
    E_MOUNT_BIND_AND_REC = STORAGE_SERVICE_SYS_CAP_TAG + 729,
    E_CREATE_DIR_FILE_MANAGER = STORAGE_SERVICE_SYS_CAP_TAG + 730,
    E_CREATE_DIR_VIRTUAL = STORAGE_SERVICE_SYS_CAP_TAG + 731,
    E_CREATE_DIR_HMDFS = STORAGE_SERVICE_SYS_CAP_TAG + 732,
    E_CREATE_DIR_SA = STORAGE_SERVICE_SYS_CAP_TAG + 733,
    E_CREATE_DIR_APPDATA = STORAGE_SERVICE_SYS_CAP_TAG + 734,
    E_CREATE_DIR_APPDATA_OTHER = STORAGE_SERVICE_SYS_CAP_TAG + 735,
    E_CREATE_DIR_APPDATA_FILEMGR = STORAGE_SERVICE_SYS_CAP_TAG + 736,
    E_UMOUNT_LOCAL_MEDIA = STORAGE_SERVICE_SYS_CAP_TAG + 737,
    E_UMOUNT_LOCAL_CLOUD = STORAGE_SERVICE_SYS_CAP_TAG + 738,
    E_UMOUNT_FIND_PROCESS = STORAGE_SERVICE_SYS_CAP_TAG + 739,
    E_MOUNT_SANDBOX = STORAGE_SERVICE_SYS_CAP_TAG + 740,
    E_MOUNT_MEDIA_FUSE = STORAGE_SERVICE_SYS_CAP_TAG + 741,
    E_MOUNT_SHARED = STORAGE_SERVICE_SYS_CAP_TAG + 742,
    E_MOUNT_BIND_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 743,
    E_CLOUD_NOT_READY = STORAGE_SERVICE_SYS_CAP_TAG + 744,

    // 空间统计 13601201 ~ 13601700
    E_BUNDLEMGR_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 1201,
    E_MEDIALIBRARY_ERROR = STORAGE_SERVICE_SYS_CAP_TAG + 1202,
    E_STORAGE_USAGE_NOT_ENOUGH = STORAGE_SERVICE_SYS_CAP_TAG + 1203,
    E_STATVFS = STORAGE_SERVICE_SYS_CAP_TAG + 1204,
    E_APPINDEX_RANGE = STORAGE_SERVICE_SYS_CAP_TAG + 1205,
    E_QUERY = STORAGE_SERVICE_SYS_CAP_TAG + 1206,
    E_GETROWCOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1207,

    // 外卡管理 13601701 ~ 13602200
    E_VOL_STATE = STORAGE_SERVICE_SYS_CAP_TAG + 1701,
    E_VOL_MOUNT_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 1702,
    E_VOL_UMOUNT_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 1703,
    E_UMOUNT_BUSY = STORAGE_SERVICE_SYS_CAP_TAG + 1704,
    E_NO_CHILD = STORAGE_SERVICE_SYS_CAP_TAG + 1705,
    E_MTP_IS_MOUNTING = STORAGE_SERVICE_SYS_CAP_TAG + 1706,
    E_MTP_PREPARE_DIR_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 1707,
    E_MTP_MOUNT_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 1708,
    E_MTP_UMOUNT_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 1709,
    E_EXT_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1710,
    E_NTFS_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1711,
    E_EXFAT_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1712,
    E_FAT_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1713,
    E_HMFS_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1714,
    E_OTHER_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1715,
    E_DOCHECK_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1716,
    E_MKDIR_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1717,
    E_RMDIR_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1718,
    E_TIMEOUT_MOUNT = STORAGE_SERVICE_SYS_CAP_TAG + 1719,
    E_READMETADATA = STORAGE_SERVICE_SYS_CAP_TAG + 1720,
    E_UNSUPPORTED = STORAGE_SERVICE_SYS_CAP_TAG + 1721,
    E_CHECK = STORAGE_SERVICE_SYS_CAP_TAG + 1722,
    E_VOLUMEEX_IS_NULLPTR = STORAGE_SERVICE_SYS_CAP_TAG + 1723,

    // 文件分享/分布式文件/备份扫描等 13602201 ~
    E_CREATE_SHARE_FILE_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 2201,
    E_DELETE_SHARE_FILE_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 2202,
    E_INIT_QUOTA_MOUNTS_FAILED = STORAGE_SERVICE_SYS_CAP_TAG + 2203,
    E_QUOTA_CTL_KERNEL_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 2204,
    E_STAT_VFS_KERNEL_ERR = STORAGE_SERVICE_SYS_CAP_TAG + 2205,
};

enum JsErrCode {
    E_PERMISSION = 201,
    E_PERMISSION_SYS = 202,
    E_PARAMS = 401,
    E_DEVICENOTSUPPORT = 801,
    E_OSNOTSUPPORT = 901,
    E_IPCSS = STORAGE_SERVICE_SYS_CAP_TAG + 1,
    E_SUPPORTEDFS,
    E_MOUNT_ERR,
    E_UNMOUNT,
    E_VOLUMESTATE,
    E_PREPARE,
    E_DELETE,
    E_NOOBJECT,
    E_OUTOFRANGE,
};

}
#endif // STORAGE_SERVICE_UTILS_ERRNO_H