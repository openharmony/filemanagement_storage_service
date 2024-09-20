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

#include "mtpfs_fuse.h"

#include <fuse_opt.h>
#include <unistd.h>

#include "mtpfs_util.h"
#include "storage_service_log.h"

#define PERMISSION_ONE 0775
#define PERMISSION_TWO 0644

const int32_t ST_NLINK_TWO = 2;
const int32_t FILE_SIZE = 512;
const int32_t BS_SIZE = 1024;
const int32_t ARG_SIZE = 2;

int WrapGetattr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    LOGI("mtp WrapGetattr");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->GetAttr(path, buf, fi);
}

int WrapMkNod(const char *path, mode_t mode, dev_t dev)
{
    LOGI("mtp WrapMkNod");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->MkNod(path, mode, dev);
}

int WrapMkDir(const char *path, mode_t mode)
{
    LOGI("mtp WrapMkDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->MkDir(path, mode);
}

int WrapUnLink(const char *path)
{
    LOGI("mtp WrapUnLink");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->UnLink(path);
}

int WrapRmDir(const char *path)
{
    LOGI("mtp WrapRmDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->RmDir(path);
}

int WrapReName(const char *path, const char *newpath, unsigned int flags)
{
    LOGI("mtp WrapReName");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->ReName(path, newpath, flags);
}

int WrapChMod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    LOGI("mtp WrapChMod");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->ChMods(path, mode, fi);
}

int WrapChown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    LOGE("mtp WrapChown path:%{public}s ,uid:%{public}lu, gid:%{public}lu", path, uid, gid);
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Chown(path, uid, gid, fi);
}


int WrapUTimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi)
{
    LOGI("mtp WrapUTimens");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->UTimens(path, tv, fi);
}

int WrapOpen(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapOpen");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Open(path, fileInfo);
}

int WrapRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapRead");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Read(path, buf, size, offset, fileInfo);
}

int WrapWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapWrite");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Write(path, buf, size, offset, fileInfo);
}

int WrapStatfs(const char *path, struct statvfs *statInfo)
{
    LOGI("mtp WrapStatfs");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Statfs(path, statInfo);
}

int WrapFlush(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFlush");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Flush(path, fileInfo);
}

int WrapRelease(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapRelease");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Release(path, fileInfo);
}

int WrapFSync(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFSync");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->FSync(path, datasync, fileInfo);
}

int WrapOpenDir(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapOpenDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->OpenDir(path, fileInfo);
}

int WrapReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo,
    enum fuse_readdir_flags flag)
{
    LOGI("mtp WrapReadDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->ReadDir(path, buf, filler, offset, fileInfo, flag);
}

int WrapReleaseDir(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapReleaseDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->ReleaseDir(path, fileInfo);
}

int WrapFSyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFSyncDir");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->FSyncDir(path, datasync, fileInfo);
}

void *WrapInit(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    LOGI("mtp WrapInit");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Init(conn, cfg);
}

int WrapCreate(const char *path, mode_t mode, fuse_file_info *fileInfo)
{
    LOGI("mtp WrapCreate");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Create(path, mode, fileInfo);
}

int WrapTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapTruncate");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Truncate(path, offset, fileInfo);
}

MtpFileSystem::MtpFileSystemOptions::MtpFileSystemOptions()
    : good_(false),
      verBose_(false),
      enableMove_(false),
      deviceNo_(1),
      deviceFile_(nullptr),
      mountPoint_(nullptr)
{}

MtpFileSystem::MtpFileSystemOptions::~MtpFileSystemOptions()
{
    free(static_cast<void *>(deviceFile_));
    free(static_cast<void *>(mountPoint_));
}

int MtpFileSystem::MtpFileSystemOptions::OptProc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
    MtpFileSystemOptions *options = static_cast<MtpFileSystemOptions *>(data);

    if (key == FUSE_OPT_KEY_NONOPT) {
        if (options->mountPoint_ && options->deviceFile_) {
            // Unknown positional argument supplied
            return -1;
        }
        if (options->deviceFile_) {
            fuse_opt_add_opt(&options->mountPoint_, arg);
            return 0;
        }

        fuse_opt_add_opt(&options->deviceFile_, arg);
        return 0;
    }
    return 1;
}

