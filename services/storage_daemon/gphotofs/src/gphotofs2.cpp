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

#include "gphotofs2.h"
#include <cstdint>
#include <limits>
#include <mutex>
#include <cstring>
#include <securec.h>
#include "storage_service_log.h"
#include "utils.h"
using namespace std;

using FuseFillDirFlags = fuse_fill_dir_flags;
using FuseReaddirFlags = fuse_readdir_flags;

static constexpr int32_t UPLOAD_RECORD_TRUE_LEN  = 4;
static constexpr int32_t UPLOAD_RECORD_FALSE_LEN = 5;
static constexpr const char *GPHOTO_THM_FLAG = "?MTP_THM";
static constexpr size_t MAX_MALLOC_SIZE = 100 * 1024 * 1024;
static constexpr size_t MAX_WRITE_SIZE = 10 * 1024 * 1024;
static constexpr size_t KB_TO_BYTES = 1024;
static constexpr size_t DOUBLE_DOT_LENGTH = 2;
static constexpr size_t PATH_DOT_LENGTH = 2;

static constexpr mode_t DEFAULT_FILE_MODE = 0644;
static constexpr mode_t DEFAULT_DIR_MODE = 0755;
static constexpr mode_t TEMP_FILE_MODE = 0600;
static constexpr unsigned long STATFS_BLOCK_SIZE = 1024;
static constexpr long STATFS_UNLIMITED = -1;
static constexpr nlink_t FILE_NLINK_COUNT = 1;
static constexpr nlink_t DIR_NLINK_COUNT = 2;
static bool g_readOnlyMode = true;
static std::mutex g_gphotoMutex;
static int g_err;
static std::mutex g_errMutex;

template<typename F>
auto WithCameraLocked(F&& f) -> decltype(f())
{
    std::lock_guard<std::mutex> guard(g_gphotoMutex);
    return f();
}

template<typename T>
static inline T* FhToPtr(uint64_t fh)
{
    static_assert(sizeof(uint64_t) == sizeof(uintptr_t), "fh size must match pointer size");
    return reinterpret_cast<T*>(static_cast<uintptr_t>(fh));
}

template<typename T>
static inline uint64_t PtrToFh(T* p)
{
    static_assert(sizeof(uint64_t) == sizeof(uintptr_t), "fh size must match pointer size");
    return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(p));
}

struct FillState {
    void* buf;
    fuse_fill_dir_t filler;
    off_t offset;
    FuseFillDirFlags fillFlags;
    off_t idx;
};

static time_t GetLocalUtcOffset()
{
    time_t now = time(nullptr);
    struct tm lt {};
    struct tm gt {};
    localtime_r(&now, &lt);
    gmtime_r(&now, &gt);
    return mktime(&lt) - mktime(&gt);
}

static inline time_t NormalizeMtime(time_t cameraMtime)
{
    return cameraMtime - GetLocalUtcOffset();
}

static int ListSubFolders(const char *path, Dir *dir, Context *ctx)
{
    CameraList *list = NULL;
    int ret = gp_list_new(&list);
    if (ret != GP_OK) {
        LOGE("ListSubFolders gp_list_new failed, ret=%{public}d", ret);
        return GpresultToErrno(ret);
    }

    ret = WithCameraLocked([&ctx, &path, &list] {
        return gp_camera_folder_list_folders(ctx->camera(),
            path, list, ctx->context());
    });
    if (ret != 0) {
        LOGE("gphoto listdir return err by ret = %{public}d", ret);
        if (list != NULL) {
            gp_list_free(list);
        }
        return GpresultToErrno(ret);
    }
    LOGI("gphoto listdir dir.list.count = %{public}d", gp_list_count(list));
    int32_t count = gp_list_count(list);
    for (int32_t i = 0; i < count; i++) {
        const char *name;
        gp_list_get_name(list, i, &name);
        if (!name) {
            LOGW("ListSubFolders: empty folder name, skipping");
            continue;
        }
        auto subDir = std::make_unique<Dir>(name);
        dir->addDir(subDir.release());
        LOGI("gphoto listdir dirs child dir");
    }
    gp_list_free(list);
    return GP_OK;
}

