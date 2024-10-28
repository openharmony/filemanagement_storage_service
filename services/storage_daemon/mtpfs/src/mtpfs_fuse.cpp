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

int WrapGetattr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    LOGI("mtp WrapGetattr");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->GetAttr(path, buf, fi);
    LOGI("GetAttr ret = %{public}d.", ret);
    return ret;
}

int WrapMkNod(const char *path, mode_t mode, dev_t dev)
{
    LOGI("mtp WrapMkNod");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->MkNod(path, mode, dev);
    LOGI("MkNod ret = %{public}d.", ret);
    return ret;
}

int WrapMkDir(const char *path, mode_t mode)
{
    LOGI("mtp WrapMkDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->MkDir(path, mode);
    LOGI("MkDir ret = %{public}d.", ret);
    return ret;
}

int WrapUnLink(const char *path)
{
    LOGI("mtp WrapUnLink");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->UnLink(path);
    LOGI("UnLink ret = %{public}d.", ret);
    return ret;
}

int WrapRmDir(const char *path)
{
    LOGI("mtp WrapRmDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->RmDir(path);
    LOGI("RmDir ret = %{public}d.", ret);
    return ret;
}

int WrapReName(const char *path, const char *newpath, unsigned int flags)
{
    LOGI("mtp WrapReName");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->ReName(path, newpath, flags);
    LOGI("ReName ret = %{public}d.", ret);
    return ret;
}

int WrapChMod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    LOGI("mtp WrapChMod");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->ChMods(path, mode, fi);
    LOGI("ChMods ret = %{public}d.", ret);
    return ret;
}

int WrapChown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    LOGE("mtp WrapChown path:%{public}s ,uid:%{public}lu, gid:%{public}lu", path, uid, gid);
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Chown(path, uid, gid, fi);
    LOGI("Chown ret = %{public}d.", ret);
    return ret;
}


int WrapUTimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi)
{
    LOGI("mtp WrapUTimens");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->UTimens(path, tv, fi);
    LOGI("UTimens ret = %{public}d.", ret);
    return ret;
}

int WrapOpen(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapOpen");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Open(path, fileInfo);
    LOGI("Open ret = %{public}d.", ret);
    return ret;
}

int WrapRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapRead");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Read(path, buf, size, offset, fileInfo);
    LOGI("Read ret = %{public}d.", ret);
    return ret;
}

int WrapWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapWrite");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Write(path, buf, size, offset, fileInfo);
    LOGI("Write ret = %{public}d.", ret);
    return ret;
}

int WrapStatfs(const char *path, struct statvfs *statInfo)
{
    LOGI("mtp WrapStatfs");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Statfs(path, statInfo);
    LOGI("Statfs ret = %{public}d.", ret);
    return ret;
}

int WrapFlush(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFlush");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Flush(path, fileInfo);
    LOGI("Flush ret = %{public}d.", ret);
    return ret;
}

int WrapRelease(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapRelease");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Release(path, fileInfo);
    LOGI("Release ret = %{public}d.", ret);
    return ret;
}

int WrapFSync(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFSync");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->FSync(path, datasync, fileInfo);
    LOGI("FSync ret = %{public}d.", ret);
    return ret;
}

int WrapOpenDir(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapOpenDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->OpenDir(path, fileInfo);
    LOGI("OpenDir ret = %{public}d.", ret);
    return ret;
}

int WrapReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo,
    enum fuse_readdir_flags flag)
{
    LOGI("mtp WrapReadDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->ReadDir(path, buf, filler, offset, fileInfo, flag);
    LOGI("ReadDir ret = %{public}d.", ret);
    return ret;
}

int WrapReleaseDir(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapReleaseDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->ReleaseDir(path, fileInfo);
    LOGI("ReleaseDir ret = %{public}d.", ret);
    return ret;
}

int WrapFSyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapFSyncDir");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->FSyncDir(path, datasync, fileInfo);
    LOGI("FSyncDir ret = %{public}d.", ret);
    return ret;
}

void *WrapInit(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    LOGI("mtp WrapInit");
    return DelayedSingleton<MtpFileSystem>::GetInstance()->Init(conn, cfg);
}

int WrapCreate(const char *path, mode_t mode, fuse_file_info *fileInfo)
{
    LOGI("mtp WrapCreate");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Create(path, mode, fileInfo);
    LOGI("Create ret = %{public}d.", ret);
    return ret;
}

int WrapTruncate(const char *path, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("mtp WrapTruncate");
    int ret = DelayedSingleton<MtpFileSystem>::GetInstance()->Truncate(path, offset, fileInfo);
    LOGI("Truncate ret = %{public}d.", ret);
    return ret;
}

int WrapReadLink(const char *path, char *out, size_t size)
{
    LOGI("mtp WrapReadLink");
    return 0;
}

int WrapSymLink(const char *path, const char * mode)
{
    LOGI("mtp WrapSymLink");
    return 0;
}

int WrapLink(const char *path, const char *out)
{
    LOGI("mtp WrapLink");
    return 0;
}

int WrapSetXAttr(const char *path, const char *in, const char *out, size_t size, int flag)
{
    LOGI("mtp WrapSetXAttr");
    return 0;
}

int WrapGetXAttr(const char *path, const char *in, char *out, size_t size)
{
    LOGI("mtp WrapGetXAttr");
    return 0;
}