MtpFileSystem::MtpFileSystem() : args_(), tmpFilesPool_(), options_(), device_()
{
    LOGI("mtp MtpFileSystem");
    fuseOperations_.getattr = WrapGetattr;
    fuseOperations_.readlink = nullptr;
    fuseOperations_.mknod = WrapMkNod;
    fuseOperations_.mkdir = WrapMkDir;
    fuseOperations_.unlink = WrapUnLink;
    fuseOperations_.rmdir = WrapRmDir;
    fuseOperations_.symlink = nullptr;
    fuseOperations_.rename = WrapReName;
    fuseOperations_.link = nullptr;
    fuseOperations_.chmod = WrapChMod;
    fuseOperations_.chown = WrapChown;
    fuseOperations_.truncate = WrapTruncate;
    fuseOperations_.utimens = WrapUTimens;
    fuseOperations_.open = WrapOpen;
    fuseOperations_.read = WrapRead;
    fuseOperations_.write = WrapWrite;
    fuseOperations_.statfs = WrapStatfs;
    fuseOperations_.flush = WrapFlush;
    fuseOperations_.release = WrapRelease;
    fuseOperations_.fsync = WrapFSync;
    fuseOperations_.setxattr = nullptr;
    fuseOperations_.getxattr = nullptr;
    fuseOperations_.listxattr = nullptr;
    fuseOperations_.removexattr = nullptr;
    fuseOperations_.opendir = WrapOpenDir;
    fuseOperations_.readdir = WrapReadDir;
    fuseOperations_.releasedir = WrapReleaseDir;
    fuseOperations_.fsyncdir = WrapFSyncDir;
    fuseOperations_.init = WrapInit;
    fuseOperations_.destroy = nullptr;
    fuseOperations_.access = nullptr;
    fuseOperations_.create = WrapCreate;
}

MtpFileSystem::~MtpFileSystem()
{
    fuse_opt_free_args(&args_);
}

bool MtpFileSystem::ParseOptionsInner()
{
    fuse_opt_add_arg(&args_, options_.mountPoint_);
    fuse_opt_add_arg(&args_, "-s");

    if (options_.verBose_) {
        fuse_opt_add_arg(&args_, "-f");
    }

    --options_.deviceNo_;

    // device file and -- device are mutually exclusive, fail if both set
    if (options_.deviceNo_ && options_.deviceFile_) {
        options_.good_ = false;
        return false;
    }
    options_.good_ = true;
    return true;
}

bool MtpFileSystem::ParseOptions(int argc, char **argv)
{
    LOGI("mtp MtpFileSystem ParseOptions");
#define SMTPFS_OPT_KEY(t, p, v)                  \
    {                                            \
        t, offsetof(MtpFileSystemOptions, p), v \
    }

    static struct fuse_opt smtpfs_opts[] = {
        SMTPFS_OPT_KEY("enable-move", enableMove_, 1),
        SMTPFS_OPT_KEY("--device %i", deviceNo_, 0),
        SMTPFS_OPT_KEY("-v", verBose_, 1),
        SMTPFS_OPT_KEY("--verbose", verBose_, 1),

            FUSE_OPT_END
                                           };

    if (argc < ARG_SIZE) {
        LOGE("Wrong usage");
        options_.good_ = false;
        return false;
    }

    fuse_opt_free_args(&args_);
    args_ = FUSE_ARGS_INIT(argc, argv);
    if (fuse_opt_parse(&args_, &options_, smtpfs_opts, MtpFileSystemOptions::OptProc) == -1) {
        options_.good_ = false;
        return false;
    }

    if (options_.deviceFile_ && !options_.mountPoint_) {
        options_.mountPoint_ = options_.deviceFile_;
        options_.deviceFile_ = nullptr;
    }

    if (!options_.mountPoint_) {
        LOGE("Mount point missing");
        options_.good_ = false;
        return false;
    }
    if (!ParseOptionsInner()) {
        return false;
    }
    return true;
}

bool MtpFileSystem::Exec()
{
    if (!options_.good_) {
        return false;
    }

    if (!SmtpfsCheckDir(options_.mountPoint_)) {
        LOGE("Can not mount the device to %{public}s", options_.mountPoint_);
        return false;
    }

    if (!tmpFilesPool_.CreateTmpDir()) {
        LOGE("Can not create a temporary directory");
        return false;
    }

    if (options_.deviceFile_) {
        // Try to use device file first, if provided
        if (!device_.Connect(options_.deviceFile_)) {
            return false;
        }
    } else {
        // Connect to MTP device by order number, if no device file supplied
        if (!device_.Connect(options_.deviceNo_)) {
            return false;
        }
    }
    device_.EnableMove(options_.enableMove_);
    if (fuse_main(args_.argc, args_.argv, &fuseOperations_, nullptr) > 0) {
        return false;
    }
    device_.Disconnect();

    if (!tmpFilesPool_.RemoveTmpDir()) {
        LOGE("Can not remove a temporary directory");
        return false;
    }

    return true;
}