static int ListFiles(const char *path, Dir *dir, Context *ctx)
{
    CameraList *list = NULL;
    int ret = gp_list_new(&list);
    if (ret != GP_OK) {
        LOGE("ListFiles gp_list_new failed, ret=%{public}d", ret);
        return GpresultToErrno(ret);
    }

    ret = WithCameraLocked([&ctx, &path, &list] {
        return gp_camera_folder_list_files(ctx->camera(),
            path, list, ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("gphoto listdir files return err by ret = %{public}d", ret);
        if (list != NULL) {
            gp_list_free(list);
        }
        return GpresultToErrno(ret);
    }
    LOGI("gphoto listdir enter file.list.count = %{public}d", gp_list_count(list));
    for (int32_t fileIdx = 0; fileIdx < gp_list_count(list); fileIdx++) {
        const char *name = nullptr;
        CameraFileInfo info;

        gp_list_get_name(list, fileIdx, &name);
        if (!name) {
            LOGW("ListFiles: empty file name, skipping");
            continue;
        }
        int getInfoResult = WithCameraLocked([&ctx, &path, &name, &info] {
            return gp_camera_file_get_info(ctx->camera(), path, name, &info,
                ctx->context());
        });
        if (getInfoResult != GP_OK) {
            LOGW("ListFiles: failed to get file info for '%{public}s', skipping, gp_ret=%{public}d",
                 name ? name : "unknown", getInfoResult);
            continue;  // Skip this file and continue with others
        }

        auto file = std::make_unique<File>(name, info);
        dir->addFile(file.release());
        LOGI("gphoto listdir files child file");
    }

    gp_list_free(list);
    return GP_OK;
}

static int ListDir(const char *path, Dir *dir, Context *ctx)
{
    LOGI("gphoto listdir enter");
    if (path == nullptr) {
        LOGE("gphoto ListDir invalid args, path=nullptr");
        return -EINVAL;
    }
    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto ListDir invalid path");
        return -EINVAL;
    }

    int ret = ListSubFolders(path, dir, ctx);
    if (ret != GP_OK) {
        return ret;
    }

    ret = ListFiles(path, dir, ctx);
    if (ret != GP_OK) {
        return ret;
    }

    dir->listed = true;
    LOGI("gphoto listdir return 0");
    return 0;
}

static Dir* FindSubDir(Dir* currentDir, const std::string& name, const std::string& dirPath,
                       Context* ctx)
{
    if (!currentDir->listed) {
        ListDir(dirPath.c_str(), currentDir, ctx);
    }
    return currentDir->getDir(name);
}

static std::string UpdateDirPath(const std::string& currentPath, const std::string& name)
{
    if (currentPath == "/") {
        return "/" + name;
    }
    return currentPath + "/" + name;
}

Dir* FindDir(const string& path, Context *ctx)
{
    Dir* dir = &ctx->root();
    string dirPath = "/";
    LOGI("gphoto finddir enter");
    if (!IsFilePathValid(path)) {
        LOGE("gphoto FindDir invalid path");
        return nullptr;
    }
    int last;
    if (path[0] == '/') {
        last = 0;
    } else {
        last = -1;
    }

    bool continueLoop = true;
    while (continueLoop) {
        int next = path.find('/', last + 1);
        if (next == string::npos) {
            string name = path.substr(last + 1);
            if (name == "") {
                return dir;
            }
            dir = FindSubDir(dir, name, dirPath, ctx);
            return dir;
        }

        string name = path.substr(last + 1, next - last - 1);
        if (name == "") {
            last = next;
            continue;
        }
        dir = FindSubDir(dir, name, dirPath, ctx);
        if (dir == nullptr) {
            LOGE("gphoto finddir return null by dir = null");
            return nullptr;
        }
        dirPath = UpdateDirPath(dirPath, name);
        last = next;
    }
    LOGE("gphoto finddir unexpected loop exit");
    return nullptr;
}

File* FindFile(const string& path, Context *ctx)
{
    LOGI("gphoto findfile enter");
    if (!IsFilePathValid(path)) {
        LOGE("FindFile invalid path");
        return nullptr;
    }
    size_t pos = path.rfind("/");
    Dir *dir;
    string name;
    if (pos == string::npos) {
        dir = &ctx->root();
        name = path;
        if (!dir->listed) {
            int ret = ListDir("/", dir, ctx);
            if (ret != 0) {
                LOGE("FindFile ListDir failed, ret=%{public}d", ret);
                return nullptr;
            }
        }
    } else {
        string parentPath = path.substr(0, pos + 1);
        dir = FindDir(parentPath, ctx);
        if (dir == nullptr) {
            LOGE("gphoto findfile return null by dir = null");
            return nullptr;
        }
        name = path.substr(pos + 1);
        if (!dir->listed) {
            int ret = ListDir(parentPath.c_str(), dir, ctx);
            if (ret != 0) {
                LOGE("FindFile ListDir failed, ret=%{public}d", ret);
                return nullptr;
            }
        }
    }
    return dir->getFile(name);
}

static int CreateTempFile(File *file, const std::string &filePath)
{
    if (file->tmpFileCreated) {
        return 0;
    }

    bool continueLoop = true;
    size_t start = 0;
    while (continueLoop) {
        size_t end = filePath.find('/', start);
        std::string component;
        if (end == std::string::npos) {
            component = filePath.substr(start);
            continueLoop = false;
        } else {
            component = filePath.substr(start, end - start);
            start = end + 1;
        }
        if (component == "..") {
            LOGE("CreateTempFile: invalid path contains '..' component");
            return -EINVAL;
        }
    }

    std::string safePath = filePath;
    std::replace(safePath.begin(), safePath.end(), '/', '_');
    std::replace(safePath.begin(), safePath.end(), '\\', '_');

    std::string tmpDirResult = SmtpfsGetTmpDir();
    if (tmpDirResult == "") {
        LOGE("CreateTempFile: mkdtemp failed, errno=%{public}d", errno);
        return -EIO;
    }

    file->tmpPath = tmpDirResult + "/" + safePath + TMP_FILE_SUFFIX;

    file->tmpFd = open(file->tmpPath.c_str(), O_RDWR | O_CREAT | O_EXCL | O_TRUNC, TEMP_FILE_MODE);
    if (file->tmpFd < 0) {
        LOGE("CreateTempFile: open failed, errno=%{public}d", errno);
        if (!tmpDirResult.empty()) {
            if (rmdir(tmpDirResult.c_str()) != 0) {
                LOGE("CreateTempFile: failed to remove temp dir, errno=%{public}d", errno);
            }
        }
        return -EIO;
    }

    file->tmpFileCreated = true;
    LOGI("CreateTempFile: created tmp fd=%{public}d", file->tmpFd);
    return 0;
}

static void CleanupTempFile(File *file)
{
    if (file->tmpFd >= 0) {
        close(file->tmpFd);
        file->tmpFd = -1;
    }

    if (!file->tmpPath.empty()) {
        unlink(file->tmpPath.c_str());
        file->tmpPath.clear();
    }

    file->tmpFileCreated = false;
}

static int SaveCameraFileToTemp(File *file, const char *path)
{
    if (!file->tmpFileCreated) {
        std::string tempPath = "/" + file->name;
        int ret = CreateTempFile(file, tempPath.c_str());
        if (ret < 0) {
            return ret;
        }
    }
    if (lseek(file->tmpFd, 0, SEEK_SET) < 0) {
        CleanupTempFile(file);
        return -errno;
    }
    if (ftruncate(file->tmpFd, 0) != 0) {
        CleanupTempFile(file);
        return -errno;
    }
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    std::string dirName(SmtpfsDirName(path));
    std::string fileName(SmtpfsBaseName(path));
    CameraFile *cf = nullptr;

    int dupFd = dup(file->tmpFd);
    if (dupFd < 0) {
        CleanupTempFile(file);
        return -errno;
    }
    int gpResult = gp_file_new_from_fd(&cf, dupFd);
    if (gpResult != GP_OK) {
        close(dupFd);
        return GpresultToErrno(gpResult);
    }
    gpResult = WithCameraLocked([&ctx, &dirName, &fileName, &cf] {
        return gp_camera_file_get(ctx->camera(), dirName.c_str(), fileName.c_str(),
                                  GP_FILE_TYPE_NORMAL, cf, ctx->context());
    });
    gp_file_unref(cf);
    if (gpResult != GP_OK) {
        return GpresultToErrno(gpResult);
    }
    off_t end = lseek(file->tmpFd, 0, SEEK_END);
    if (end < 0) {
        CleanupTempFile(file);
        return -errno;
    }

    file->size = end;
    file->tmpFileCreated = true;
    return 0;
}

static inline bool IsThumbPath(const char* path)
{
    if (path == nullptr) {
        return false;
    }
    const size_t pathLen = strlen(path);
    const size_t thumbFlagLen = strlen(GPHOTO_THM_FLAG);
    if (pathLen < thumbFlagLen) {
        return false;
    }
    const char* suffixPos = path + (pathLen - thumbFlagLen);
    return strcmp(suffixPos, GPHOTO_THM_FLAG) == 0;
}

static inline std::string StripThumbFlag(const std::string& thumbPath)
{
    const size_t thumbFlagLen = strlen(GPHOTO_THM_FLAG);
    if (thumbPath.size() < thumbFlagLen) {
        return thumbPath;
    }
    // Check if the path actually ends with GPHOTO_THM_FLAG
    if (thumbPath.compare(thumbPath.size() - thumbFlagLen, thumbFlagLen, GPHOTO_THM_FLAG) != 0) {
        return thumbPath;
    }
    const size_t realPathLen = thumbPath.size() - thumbFlagLen;
    return thumbPath.substr(0, realPathLen);
}

static int FillThumbStat(struct stat* st, const File* file, unsigned long dataSize, Context* ctx)
{
    memset_s(st, sizeof(*st), 0, sizeof(*st));
    st->st_mode = S_IFREG | DEFAULT_FILE_MODE;
    st->st_mtime = NormalizeMtime(file->mtime);
    st->st_nlink = FILE_NLINK_COUNT;
    if (dataSize > static_cast<unsigned long>(std::numeric_limits<off_t>::max())) {
        LOGE("FillThumbStat: dataSize too large for off_t, size=%{public}lu", dataSize);
        return -EFBIG;
    }
    st->st_size = static_cast<off_t>(dataSize);
    st->st_blocks = SizeToBlocks(dataSize);
    st->st_uid = ctx->uid();
    st->st_gid = ctx->gid();
    return 0;
}

static int GetThumbAttr(const char* path, struct stat* st, Context* ctx)
{
    std::string real = StripThumbFlag(path);
    if (!IsFilePathValid(real)) {
        LOGE("GetThumbAttr invalid path");
        return -EINVAL;
    }
    File* file = FindFile(real, ctx);
    if (!file) {
        LOGE("GetThumbAttr file not found");
        return -ENOENT;
    }

    std::string dirName(SmtpfsDirName(real.c_str()));
    std::string fileName(SmtpfsBaseName(real.c_str()));
    CameraFile* thm = nullptr;
    int gpResult = gp_file_new(&thm);
    if (gpResult != GP_OK) {
        LOGE("GetThumbAttr gp_file_new failed, ret=%{public}d", gpResult);
        return GpresultToErrno(gpResult);
    }

    int ret = WithCameraLocked([&ctx, &dirName, &fileName, &thm] {
         return gp_camera_file_get(ctx->camera(), dirName.c_str(), fileName.c_str(),
                                   GP_FILE_TYPE_PREVIEW, thm, ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("GetThumbAttr gp_camera_file_get fail, gp_ret=%{public}d", ret);
        gp_file_unref(thm);
        return GpresultToErrno(ret);
    }

    const char* data = nullptr;
    unsigned long dataSize = 0;
    ret = WithCameraLocked([&thm, &data, &dataSize] {
        return gp_file_get_data_and_size(thm, &data, &dataSize);
    });
    if (ret != GP_OK) {
        LOGE("GetThumbAttr gp_file_get_data_and_size fail, gp_ret=%{public}d", ret);
        gp_file_unref(thm);
        return GpresultToErrno(ret);
    }

    ret = FillThumbStat(st, file, dataSize, ctx);
    gp_file_unref(thm);
    return ret;
}

static int Getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    if (path == nullptr) {
        LOGE("Getattr path is null");
        return -EINVAL;
    }
    LOGI("gphoto getattr enter");
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (ctx == nullptr) {
        LOGE("Getattr ctx nullptr");
        return -EIO;
    }
    if (IsThumbPath(path)) {
        std::string real = StripThumbFlag(path);
        if (!IsFilePathValid(real)) {
            LOGE("Getattr invalid thumb path");
            return -EINVAL;
        }
        return GetThumbAttr(real.c_str(), st, ctx);
    }
    if (!IsFilePathValid(path)) {
        LOGE("Getattr: Invalid path");
        return -EINVAL;
    }

    Dir *dir = FindDir(path, ctx);
    if (dir != nullptr) {
        st->st_mode = S_IFDIR | DEFAULT_DIR_MODE;
        st->st_nlink = DIR_NLINK_COUNT;
        st->st_uid = ctx->uid();
        st->st_gid = ctx->gid();
        LOGI("gphoto getattr return 0 by dir != null");
        return 0;
    }

    File *file = FindFile(path, ctx);
    if (file != nullptr) {
        st->st_mode = S_IFREG | DEFAULT_FILE_MODE;
        st->st_nlink = FILE_NLINK_COUNT;
        st->st_size = file->size;
        st->st_blocks = SizeToBlocks(file->size);
        st->st_mtime = NormalizeMtime(file->mtime);
        st->st_uid = ctx->uid();
        st->st_gid = ctx->gid();
        LOGI("gphoto getattr return 0 by file != null");
        return 0;
    }
    LOGE("Getattr: path not found");
    return -ENOENT;
}

static int AllocateAndCopyThumbData(ThumbDesc *td, const char *data,
                                    unsigned long dataSize, const std::string &realPath)
{
    td->realPath = realPath;
    td->size = static_cast<size_t>(dataSize);

    if (td->size > MAX_MALLOC_SIZE) {
        LOGE("AllocateAndCopyThumbData size too large, size=%{public}zu", td->size);
        return -ENOMEM;
    }

    if (td->size == 0) {
        td->buf = nullptr;
        return 0;
    }

    td->buf = static_cast<char*>(malloc(td->size));
    if (!td->buf) {
        LOGE("AllocateAndCopyThumbData malloc fail, size=%{public}zu", td->size);
        return -ENOMEM;
    }

    if (data == nullptr) {
        free(td->buf);
        td->buf = nullptr;
        return 0;
    }

    int ret = memcpy_s(td->buf, td->size, data, td->size);
    if (ret != EOK) {
        LOGE("AllocateAndCopyThumbData memcpy_s failed, ret=%{public}d", ret);
        free(td->buf);
        td->buf = nullptr;
        return -EIO;
    }
    return 0;
}

static int LoadThumbPreviewData(Context *ctx, const std::string &dirName,
                                const std::string &fileName, ThumbDesc *td,
                                const std::string &realPath)
{
    if (ctx == nullptr || td == nullptr) {
        LOGE("LoadThumbPreviewData invalid args");
        return -EINVAL;
    }

    CameraFile* thm = nullptr;
    int gpResult = gp_file_new(&thm);
    if (gpResult != GP_OK) {
        LOGE("LoadThumbPreviewData gp_file_new failed, ret=%{public}d", gpResult);
        return GpresultToErrno(gpResult);
    }

    gpResult = WithCameraLocked([&ctx, &dirName, &fileName, &thm] {
        return gp_camera_file_get(ctx->camera(), dirName.c_str(),
                                  fileName.c_str(), GP_FILE_TYPE_PREVIEW,
                                  thm, ctx->context());
    });
    if (gpResult != GP_OK) {
        LOGE("LoadThumbPreviewData gp_camera_file_get fail, gp_ret=%{public}d", gpResult);
        gp_file_unref(thm);
        return GpresultToErrno(gpResult);
    }

    const char* data = nullptr;
    unsigned long dataSize = 0;
    gpResult = WithCameraLocked([&thm, &data, &dataSize] {
        return gp_file_get_data_and_size(thm, &data, &dataSize);
    });
    if (gpResult != GP_OK) {
        LOGE("LoadThumbPreviewData gp_file_get_data_and_size fail, gp_ret=%{public}d", gpResult);
        gp_file_unref(thm);
        return GpresultToErrno(gpResult);
    }

    int ret = AllocateAndCopyThumbData(td, data, dataSize, realPath);

    gp_file_unref(thm);
    return ret;
}

static int OpenThumb(const char* path, struct fuse_file_info* fi)
{
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (ctx == nullptr) {
        LOGE("OpenThumb ctx nullptr");
        return -EIO;
    }

    std::string real = StripThumbFlag(path);
    if (!IsFilePathValid(real)) {
        LOGE("OpenThumb invalid path");
        return -EINVAL;
    }

    std::string dirName(SmtpfsDirName(real.c_str()));
    std::string fileName(SmtpfsBaseName(real.c_str()));

    auto* td = new (std::nothrow) ThumbDesc();
    if (td == nullptr) {
        LOGE("OpenThumb malloc ThumbDesc fail");
        return -ENOMEM;
    }

    int ret = LoadThumbPreviewData(ctx, dirName, fileName, td, real);
    if (ret != 0) {
        delete td;
        return ret;
    }

    fi->fh = PtrToFh(td);
    return 0;
}

static int ReadThumb(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    if (!fi) {
        LOGE("ReadThumb fi is null");
        return -EIO;
    }
    auto* td = FhToPtr<ThumbDesc>(fi->fh);
    if (!td) {
        LOGE("ReadThumb td nullptr fh=%{public}llu",
             static_cast<unsigned long long>(fi ? fi->fh : 0));
        return -EIO;
    }

    lock_guard<std::mutex> g(td->lock);
    if (static_cast<size_t>(offset) >= td->size) {
        return 0;
    }
    size_t bytesToRead = size;
    if (static_cast<size_t>(offset) + bytesToRead > td->size) {
        bytesToRead = td->size - static_cast<size_t>(offset);
    }
    if (bytesToRead > static_cast<size_t>(std::numeric_limits<int>::max())) {
        LOGE("ReadThumb size exceeds INT_MAX");
        return -EIO;
    }
    if (bytesToRead > 0 && buf != nullptr && td->buf != nullptr) {
        int ret = memcpy_s(buf, bytesToRead, td->buf + offset, bytesToRead);
        if (ret != EOK) {
            LOGE("ReadThumb memcpy_s failed, ret=%{public}d", ret);
            return -EIO;
        }
    }
    return static_cast<int>(bytesToRead);
}

static int ReleaseThumb(const char* path, struct fuse_file_info* fi)
{
    auto* td = FhToPtr<ThumbDesc>(fi->fh);
    if (td) {
        lock_guard<mutex> g(td->lock);
        delete td;
        fi->fh = 0;
    }
    return 0;
}

static int InitializeNewFile(File *file, const char *path, Context *ctx)
{
    if (file->camFile == nullptr) {
        std::string dirName(SmtpfsDirName(path));
        std::string fileName(SmtpfsBaseName(path));
        CameraFile *camFile;
        int gpResult = gp_file_new(&camFile);
        if (gpResult != GP_OK) {
            LOGE("InitializeNewFile gp_file_new failed, ret=%{public}d", gpResult);
            return GpresultToErrno(gpResult);
        }
        file->camFile = camFile;
    }

    int ret = CreateTempFile(file, path);
    if (ret < 0) {
        LOGE("gphoto create CreateTempFile failed ret=%{public}d", ret);
        if (file->camFile != nullptr) {
            gp_file_unref(file->camFile);
            file->camFile = nullptr;
        }
        return ret;
    }
    return 0;
}

static int Create(const char *path, mode_t mode,
                  struct fuse_file_info *fileInfo)
{
    LOGI("gphoto create enter");
    if (g_readOnlyMode) {
        LOGE("Create: operation denied in read-only mode");
        return -EROFS;
    }

    if (!IsFilePathValid(path)) {
        LOGE("Create: Invalid path");
        return -EINVAL;
    }

    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    std::string dirName(SmtpfsDirName(path));
    std::string fileName(SmtpfsBaseName(path));

    Dir *dir = FindDir(dirName, ctx);
    if (dir == nullptr || dir->getFile(fileName) != nullptr) {
        LOGE("Create: parent directory failed");
        return dir == nullptr ? -ENOENT : -EEXIST;
    }

    File *file = new (std::nothrow) File(fileName);
    if (file == nullptr) {
        LOGE("gphoto create failed to allocate File");
        return -ENOMEM;
    }
    file->size = 0;
    file->changed = true;
    dir->addFile(file);

    int ret = InitializeNewFile(file, path, ctx);
    if (ret < 0) {
        dir->removeFile(file);
        delete file;
        LOGE("gphoto create InitializeNewFile failed ret=%{public}d", ret);
        return ret;
    }

    FileDesc *fd = new (std::nothrow) FileDesc();
    if (fd == nullptr) {
        LOGE("gphoto create failed to allocate FileDesc");
        dir->removeFile(file);
        delete file;
        return -ENOMEM;
    }
    fd->writable = true;
    fd->file = file;
    file->ref++;
    fileInfo->fh = PtrToFh(fd);
    LOGI("gphoto create success");
    return 0;
}

static Dir* FindDirNoFetch(const std::string& path, Context* ctx)
{
    if (!IsFilePathValid(path)) {
        LOGE("FindDirNoFetch invalid path");
        return nullptr;
    }
    Dir* dir = &ctx->root();
    if (path.empty() || path == "/") {
        return dir;
    }

    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() == '/') {
        normalizedPath = normalizedPath.substr(0, normalizedPath.size() - 1);
    }

    int last = (normalizedPath[0] == '/') ? 0 : -1;
    bool continueLoop = true;
    while (continueLoop) {
        int next = static_cast<int>(normalizedPath.find('/', last + 1));
        if (next == static_cast<int>(std::string::npos)) {
            std::string name = normalizedPath.substr(last + 1);
            if (name.empty()) {
                return dir;
            }
            return dir ? dir->getDir(name) : nullptr;
        }
        std::string name = normalizedPath.substr(last + 1, next - last - 1);
        if (name.empty()) {
            last = next;
            continue;
        }

        dir = dir ? dir->getDir(name) : nullptr;
        if (!dir) {
            return nullptr;
        }
        last = next;
    }
    LOGE("FindDirNoFetch unexpected loop exit");
    return nullptr;
}

static int IsDirFetchedGphoto(const std::string& path, Context* ctx, char* out, size_t size)
{
    auto writeBool = [&out, &size](bool boolValue)->int {
        const char* s = boolValue ? "true" : "false";
        size_t len = strlen(s);

        if (out == nullptr || size == 0) {
            return static_cast<int>(len);
        }

        if (size < len) {
            return -ERANGE;
        }
        int ret = memcpy_s(out, size, s, len);
        if (ret != EOK) {
            LOGE("IsDirFetchedGphoto memcpy_s failed, ret=%{public}d", ret);
            return -EIO;
        }
        return static_cast<int>(len);
    };

    if (path == "/") {
        return writeBool(true);
    }

    Dir* dir = FindDirNoFetch(path, ctx);
    if (!dir) {
        return writeBool(false);
    }
    return writeBool(dir->listed);
}

static int Open(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("gphoto open enter");
    if (IsThumbPath(path)) {
        std::string real = StripThumbFlag(path);
        if (!IsFilePathValid(real)) {
            LOGE("gphoto Open invalid thumb path");
            return -EINVAL;
        }
        int retThumb = OpenThumb(path, fileInfo);
        if (retThumb != 0) {
            LOGE("gphoto Open OpenThumb fail ret=%{public}d", retThumb);
        }
        return retThumb;
    }

    int mode = fileInfo->flags & O_ACCMODE;
    if (g_readOnlyMode) {
        if (mode != O_RDONLY) {
            LOGE("Open: write access denied in read-only mode");
            return -EROFS;
        }
    }

    if (!IsFilePathValid(path)) {
        LOGE("gphoto Open: Invalid path");
        return -EINVAL;
    }

    fileInfo->direct_io = true;
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    File *file = FindFile(path, ctx);
    if (file == nullptr) {
        LOGE("Open: file not found");
        return -ENOENT;
    }
    lock_guard<mutex> fileGuard(file->lock);

    FileDesc *fd = new (std::nothrow) FileDesc();
    if (fd == nullptr) {
        LOGE("gphoto open failed to allocate FileDesc");
        return -ENOMEM;
    }
    if (mode == O_RDONLY) {
        fd->writable = false;
    } else {
        fd->writable = true;
    }
    fd->file = file;
    file->ref++;
    fileInfo->fh = PtrToFh(fd);
    LOGI("gphoto open return 0");
    return 0;
}

static int UploadChangedFile(File *file, const char *path, Context *ctx)
{
    if (g_readOnlyMode) {
        LOGE("UploadChangedFile: operation denied in read-only mode");
        return -EROFS;
    }

    if (!file->changed || !file->tmpFileCreated || file->tmpFd < 0) {
        return 0;
    }

    LOGI("gphoto release file changed, uploading to camera");
    if (fsync(file->tmpFd) < 0) {
        LOGE("UploadChangedFile: fsync failed, errno=%{public}d", errno);
        return -errno;
    }
    lseek(file->tmpFd, 0, SEEK_SET);
    std::string dirName(SmtpfsDirName(path));
    std::string fileName(SmtpfsBaseName(path));

    CameraFile *upload = nullptr;
    int dupFd = dup(file->tmpFd);
    if (dupFd < 0) {
        CleanupTempFile(file);
        return -errno;
    }
    int gpResult = gp_file_new_from_fd(&upload, dupFd);
    if (gpResult != GP_OK) {
        close(dupFd);
        return GpresultToErrno(gpResult);
    }
    gpResult = WithCameraLocked([&ctx, &dirName, &fileName, &upload] {
        int deleteResult = gp_camera_file_delete(ctx->camera(), dirName.c_str(), fileName.c_str(), ctx->context());
        if (deleteResult != GP_OK && deleteResult != GP_ERROR_FILE_NOT_FOUND) {
            LOGE("UploadChangedFile: delete failed with %{public}d, continuing upload", deleteResult);
        }
        return gp_camera_folder_put_file(ctx->camera(), dirName.c_str(), fileName.c_str(),
            GP_FILE_TYPE_NORMAL, upload, ctx->context());
    });
    gp_file_unref(upload);
    if (gpResult != GP_OK) {
        return GpresultToErrno(gpResult);
    }
    file->changed = false;
    return 0;
}

static int Release(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("gphoto release enter");
    if (!IsFilePathValid(path)) {
        LOGE("gphoto Release: Invalid path");
        return -EINVAL;
    }
    if (IsThumbPath(path)) {
        int ret = ReleaseThumb(path, fileInfo);
        if (ret != 0) {
            LOGE("Release ReleaseThumb fail ret=%{public}d", ret);
        }
        return ret;
    }

    struct fuse_context* ctxStruct = fuse_get_context();
    if (ctxStruct == nullptr) {
        LOGE("Release: fuse_get_context() returned null");
        return -EIO;
    }
    if (ctxStruct->private_data == nullptr) {
        LOGE("Release: fuse_context private_data is null");
        return -EIO;
    }
    Context *ctx = static_cast<Context*>(ctxStruct->private_data);
    FileDesc *fd = FhToPtr<FileDesc>(fileInfo->fh);
    if (fd == nullptr) {
        LOGE("Release: file descriptor is null");
        return -EIO;
    }
    File *file = fd->file;
    if (file == nullptr) {
        delete fd;
        LOGI("Release: file is null");
        return -EIO;
    }
    lock_guard<mutex> fileGuard(file->lock);
    delete fd;
    file->ref--;

    int ret = UploadChangedFile(file, path, ctx);
    if (ret != 0) {
        return ret;
    }

    CleanupTempFile(file);
    LOGI("gphoto release return 0");
    return 0;
}

static File* ValidateAndGetFile(struct fuse_file_info *fileInfo, const char *path)
{
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (!ctx || !fileInfo || !fileInfo->fh) {
        LOGE("gphoto Read bad fd fileInfo");
        return nullptr;
    }
    FileDesc *fd = FhToPtr<FileDesc>(fileInfo->fh);
    if (!fd || !fd->file) {
        LOGE("gphoto Read bad FileDesc");
        return nullptr;
    }
    return fd->file;
}

static ssize_t ReadFromFileFd(File *file, char *buf, size_t size, off_t offset)
{
    ssize_t bytesRead = pread(file->tmpFd, buf, size, offset);
    if (bytesRead < 0) {
        LOGE("gphoto read failed, errno=%{public}d", errno);
        return -errno;
    }

    return bytesRead;
}

static int Read(const char *path, char *buf, size_t size, off_t offset,
                struct fuse_file_info *fileInfo)
{
    LOGI("gphoto read enter");
    if (!IsFilePathValid(path)) {
        LOGE("Read Invalid path");
        return -EINVAL;
    }
    if (IsThumbPath(path)) {
        std::string real = StripThumbFlag(path);
        if (!IsFilePathValid(real)) {
            LOGE("gphoto Read invalid thumb path");
            return -EINVAL;
        }
        int ret = ReadThumb(path, buf, size, offset, fileInfo);
        if (ret < 0) {
            LOGE("gphoto Read ReadThumb fail ret=%{public}d", ret);
        }
        return ret;
    }

    File *file = ValidateAndGetFile(fileInfo, path);
    if (!file) {
        return -EIO;
    }

    lock_guard<mutex> guard(file->lock);

    if (!file->tmpFileCreated) {
        int ret = SaveCameraFileToTemp(file, path);
        LOGI("gphoto read enter SaveCameraFileToTemp ret = %{public}d", ret);
        if (ret != 0) {
            return ret;
        }
    }

    ssize_t bytesRead = ReadFromFileFd(file, buf, size, offset);
    if (bytesRead < 0) {
        return static_cast<int>(bytesRead);
    }

    LOGI("gphoto read return size = %{public}zd", bytesRead);
    return static_cast<int>(bytesRead);
}

static File* ValidateFileDesc(struct fuse_file_info *fileInfo, const char *path)
{
    if (!fileInfo || !fileInfo->fh) {
        LOGE("gphoto Write bad fd");
        return nullptr;
    }
    FileDesc *fd = FhToPtr<FileDesc>(fileInfo->fh);
    if (!fd || !fd->file) {
        return nullptr;
    }
    return fd->file;
}

static int ExtendFileIfNeeded(File *file, size_t needSize)
{
    if (file == nullptr) {
        LOGE("ExtendFileIfNeeded: file is null");
        return -EINVAL;
    }
    if (file->tmpFd < 0) {
        LOGE("ExtendFileIfNeeded: invalid file descriptor");
        return -EIO;
    }

    if (needSize > static_cast<size_t>(std::numeric_limits<off_t>::max())) {
        return -EFBIG;
    }

    if (static_cast<size_t>(file->size) >= needSize) {
        return 0;
    }

    if (ftruncate(file->tmpFd, static_cast<off_t>(needSize)) < 0) {
        return -errno;
    }

    file->size = static_cast<off_t>(needSize);
    return 0;
}

static int PrepareFileWrite(File *file, const char *path, size_t size, off_t offset)
{
    if (!file->tmpFileCreated) {
        LOGI("gphoto write creating new temp file");
        int ret = CreateTempFile(file, path);
        if (ret < 0) {
            LOGE("gphoto write CreateTempFile failed ret=%{public}d", ret);
            return ret;
        }
    }

    return ExtendFileIfNeeded(file, static_cast<size_t>(offset) + size);
}

static ssize_t WriteToFileFd(File *file, const char *buf, size_t size, off_t offset)
{
    if (buf == nullptr) {
        LOGE("gphoto Write buf is null");
        return -EINVAL;
    }

    ssize_t bytesWritten = pwrite(file->tmpFd, buf, size, offset);
    if (bytesWritten < 0) {
        LOGE("gphoto write failed, errno=%{public}d", errno);
        return -errno;
    }

    if (static_cast<size_t>(bytesWritten) != size) {
        LOGE("gphoto write partial write, written=%{public}zd, size=%{public}zu",
             bytesWritten, size);
        return -EIO;
    }
    return bytesWritten;
}

static int Write(const char *path, const char *buf, size_t size, off_t offset,
                 struct fuse_file_info *fileInfo)
{
    LOGI("gphoto write enter");
    if (g_readOnlyMode) {
        LOGE("Write: operation denied in read-only mode");
        return -EROFS;
    }

    if (!IsFilePathValid(path)) {
        LOGE("gphoto Write: Invalid path");
        return -EINVAL;
    }
    if (size > MAX_WRITE_SIZE) {
        LOGE("gphoto Write size too large, size=%{public}zu", size);
        return -EINVAL;
    }

    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (ctx == nullptr) {
        LOGE("gphoto Write ctx nullptr");
        return -EIO;
    }

    if (offset < 0 || size > SSIZE_MAX - offset) {
        LOGE("gphoto Write offset or size overflow, offset=%{public}lld size=%{public}zu",
            static_cast<long long>(offset), size);
        return -EINVAL;
    }

    File *file = ValidateFileDesc(fileInfo, path);
    if (!file) {
        return -EBADF;
    }

    lock_guard<mutex> guard(file->lock);

    int ret = PrepareFileWrite(file, path, size, offset);
    if (ret < 0) {
        return ret;
    }

    ssize_t bytesWritten = WriteToFileFd(file, buf, size, offset);
    if (bytesWritten < 0) {
        return static_cast<int>(bytesWritten);
    }

    file->changed = true;
    LOGI("gphoto write return = %{public}zd", bytesWritten);
    return static_cast<int>(bytesWritten);
}

static int Flush(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("gphoto flush enter");
    return 0;
}

static int Truncate(const char *path, off_t size, struct fuse_file_info *fi)
{
    LOGI("gphoto truncate enter");
    if (g_readOnlyMode) {
        LOGE("Truncate: operation denied in read-only mode");
        return -EROFS;
    }

    if (!IsFilePathValid(path)) {
        LOGE("gphoto Truncate: Invalid path");
        return -EINVAL;
    }
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (!ctx) {
        LOGE("gphoto Truncate ctx nullptr");
        return -EIO;
    }
    File *file = FindFile(path, ctx);
    if (file == nullptr) {
        LOGE("gphoto truncate file not found");
        return -ENOENT;
    }
    lock_guard<mutex> guard(file->lock);

    if (!file->tmpFileCreated) {
        int ret = SaveCameraFileToTemp(file, path);
        if (ret < 0) {
            LOGE("gphoto truncate SaveCameraFileToTemp failed ret=%{public}d", ret);
            return ret;
        }
    }

    if (ftruncate(file->tmpFd, size) < 0) {
        LOGE("gphoto truncate ftruncate failed, errno=%{public}d", errno);
        return -errno;
    }

    file->size = size;
    file->changed = true;

    LOGI("gphoto truncate return 0");
    return 0;
}

static int FillOneEntry(FillState* state, const char* name, const struct stat* st)
{
    if (state->idx < state->offset) {
        state->idx++;
        return 0;
    }

    int ret = state->filler(state->buf, name, st, state->idx + 1, state->fillFlags);
    if (ret < 0) {
        return ret;
    }
    state->idx++;
    return ret;
}

static int FillDirSubentries(Dir* dir, FillState* state, Context* ctx)
{
    for (const auto& [unused, subDir] : dir->dirs) {
        struct stat st = {};
        st.st_mode = S_IFDIR | DEFAULT_DIR_MODE;
        st.st_nlink = DIR_NLINK_COUNT;
        st.st_uid = ctx->uid();
        st.st_gid = ctx->gid();
        
        if (FillOneEntry(state, subDir->name.c_str(), &st) != 0) {
            return -1;
        }
    }
    return 0;
}

static int FillDirFiles(Dir* dir, FillState* state, Context* ctx)
{
    for (const auto& [unused, file] : dir->files) {
        struct stat st = {};
        st.st_mode = S_IFREG | DEFAULT_FILE_MODE;
        st.st_nlink = 1;
        st.st_uid = ctx->uid();
        st.st_gid = ctx->gid();
        st.st_size = file->size;
        st.st_mtime = NormalizeMtime(file->mtime);
        st.st_blocks = (file->size / SIZE_TO_BLOCK) + (file->size % SIZE_TO_BLOCK > 0 ? 1 : 0);
        
        if (FillOneEntry(state, file->name.c_str(), &st) != 0) {
            return -1;
        }
    }
    return 0;
}

static int Readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                   off_t offset, struct fuse_file_info* fileInfo, FuseReaddirFlags flags)
{
    static std::atomic<uint64_t> g_rd_seq{0};
    uint64_t seq = ++g_rd_seq;
    
    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto Readdir invalid path=%{public}s", path ? path : "null");
        return -EINVAL;
    }
    
    LOGI("gphoto readdir seq=%{public}llu path=%{public}s offset=%{public}lld flags=%{public}d",
        static_cast<unsigned long long>(seq), path, static_cast<long long>(offset), static_cast<int>(flags));

    Context* ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (ctx == nullptr) {
        LOGE("gphoto readdir return by ctx = null");
        return -EIO;
    }

    Dir* dir = FindDir(path, ctx);
    if (dir == nullptr) {
        LOGE("gphoto readdir return by finddir = null");
        return -ENOENT;
    }
    
    LOGI("readdir seq=%{public}llu listed=%{public}d dirs=%{public}zu files=%{public}zu",
        static_cast<unsigned long long>(seq), dir->listed, dir->dirs.size(), dir->files.size());

    if (!dir->listed) {
        int ret = ListDir(path, dir, ctx);
        if (ret) {
            LOGE("gphoto readdir return by listdir = %{public}d\n", ret);
            return ret;
        }
    }

    FillState state = {
        .buf = buf,
        .filler = filler,
        .offset = offset,
        .fillFlags = (flags & FUSE_READDIR_PLUS) ? FUSE_FILL_DIR_PLUS : static_cast<FuseFillDirFlags>(0),
        .idx = 0
    };

    int ret = FillDirSubentries(dir, &state, ctx);
    if (ret != 0) {
        return ret;
    }

    ret = FillDirFiles(dir, &state, ctx);
    if (ret != 0) {
        return ret;
    }

    LOGI("gphoto readdir return 0\n");
    return 0;
}

