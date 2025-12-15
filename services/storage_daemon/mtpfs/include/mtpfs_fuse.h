/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MTPFS_FUSE_H
#define MTPFS_FUSE_H

#include "fuse.h"
#include <mutex>
#include <singleton.h>

#include "mtpfs_mtp_device.h"
#include "mtpfs_tmp_files_pool.h"
#include "mtpfs_type_tmp_file.h"
#include "os_account_manager.h"
#include "storage_service_errno.h"

using namespace OHOS;

class AccountSubscriber final : public OHOS::AccountSA::OsAccountSubscriber {
public:
    AccountSubscriber(const OHOS::AccountSA::OsAccountSubscribeInfo &info)
        : OHOS::AccountSA::OsAccountSubscriber(info){};
    void OnStateChanged(const OHOS::AccountSA::OsAccountStateData &data) override;
};

class AccountConstraintSubscriber final : public OHOS::AccountSA::OsAccountConstraintSubscriber {
public:
    explicit AccountConstraintSubscriber(const std::set<std::string> &constraintSet)
        : OHOS::AccountSA::OsAccountConstraintSubscriber(constraintSet){};
    void OnConstraintChanged(const OHOS::AccountSA::OsAccountConstraintStateData &constraintData) override;
};

class MtpFileSystem {
private:
    struct MtpFileSystemOptions {
    public:
        int good_;
        int verBose_;
        int enableMove_;
        int deviceNo_;
        char *deviceFile_;
        char *mountPoint_;

        MtpFileSystemOptions();
        ~MtpFileSystemOptions();

        static int OptProc(void *data, const char *arg, int key, struct fuse_args *outargs);
    };

    enum {
        KEY_ENABLE_MOVE,
        KEY_DEVICE_NO,
        KEY_LIST_DEVICES,
        KEY_VERBOSE,
        KEY_VERSION,
        KEY_HELP
    };

public:
    static MtpFileSystem &GetInstance()
    {
        static MtpFileSystem instance;
        return instance;
    }
    bool ParseOptions(int argc, char **argv);
    bool Exec();
    bool IsGood() const
    {
        return options_.good_;
    }
    int GetAttr(const char *path, struct stat *buf);
    int MkNod(const char *path, mode_t mode, dev_t dev);
    int MkDir(const char *path, mode_t mode);
    int UnLink(const char *path);
    int RmDir(const char *path);
    int ReName(const char *path, const char *newpath, unsigned int flags);
    int ChMods(const char *path, mode_t mode, struct fuse_file_info *fi);
    int Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi);
    int UTimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi);
    int OpenFile(const char *path, struct fuse_file_info *fileInfo);
    int ReadFile(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
    int OpenThumb(const char *path, struct fuse_file_info *fileInfo);
    int ReadThumb(const std::string &path, char *buf);
    int Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
    int Statfs(const char *path, struct statvfs *statInfo);
    int Flush(const char *path, struct fuse_file_info *fileInfo);
    int Release(const char *path, struct fuse_file_info *fileInfo);
    int FSync(const char *path, int datasync, struct fuse_file_info *fi);
    int OpenDir(const char *path, struct fuse_file_info *fileInfo);
    int ReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo,
        enum fuse_readdir_flags flag);
    int ReleaseDir(const char *path, struct fuse_file_info *fileInfo);
    int FSyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo);
    int Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo);
    void *Init(struct fuse_conn_info *conn, struct fuse_config *cfg);
    int Create(const char *path, mode_t mode, fuse_file_info *fileInfo);
    int SetXAttr(const char *path, const char *in, const char *out = nullptr);
    int GetXAttr(const char *path, const char *in, char *out, size_t size);
    int GetThumbAttr(const std::string &path, struct stat *buf);
    void HandleRemove(uint32_t handleId);
    void InitCurrentUidAndCacheMap();
    bool IsCurrentUserReadOnly();
    void SetCurrentUid(int32_t uid);
    void SetMtpClientWriteMap(uid_t first, bool second);
    MtpFsTmpFilesPool* GetTempFilesPool();

private:
    MtpFileSystem();
    ~MtpFileSystem();
    bool HasGetPartialSupport();
    bool HasSendPartialSupport();
    bool ParseOptionsInner();
    int GetFriendlyName(const char *in, char *out, size_t size);
    int HandleTemporaryFile(const std::string stdPath, struct fuse_file_info *fileInfo);
    int SetupFileAttributes(const char *path, const MtpFsTypeFile *file, struct stat *buf);
    struct fuse_args args_;
    struct fuse_operations fuseOperations_;
    MtpFsTmpFilesPool tmpFilesPool_;
    MtpFileSystemOptions options_;
    MtpFsDevice device_;
    std::mutex fuseMutex_;
    std::map<std::string, const MtpFsTypeDir *> dirMap_ {};
    std::mutex mtpClientMutex_;
    int32_t currentUid = 0;
    std::map<uid_t, bool> mtpClientWriteMap_ {};
};

#endif // MTPFS_FUSE_H