void *MtpFileSystem::Init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    return nullptr;
}

int MtpFileSystem::GetAttr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    LOGI("mtp GetAttr path:%{public}s", path);
    if (memset_s(buf, sizeof(struct stat), 0, sizeof(struct stat)) != EOK) {
        LOGE("memset stat fail");
    }
    struct fuse_context *fc = fuse_get_context();
    buf->st_uid = fc->uid;
    buf->st_gid = fc->gid;
    if (path == std::string("/")) {
        buf->st_mode = S_IFDIR | PERMISSION_ONE;
        buf->st_nlink = ST_NLINK_TWO;
        return 0;
    } else {
        std::string tmpPath(SmtpfsDirName(path));
        std::string tmpFile(SmtpfsBaseName(path));
        const MtpFsTypeDir *content = device_.DirFetchContent(tmpPath);
        if (!content) {
            return -ENOENT;
        }

        if (content->Dir(tmpFile)) {
            const MtpFsTypeDir *dir = content->Dir(tmpFile);
            buf->st_ino = dir->Id();
            buf->st_mode = S_IFDIR | PERMISSION_ONE;
            buf->st_nlink = ST_NLINK_TWO;
            buf->st_mtime = dir->ModificationDate();
        } else if (content->File(tmpFile)) {
            const MtpFsTypeFile *file = content->File(tmpFile);
            buf->st_ino = file->Id();
            buf->st_size = file->Size();
            buf->st_blocks = (file->Size() / FILE_SIZE) + (file->Size() % FILE_SIZE > 0 ? 1 : 0);
            buf->st_nlink = 1;
            buf->st_mode = S_IFREG | PERMISSION_TWO;
            buf->st_mtime = file->ModificationDate();
            buf->st_ctime = buf->st_mtime;
            buf->st_atime = buf->st_mtime;
        } else {
            return -ENOENT;
        }
    }

    return 0;
}

int MtpFileSystem::MkNod(const char *path, mode_t mode, dev_t dev)
{
    if (!S_ISREG(mode)) {
        return -EINVAL;
    }
    std::string tmpPath = tmpFilesPool_.MakeTmpPath(std::string(path));
    int rval = ::open(tmpPath.c_str(), O_CREAT | O_WRONLY, mode);
    if (rval < 0) {
        return -errno;
    }
    rval = ::close(rval);
    if (rval < 0) {
        return -errno;
    }
    rval = device_.FilePush(tmpPath, std::string(path));
    ::unlink(tmpPath.c_str());

    if (rval != 0) {
        return rval;
    }
    return 0;
}

int MtpFileSystem::MkDir(const char *path, mode_t mode)
{
    return device_.DirCreateNew(std::string(path));
}

int MtpFileSystem::UnLink(const char *path)
{
    return device_.FileRemove(std::string(path));
}

int MtpFileSystem::RmDir(const char *path)
{
    return device_.DirRemove(std::string(path));
}

int MtpFileSystem::ReName(const char *path, const char *newpath, unsigned int flags)
{
    const std::string tmpOldDirName(SmtpfsDirName(std::string(path)));
    const std::string tmpNewDirName(SmtpfsDirName(std::string(newpath)));
    if (tmpOldDirName == tmpNewDirName) {
        return device_.ReName(std::string(path), std::string(newpath));
    }
    if (!options_.enableMove_) {
        return -EPERM;
    }
    const std::string tmpFile = tmpFilesPool_.MakeTmpPath(std::string(newpath));
    int rval = device_.FilePull(std::string(path), tmpFile);
    if (rval != 0) {
        return -rval;
    }
    rval = device_.FilePush(tmpFile, std::string(newpath));
    if (rval != 0) {
        return -rval;
    }
    rval = device_.FileRemove(std::string(path));
    if (rval != 0) {
        return -rval;
    }
    return 0;
}

int MtpFileSystem::ChMods(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int res;

    if (fi) {
        res = fchmod(fi->fh, mode);
    } else {
        res = chmod(path, mode);
    }
    if (res == -1) {
        return -errno;
    }
    return 0;
}