int WrapListXAttr(const char *path, char *in, size_t size)
{
    LOGI("mtp WrapListXAttr");
    return 0;
}

int WrapRemoveXAttr(const char *path, const char *in)
{
    LOGI("mtp WrapRemoveXAttr");
    return 0;
}

void WrapDestroy(void *path)
{
    LOGI("mtp WrapDestroy");
    return;
}

int WrapAccess(const char *path, int size)
{
    LOGI("mtp WrapAccess");
    return 0;
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
    fuseOperations_.readlink = WrapReadLink;
    fuseOperations_.mknod = WrapMkNod;
    fuseOperations_.mkdir = WrapMkDir;
    fuseOperations_.unlink = WrapUnLink;
    fuseOperations_.rmdir = WrapRmDir;
    fuseOperations_.symlink = WrapSymLink;
    fuseOperations_.rename = WrapReName;
    fuseOperations_.link = WrapLink;
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
    fuseOperations_.setxattr = WrapSetXAttr;
    fuseOperations_.getxattr = WrapGetXAttr;
    fuseOperations_.listxattr = WrapListXAttr;
    fuseOperations_.removexattr = WrapRemoveXAttr;
    fuseOperations_.opendir = WrapOpenDir;
    fuseOperations_.readdir = WrapReadDir;
    fuseOperations_.releasedir = WrapReleaseDir;
    fuseOperations_.fsyncdir = WrapFSyncDir;
    fuseOperations_.init = WrapInit;
    fuseOperations_.destroy = WrapDestroy;
    fuseOperations_.access = WrapAccess;
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
        if (!device_.ConnectByDevFile(options_.deviceFile_)) {
            return false;
        }
    } else {
        // Connect to MTP device by order number, if no device file supplied
        if (!device_.ConnectByDevNo(options_.deviceNo_)) {
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

void *MtpFileSystem::Init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    return nullptr;
}

int MtpFileSystem::GetAttr(const char *path, struct stat *buf, struct fuse_file_info *fi)
{
    LOGI("MtpFileSystem: GetAttr enter, path: %{public}s", path);
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
            LOGE("MtpFileSystem: GetAttr error, content is null, path: %{public}s", path);
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
            LOGE("MtpFileSystem: GetAttr error, content dir is null, path: %{public}s", path);
            return -ENOENT;
        }
    }
    LOGI("MtpFileSystem: GetAttr success, path: %{public}s", path);
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

int MtpFileSystem::ChMods(const char *path, mode_t mode, struct fuse_file_info *fi)
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

int MtpFileSystem::Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    LOGI("mtp Chown path:%{public}s ,uid:%{public}lu, gid:%{public}lu", path, uid, gid);
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

int MtpFileSystem::UTimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi)
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
    LOGI("MtpFileSystem: Open enter, path: %{public}s", path);
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
        LOGE("MtpFileSystem: Open error, errno=%{public}d", errno);
        return -errno;
    }

    fileInfo->fh = fd;

    if (tmpFile) {
        tmpFile->AddFileDescriptor(fd);
    } else {
        tmpFilesPool_.AddFile(MtpFsTypeTmpFile(stdPath, tmpPath, fd));
    }
    LOGI("MtpFileSystem: Open success, path: %{public}s", path);
    return 0;
}

int MtpFileSystem::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    LOGI("MtpFileSystem: Read enter, path: %{public}s", path);
    int rval = 0;
    if (HasPartialObjectSupport()) {
        const std::string stdPath(path);
        rval = device_.FileRead(stdPath, buf, size, offset);
    } else {
        rval = ::pread(fileInfo->fh, buf, size, offset);
        if (rval < 0) {
            LOGE("MtpFileSystem: Read error, errno=%{public}d", errno);
            return -errno;
        }
    }
    LOGI("MtpFileSystem: Open success, path: %{public}s, rval=%{public}d", path, rval);
    return rval;
}

int MtpFileSystem::Write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fileInfo)
{
    LOGI("MtpFileSystem: Write enter, path: %{public}s", path);
    int rval = 0;
    if (HasPartialObjectSupport()) {
        const std::string stdPath(path);
        rval = device_.FileWrite(stdPath, buf, size, offset);
    } else {
        const MtpFsTypeTmpFile *tmpFile = tmpFilesPool_.GetFile(std::string(path));
        if (!tmpFile) {
            LOGE("MtpFileSystem: Write tmpFile error.");
            return -EINVAL;
        }
        rval = ::pwrite(fileInfo->fh, buf, size, offset);
        if (rval < 0) {
            LOGE("MtpFileSystem: Write pwrite error, errno=%{public}d", errno);
            return -errno;
        }
        const_cast<MtpFsTypeTmpFile *>(tmpFile)->SetModified();
    }
    LOGI("MtpFileSystem: Write success, path: %{public}s, rval=%{public}d", path, rval);
    return rval;
}

int MtpFileSystem::Release(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("MtpFileSystem: Release enter, path: %{public}s", path);
    int rval = ::close(fileInfo->fh);
    if (rval < 0) {
        LOGE("MtpFileSystem: Release close error, errno=%{public}d", errno);
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
    LOGI("MtpFileSystem: Release success, path: %{public}s", path);
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
    struct fuse_file_info *fileInfo, enum fuse_readdir_flags flag)
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