static int Unlink(const char *path)
{
    LOGI("gphoto unlink enter");
    if (!IsFilePathValid(path)) {
        LOGE("Unlink: Invalid path");
        return -EINVAL;
    }
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    std::string dirName(SmtpfsDirName(path));
    std::string fileName(SmtpfsBaseName(path));

    Dir *dir = FindDir(dirName, ctx);
    File *file = FindFile(path, ctx);
    if (dir == nullptr || file == nullptr) {
        LOGE("Unlink: path not found");
        return -ENOENT;
    }

    unique_lock<mutex> fileGuard(file->lock);
    if (file->ref > 0) {
        LOGE("Unlink: file is still in use (ref count > 0)");
        return -EBUSY;
    }

    int ret = WithCameraLocked([&ctx, &dirName, &fileName] {
        return gp_camera_file_delete(ctx->camera(), dirName.c_str(), fileName.c_str(),
            ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("gphoto unlink return err by ret = %{public}d", ret);
        return GpresultToErrno(ret);
    }

    dir->removeFile(file);
    delete file;
    LOGI("gphoto unlink return 0");
    return 0;
}

static int Mkdir(const char *path, mode_t mode)
{
    LOGI("gphoto mkdir enter");
    if (g_readOnlyMode) {
        LOGE("Mkdir: operation denied in read-only mode");
        return -EROFS;
    }

    if (!IsFilePathValid(path)) {
        LOGE("gphoto Mkdir: Invalid path");
        return -EINVAL;
    }
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    std::string parentName(SmtpfsDirName(path));
    std::string dirName(SmtpfsBaseName(path));
    LOGI("gphoto mkdir enter");
    Dir *parent = FindDir(parentName, ctx);
    if (parent == nullptr) {
        LOGE("gphoto mkdir return by parent = null");
        return -ENOENT;
    }
    int ret = WithCameraLocked([&ctx, &parentName, &dirName] {
        return gp_camera_folder_make_dir(ctx->camera(), parentName.c_str(),
            dirName.c_str(), ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("gphoto mkdir return by ret = %{public}d", ret);
        return GpresultToErrno(ret);
    }
    Dir *dir = new (std::nothrow) Dir(dirName);
    if (dir == nullptr) {
        LOGE("gphoto mkdir failed to allocate Dir");
        return -ENOMEM;
    }
    parent->addDir(dir);
    return 0;
}

static int Rmdir(const char *path)
{
    LOGI("gphoto rmdir enter");
    if (!IsFilePathValid(path)) {
        LOGE("Rmdir: Invalid path");
        return -EINVAL;
    }
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);

    std::string parentName(SmtpfsDirName(path));
    std::string dirName(SmtpfsBaseName(path));

    Dir *parent = FindDir(parentName, ctx);
    Dir *dir = FindDir(path, ctx);
    if (parent == nullptr || dir == nullptr) {
        LOGE("gphoto rmdir return by parent or dir null");
        return -ENOENT;
    }
    if (!dir->empty()) {
        LOGE("gphoto rmdir return by dir->empty");
        return -ENOTEMPTY;
    }

    int ret = WithCameraLocked([&ctx, &parentName, &dirName] {
        return gp_camera_folder_remove_dir(ctx->camera(), parentName.c_str(), dirName.c_str(),
            ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("gphoto rmdir return err by ret = %{public}d", ret);
        return GpresultToErrno(ret);
    }
    LOGI("gphoto rmdir return 0");
    parent->removeDir(dir);
    delete dir;
    return 0;
}

static void* Init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    LOGI("gphoto init enter");
    return new Context();
}

static void Destroy(void *voidContext)
{
    LOGI("gphoto destroy enter");
    Context *context = static_cast<Context *>(voidContext);
    delete context;
    context = nullptr;
    LOGI("gphoto destroy end");
}

static void FillStatfsFromStorageInfo(struct statvfs *stat, const CameraStorageInformation *storageInfo)
{
    stat->f_bsize = STATFS_BLOCK_SIZE;
    stat->f_frsize = STATFS_BLOCK_SIZE;
    stat->f_blocks = (storageInfo->capacitykbytes * KB_TO_BYTES) / STATFS_BLOCK_SIZE;
    stat->f_bfree = (storageInfo->freekbytes * KB_TO_BYTES) / STATFS_BLOCK_SIZE;
    stat->f_bavail = (storageInfo->freekbytes * KB_TO_BYTES) / STATFS_BLOCK_SIZE;
    stat->f_files = STATFS_UNLIMITED;
    stat->f_ffree = STATFS_UNLIMITED;
}

static int Statfs(const char *path, struct statvfs *stat)
{
    LOGI("gphoto statfs enter");
    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto Statfs invalid path");
        return -EINVAL;
    }

    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    CameraStorageInformation *storageInfo = nullptr;
    int numInfo = 0;
    int res = WithCameraLocked([&ctx, &storageInfo, &numInfo] {
        return gp_camera_get_storageinfo(ctx->camera(), &storageInfo, &numInfo, ctx->context());
    });
    if (res != GP_OK) {
        {
            std::lock_guard<std::mutex> lock(g_errMutex);
            g_err = res;
        }
        if (ctx->statCache()) {
            LOGI("gphoto statfs return 0 by statcache() = true");
            *stat = *ctx->statCache();
            return 0;
        }
        LOGE("gphoto statfs returned err by ret = %{public}d", res);
        return GpresultToErrno(res);
    }

    if (numInfo == 0) {
        LOGE("Statfs: no storage found");
        return -EINVAL;
    }

    LOGI("gphoto statfs numinfo = %{public}d", numInfo);
    if (numInfo == 1) {
        FillStatfsFromStorageInfo(stat, storageInfo);
    }
    ctx->cacheStat(stat);
    LOGI("gphoto statfs return 0");
    return 0;
}

static int Chmod(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    LOGI("gphoto chmod enter");
    return 0;
}

static int Chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    LOGI("gphoto chown enter");
    return 0;
}

static int GpOpendir(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("gphoto gp_opendir enter");
    return 0;
}

static int GpRelease(const char *path, struct fuse_file_info *fileInfo)
{
    LOGI("gphoto gp_release enter");
    return 0;
}

static int GpReadlink(const char *path, char *buf, size_t size)
{
    LOGI("GpReadlink: symbolic links not supported");
    return -EINVAL;  // Camera filesystem does not support symbolic links
}
static int GpMknod(const char *path, mode_t mode, dev_t dev)
{
    LOGI("gphoto gp_mknod enter");
    return -EROFS;
}

static int GpSymlink(const char *path, const char *target)
{
    LOGI("gphoto gp_symlink enter");
    return -EROFS;
}

static int GpRename(const char *path, const char *newPath, unsigned int flags)
{
    LOGI("gphoto gp_rename enter");
    return 0;
}

static int GpLink(const char *path, const char *target)
{
    LOGI("gphoto gp_link enter");
    return -EROFS;
}

static int GpFsync(const char *path, int isDataSync, struct fuse_file_info *fi)
{
    LOGI("gphoto gp_fsync enter");
    return 0;
}

static int GpWriteBoolXattr(bool boolValue, char *out, size_t size)
{
    const char *s = boolValue ? "true" : "false";
    const int len = boolValue ? UPLOAD_RECORD_TRUE_LEN : UPLOAD_RECORD_FALSE_LEN;
    if (out == nullptr || size == 0) {
        return len;
    }

    if (size < static_cast<size_t>(len)) {
        LOGE("gphoto GpWriteBoolXattr enter size = %{public}d,", size);
        return -ERANGE;
    }

    if (out != nullptr && s != nullptr && len > 0) {
        int ret = memcpy_s(out, size, s, len);
        if (ret != EOK) {
            LOGE("GpWriteBoolXattr memcpy_s failed, ret=%{public}d", ret);
            return -EIO;
        }
    }
    LOGI("gphoto GpWriteBoolXattr enter out = %{public}s, bool = %{public}s, len = %{public}d", out, s, len);
    return len;
}

static int GpDirFetched(const char *path, const char *in, char *out, size_t size)
{
    Context *ctx = static_cast<Context*>(fuse_get_context()->private_data);
    if (ctx == nullptr) {
        LOGE("gphoto gp_getxattr return ctx is null");
        return -EIO;
    }

    if (strcmp(path, "/") == 0) {
        return GpWriteBoolXattr(true, out, size);
    }

    Dir *dir = FindDirNoFetch(std::string(path), ctx);
    if (dir == nullptr) {
        return GpWriteBoolXattr(false, out, size);
    }

    return GpWriteBoolXattr(dir->listed, out, size);
}

static int GpGetxattr(const char *path, const char *in, char *out, size_t size)
{
    LOGI("gphoto gp_getxattr enter, in = %{public}s", in);

    if (path == nullptr || in == nullptr) {
        LOGE("gphoto gp_getxattr invalid args");
        return -EINVAL;
    }

    if (!IsFilePathValid(path)) {
        LOGE("Getxattr: invalid path");
        return -EINVAL;
    }

    if (strcmp(in, "user.isDirFetched") == 0) {
        LOGI("gphoto gp_getxattr: user.isDirFetched");
        return GpDirFetched(path, in, out, size);
    }

    return 0;
}

static int GpSetxattr(const char *path, const char *ch1, const char *ch2, size_t st1, int st2)
{
    LOGI("gphoto gp_setxattr enter ch1 = %{public}s, ch2 = %{public}s, "
         "st1 = %{public}d, st2 = %{public}d",
        ch1, ch2, st1, st2);
    return -EROFS;
}

static int GpListxattr(const char *path, char *ch1, size_t st1)
{
    LOGI("gphoto gp_listxattr enter ch1 = %{public}s, st1 = %{public}d", ch1, st1);
    return 0;
}

static int GpRemovexattr(const char *path, const char *ch1)
{
    LOGI("gphoto gp_removexattr enter ch1 = %{public}s", ch1);
    return -EROFS;
}

static int GpFsyncdir(const char *path, int isDataSync, struct fuse_file_info *fi)
{
    LOGI("gphoto gp_fsyncdir enter");
    return 0;
}

static int GpAccess(const char *path, int mask)
{
    LOGI("gphoto gp_access enter");
    return 0;
}

static int GpLock(const char *path, struct fuse_file_info *fi, int cmd,
                  struct flock *lockInfo)
{
    LOGI("gphoto gp_lock enter");
    return 0;
}

static int GpUtimens(const char *path, const struct timespec tv[2],
                     struct fuse_file_info *fi)
{
    LOGI("gphoto GpUtimens enter");
    return 0;
}

static int GpIoctl(const char *path, int cmd, void *arg,
                   struct fuse_file_info *fi, unsigned int flags, void *data)
{
    LOGI("gphoto gp_ioctl enter");
    return 0;
}

static int GpPoll(const char *path, struct fuse_file_info *fi,
                  struct fuse_pollhandle *ph, unsigned *reventsp)
{
    LOGI("gphoto gp_poll enter");
    return 0;
}

static int GpFlock(const char *path, struct fuse_file_info *fi, int op)
{
    LOGI("gphoto gp_flock enter");
    return 0;
}

static int GpCancel(const char *path, struct fuse_file_info *fi, int op)
{
    LOGI("gphoto gp_cancel enter");
    return 0;
}

fuse_operations fuseOperations_ = {
    .init = Init,
    .destroy = Destroy,
    .statfs = Statfs,
    .chown = Chown,
    .chmod = Chmod,
    .truncate = Truncate,
    .getattr = Getattr,
    .readdir = Readdir,
    .mkdir = Mkdir,
    .rmdir = Rmdir,
    .create = Create,
    .open = Open,
    .release = Release,
    .unlink = Unlink,
    .read = Read,
    .write = Write,
    .flush = Flush,
    .readlink = GpReadlink,
    .mknod = GpMknod,
    .symlink = GpSymlink,
    .rename = GpRename,
    .link = GpLink,
    .utimens = GpUtimens,
    .fsync = GpFsync,
    .getxattr = GpGetxattr,
    .setxattr = GpSetxattr,
    .listxattr = GpListxattr,
    .removexattr = GpRemovexattr,
    .opendir = GpOpendir,
    .releasedir = GpRelease,
    .fsyncdir = GpFsyncdir,
    .access = GpAccess,
    .ioctl = GpIoctl,
    .lock = GpLock,
    .flock = GpFlock,
    .poll = GpPoll,
    .bmap = nullptr,
    .read_buf = nullptr,
    .lseek = nullptr,
    .copy_file_range = nullptr,
    .fallocate = nullptr,
    .write_buf = nullptr
};