int MtpFileSystem::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    LOGE("mtp Chown path:%{public}s ,uid:%{public}lu, gid:%{public}lu", path, uid, gid);
    int res;

    if (fi) {
        res = fchown(fi->fh, uid, gid);
    } else {
        res = lchown(path, uid, gid);
    }
    if (res == -1) {
        return -errno;
    }
    return 0;
}

int MtpFileSystem::Truncate(const char *path, off_t new_size, struct fuse_file_info *fileInfo)
{
    const std::string tmpPath = tmpFilesPool_.MakeTmpPath(std::string(path));
    int rval = device_.FilePull(std::string(path), tmpPath);
    if (rval != 0) {
        ::unlink(tmpPath.c_str());
        return -rval;
    }

    rval = ::truncate(tmpPath.c_str(), new_size);
    if (rval != 0) {
        int errnoTmp = errno;
        ::unlink(tmpPath.c_str());
        return -errnoTmp;
    }

    rval = device_.FileRemove(std::string(path));
    if (rval != 0) {
        ::unlink(tmpPath.c_str());
        return -rval;
    }

    rval = device_.FilePush(tmpPath, std::string(path));
    ::unlink(tmpPath.c_str());

    if (rval != 0) {
        return -rval;
    }
    return 0;
}

int MtpFileSystem::UTimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi)
{
    std::string tmpBaseName(SmtpfsBaseName(std::string(path)));
    std::string tmpDirName(SmtpfsDirName(std::string(path)));

    int ret = utimensat(0, path, tv, AT_SYMLINK_NOFOLLOW);
    if (ret == -1) {
        return -ENOENT;
    }
    const MtpFsTypeDir *parent = device_.DirFetchContent(tmpDirName);
    if (!parent) {
        return -ENOENT;
    }
    const MtpFsTypeFile *file = parent->File(tmpBaseName);
    if (!file) {
        return -ENOENT;
    }
    const_cast<MtpFsTypeFile *>(file)->SetModificationDate(tv->tv_sec);

    return 0;
}


int MtpFileSystem::Create(const char *path, mode_t mode, fuse_file_info *fileInfo)
{
    const std::string tmpPath = tmpFilesPool_.MakeTmpPath(std::string(path));

    int rval = ::creat(tmpPath.c_str(), mode);
    if (rval < 0) {
        return -errno;
    }

    fileInfo->fh = rval;
    tmpFilesPool_.AddFile(MtpFsTypeTmpFile(std::string(path), tmpPath, rval, true));
    rval = device_.FilePush(tmpPath, std::string(path));
    if (rval != 0) {
        return rval;
    }
    return 0;
}

int MtpFileSystem::Open(const char *path, struct fuse_file_info *fileInfo)
{
    if (fileInfo->flags & O_WRONLY) {
        fileInfo->flags |= O_TRUNC;
    }
    const std::string stdPath(path);

    MtpFsTypeTmpFile *tmpFile = const_cast<MtpFsTypeTmpFile *>(tmpFilesPool_.GetFile(stdPath));

    std::string tmpPath;
    if (tmpFile) {
        tmpPath = tmpFile->PathTmp();
    } else {
        tmpPath = tmpFilesPool_.MakeTmpPath(stdPath);

        // only copy the file if needed
        if (!HasPartialObjectSupport()) {
            int rval = device_.FilePull(stdPath, tmpPath);
            if (rval != 0) {
                return -rval;
            }
        } else {
            int fd = ::creat(tmpPath.c_str(), S_IRUSR | S_IWUSR);
            ::close(fd);
        }
    }

    // we create the tmp file even if we can use partial get/send to
    // have a valid file descriptor
    int fd = ::open(tmpPath.c_str(), fileInfo->flags);
    if (fd < 0) {
        ::unlink(tmpPath.c_str());
        return -errno;
    }

    fileInfo->fh = fd;

    if (tmpFile) {
        tmpFile->AddFileDescriptor(fd);
    } else {
        tmpFilesPool_.AddFile(MtpFsTypeTmpFile(stdPath, tmpPath, fd));
    }
    return 0;
}

int MtpFileSystem::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    int rval = 0;
    if (HasPartialObjectSupport()) {
        const std::string stdPath(path);
        rval = device_.FileRead(stdPath, buf, size, offset);
    } else {
        rval = ::pread(fileInfo->fh, buf, size, offset);
        if (rval < 0) {
            return -errno;
        }
    }
    return rval;
}

int MtpFileSystem::Write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fileInfo)
{
    int rval = 0;
    if (HasPartialObjectSupport()) {
        const std::string stdPath(path);
        rval = device_.FileWrite(stdPath, buf, size, offset);
    } else {
        const MtpFsTypeTmpFile *tmpFile = tmpFilesPool_.GetFile(std::string(path));
        if (!tmpFile) {
            return -EINVAL;
        }
        rval = ::pwrite(fileInfo->fh, buf, size, offset);
        if (rval < 0) {
            return -errno;
        }
        const_cast<MtpFsTypeTmpFile *>(tmpFile)->SetModified();
    }
    return rval;
}

int MtpFileSystem::Release(const char *path, struct fuse_file_info *fileInfo)
{
    int rval = ::close(fileInfo->fh);
    if (rval < 0) {
        return -errno;
    }
    const std::string stdPath(path);
    if (stdPath == std::string("-")) {
        return 0;
    }
    MtpFsTypeTmpFile *tmpFile = const_cast<MtpFsTypeTmpFile *>(tmpFilesPool_.GetFile(stdPath));
    tmpFile->RemoveFileDescriptor(fileInfo->fh);
    if (tmpFile->RefCnt() != 0) {
        return 0;
    }
    const bool modIf = tmpFile->IsModified();
    const std::string tmpPath = tmpFile->PathTmp();
    tmpFilesPool_.RemoveFile(stdPath);
    if (modIf) {
        rval = device_.FilePush(tmpPath, stdPath);
        if (rval != 0) {
            ::unlink(tmpPath.c_str());
            return -rval;
        }
    }

    ::unlink(tmpPath.c_str());

    return 0;
}

int MtpFileSystem::Statfs(const char *path, struct statvfs *statInfo)
{
    uint64_t bs = BS_SIZE;
    // XXX: linux coreutils still use bsize member to calculate free space
    statInfo->f_bsize = static_cast<unsigned long>(bs);
    statInfo->f_frsize = static_cast<unsigned long>(bs);
    statInfo->f_blocks = device_.StorageTotalSize() / bs;
    statInfo->f_bavail = device_.StorageFreeSize() / bs;
    statInfo->f_bfree = statInfo->f_bavail;
    return 0;
}

int MtpFileSystem::Flush(const char *path, struct fuse_file_info *fileInfo)
{
    return 0;
}

int MtpFileSystem::FSync(const char *path, int datasync, struct fuse_file_info *fi)
{
    int rval = -1;
#ifdef HAVE_FDATASYNC
    if (datasync) {
        rval = ::fdatasync(fi->fh);
    }
#else
    rval = ::fsync(fi->fh);
#endif
    if (rval != 0) {
        return -errno;
    }
    return 0;
}

int MtpFileSystem::OpenDir(const char *path, struct fuse_file_info *fileInfo)
{
    const MtpFsTypeDir *content = device_.DirFetchContent(std::string(path));
    if (!content) {
        return -ENOENT;
    }
    return 0;
}

int MtpFileSystem::ReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
    struct fuse_file_info *fileInfo, enum fuse_readdir_flags flag)
{
    enum fuse_fill_dir_flags fillFlags = FUSE_FILL_DIR_PLUS;
    const MtpFsTypeDir *content = device_.DirFetchContent(std::string(path));
    if (!content) {
        return -ENOENT;
    }
    const std::set<MtpFsTypeDir> dirs = content->Dirs();
    const std::set<MtpFsTypeFile> files = content->Files();

    for (const MtpFsTypeDir &d : dirs) {
        struct stat st;
        if (memset_s(&st, sizeof(st), 0, sizeof(st)) != EOK) {
            LOGE("memset st fail");
        }
        st.st_ino = d.Id();
        st.st_mode = S_IFDIR | PERMISSION_ONE;
        filler(buf, d.Name().c_str(), &st, 0, fillFlags);
    }

    for (const MtpFsTypeFile &f : files) {
        struct stat st;
        if (memset_s(&st, sizeof(st), 0, sizeof(st)) != EOK) {
            LOGE("memset st fail");
        }
        st.st_ino = f.Id();
        st.st_mode = S_IFREG | PERMISSION_TWO;
        filler(buf, f.Name().c_str(), &st, 0, fillFlags);
    }
    return 0;
}

int MtpFileSystem::ReleaseDir(const char *path, struct fuse_file_info *fileInfo)
{
    return 0;
}

int MtpFileSystem::FSyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    return 0;
}

bool MtpFileSystem::HasPartialObjectSupport()
{
    MtpFsDevice::Capabilities caps = device_.GetCapabilities();
    return (caps.CanGetPartialObject() && caps.CanSendPartialObject());
}
