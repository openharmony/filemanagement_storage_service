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
#include <thread>
#include "storage_service_log.h"
#include "gphotofs_utils.h"

using FuseFillDirFlags = fuse_fill_dir_flags;
using FuseReaddirFlags = fuse_readdir_flags;

static constexpr int32_t UPLOAD_RECORD_TRUE_LEN  = 4;
static constexpr int32_t UPLOAD_RECORD_FALSE_LEN = 5;
static constexpr const char *GPHOTO_THM_FLAG = "?MTP_THM";
static constexpr size_t MAX_MALLOC_SIZE = 100 * 1024 * 1024;
static constexpr size_t MAX_WRITE_SIZE = 10 * 1024 * 1024;
static constexpr size_t PROGRESS_THREAD_THRESHOLD = 50 * 1024 * 1024;
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
static const int DEFAULT_COUNT = 10;
static constexpr int MONITOR_INTERVAL_MS = 500;
static const unsigned int FORCE_REFESH_FLAG = 0x1;
static std::atomic<bool> g_loadOnGoing{false};
static std::atomic<bool> g_running{true};
static std::atomic<int> g_gphotoLockDepth{0};

struct LockGuard {
    LockGuard()
    {
        g_gphotoLockDepth.fetch_add(1);
    }
    ~LockGuard()
    {
        g_gphotoLockDepth.fetch_sub(1);
    }
    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
};

template<typename F>
auto WithCameraLocked(F&& f) -> decltype(f())
{
    std::lock_guard<std::mutex> guard(g_gphotoMutex);
    LockGuard depthGuard;
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
    void *buf;
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

static Context *GetContext()
{
    struct fuse_context *ctxStruct = fuse_get_context();
    if (ctxStruct == nullptr || ctxStruct->private_data == nullptr) {
        return nullptr;
    }
    return static_cast<Context *>(ctxStruct->private_data);
}

static int FillToDir(CameraList *list, Dir *dir, Context *ctx, const char *path, bool isDir)
{
    if (!list || !dir || !ctx || !path) {
        return -EINVAL;
    }
    int count = gp_list_count(list);
    for (int k = 0; k < count; k++) {
        const char *name = nullptr;
        gp_list_get_name(list, k, &name);
        if (!name || !name[0]) {
            continue;
        }
        if (isDir) {
            if (dir->GetDir(name)) {
                continue;
            }
            auto subDir = std::make_unique<Dir>(name);
            dir->AddDir(subDir.release());
            continue;
        }
        if (dir->GetFile(name)) {
            continue;
        }
        CameraFileInfo info;
        int ret = WithCameraLocked([ctx, path, name, &info] {
            return gp_camera_file_get_info(ctx->camera(), path, name, &info, ctx->context());
        });
        if (ret != GP_OK) {
            LOGE("gp_camera_file_get_info failed ret=%{public}d", ret);
            continue;
        }

        auto file = std::make_unique<File>(name, info);
        dir->AddFile(file.release());
        }
    return 0;
    }

static int CreateLists(CameraList **files, CameraList **dirs)
{
    int ret = gp_list_new(files);
    if (ret != GP_OK) {
        return GpresultToErrno(ret);
    }
    ret = gp_list_new(dirs);
    if (ret != GP_OK) {
        gp_list_free(*files);
        *files = nullptr;
        return GpresultToErrno(ret);
    }
    return 0;
}

static bool NeedListDir(Dir *dir)
{
    if (dir == nullptr) {
        return false;
    }
    if (g_loadOnGoing.load()) {
        return false;
    }
    return dir->TryBeginLoad();
}

static int FetchOnePage(const char *path, Dir *dir, Context *ctx, int &start, unsigned int &flags)
{
    CameraList *listFile = nullptr;
    CameraList *listDir = nullptr;
    int ret = CreateLists(&listFile, &listDir);
    if (ret != 0) {
        return ret;
    }
    PartialListRequest req = { path, start, DEFAULT_COUNT, listFile, listDir, flags };
    ret = WithCameraLocked([ctx, &req] {
        return gp_camera_folder_list_partial(ctx->camera(), ctx->context(), &req);
    });
    if (ret != GP_OK) {
        gp_list_free(listFile);
        gp_list_free(listDir);
        return GpresultToErrno(ret);
    }
    int got = gp_list_count(listFile) + gp_list_count(listDir);
    FillToDir(listFile, dir, ctx, path, false);
    FillToDir(listDir, dir, ctx, path, true);
    gp_list_free(listFile);
    gp_list_free(listDir);
    if (got > 0) {
        if (start > INT_MAX - got) {
            return -EOVERFLOW;
        }
        start += got;
        dir->SetNextOffset(start);
    }
    flags = 0;
    return got;
}

static int ListFolderPartial(const char *path, Dir *dir, Context *ctx, bool total)
{
    if (!path || !dir || !ctx) {
        return -EINVAL;
    }
    int start = dir->GetNextOffset();
    unsigned int flags = 0;
    if (dir->GetRefresh() && start == 0) {
        flags = FORCE_REFESH_FLAG;
        dir->Clear();
    }
    bool continueLoop = true;
    while (continueLoop) {
        int got = FetchOnePage(path, dir, ctx, start, flags);
        if (got < 0) {
            return got;
        }
        if (got <= 0 || got < DEFAULT_COUNT) {
            dir->SetListed(true);
            continueLoop = false;
        }
        if (!total) {
            continueLoop = false;
        }
    }
    return GP_OK;
}

static int ListDir(const char *path, Dir *dir, Context *ctx)
{
    LOGI("gphoto listdir enter");
    if (!path || !ctx || !dir) {
        LOGE("gphoto ListDir invalid args, path/ctx/dir=nullptr");
        return -EINVAL;
    }

    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto ListDir invalid path");
        return -EINVAL;
    }
    g_loadOnGoing.store(true);
    int ret = ListFolderPartial(path, dir, ctx, true);
    g_loadOnGoing.store(false);
    if (ret != GP_OK) {
        return ret;
    }
    return 0;
}

static Dir *FindSubDir(Dir *currentDir, const std::string &name, const std::string &dirPath,
    Context *ctx)
{
    if (currentDir == nullptr) {
        LOGE("FindSubDir: currentDir is null");
        return nullptr;
    }
    if (ctx == nullptr) {
        LOGE("FindSubDir: ctx is null");
        return nullptr;
    }
    if (NeedListDir(currentDir)) {
        ListDir(dirPath.c_str(), currentDir, ctx);
        currentDir->EndLoad();
    }
    return currentDir->GetDir(name);
}

static std::string UpdateDirPath(const std::string &currentPath, const std::string &name)
{
    if (currentPath == "/") {
        return "/" + name;
    }
    return currentPath + "/" + name;
}

Dir *FindDir(const std::string &path, Context *ctx)
{
    if (ctx == nullptr) {
        LOGE("FindDir: ctx is null");
        return nullptr;
    }
    Dir *dir = &ctx->root();
    std::string dirPath = "/";
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
        size_t next = path.find('/', last + 1);
        if (next == std::string::npos) {
            std::string name = path.substr(last + 1);
            if (name == "") {
                return dir;
            }
            dir = FindSubDir(dir, name, dirPath, ctx);
            return dir;
        }

        std::string name = path.substr(last + 1, next - last - 1);
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

File *FindFile(const std::string &path, Context *ctx)
{
    if (ctx == nullptr) {
        LOGE("FindFile: ctx is null");
        return nullptr;
    }
    LOGI("gphoto findfile enter");
    if (!IsFilePathValid(path)) {
        LOGE("FindFile invalid path");
        return nullptr;
    }
    size_t pos = path.rfind("/");
    Dir *dir;
    std::string name;
    if (pos == std::string::npos) {
        dir = &ctx->root();
        if (dir == nullptr) {
            LOGE("gphoto FindFile dir=nullptr");
            return nullptr;
        }
        name = path;
        if (NeedListDir(dir)) {
            int ret = ListDir("/", dir, ctx);
            dir->EndLoad();
            if (ret != 0) {
                LOGE("FindFile ListDir failed, ret=%{public}d", ret);
                return nullptr;
            }
        }
    } else {
        std::string parentPath = path.substr(0, pos + 1);
        dir = FindDir(parentPath, ctx);
        if (dir == nullptr) {
            LOGE("gphoto findfile return null by dir = null");
            return nullptr;
        }
        name = path.substr(pos + 1);
        if (NeedListDir(dir)) {
            int ret = ListDir(parentPath.c_str(), dir, ctx);
            dir->EndLoad();
            if (ret != 0) {
                LOGE("FindFile ListDir failed, ret=%{public}d", ret);
                return nullptr;
            }
        }
    }
    return dir->GetFile(name);
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

    std::string tmpDirResult = GphotoGetTmpDir();
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
    LOGI("CreateTempFile: created tmp success");
    return 0;
}

static void CleanupTempFile(File *file)
{
    file->downloading.store(false);
    if (file->tmpFd >= 0) {
        close(file->tmpFd);
        file->tmpFd = -1;
    }

    if (!file->tmpPath.empty()) {
        unlink(file->tmpPath.c_str());
        file->tmpPath.clear();
    }

    file->tmpFileCreated = false;
    file->downloadedSize.store(0);
}

static void MonitorDownloadProgress(File *file)
{
    if (!file) {
        LOGE("MonitorDownloadProgress file is null");
        return;
    }
    off_t cur = 0;
    while (file->downloading.load() && g_running.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(MONITOR_INTERVAL_MS));
        struct stat st {};
        if (file->tmpFd >= 0 && fstat(file->tmpFd, &st) == 0) {
            if (st.st_size > 0) {
                cur = st.st_size;
                file->downloadedSize.store(cur);
            }
        }
        LOGI("MonitorDownloadProgress downloadedSize: %{public}lld", cur);
    }
    LOGI("MonitorDownloadProgress exit loop");
}

static int PrepareTempFileForSave(File *file, const char *path)
{
    if (!file || !path) {
        return -EINVAL;
    }
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
    return 0;
}

static int CreateCameraFileFromTmpFd(File *file, CameraFile **cf)
{
    int dupFd = dup(file->tmpFd);
    if (dupFd < 0) {
        CleanupTempFile(file);
        return -errno;
    }
    int gpResult = gp_file_new_from_fd(cf, dupFd);
    if (gpResult != GP_OK) {
        close(dupFd);
        CleanupTempFile(file);
        return GpresultToErrno(gpResult);
    }
    return 0;
}

static void TryStartProgressThread(File *file, std::thread &progressThread)
{
    if (!file) {
        return;
    }
    file->downloadedSize.store(0);

    if (file->size < PROGRESS_THREAD_THRESHOLD) {
        return;
    }
    file->downloading.store(true);
    progressThread = std::thread(MonitorDownloadProgress, file);
}

static int FinalizeSavedTempFile(File *file)
{
    if (!file) {
        return -EINVAL;
    }
    off_t end = lseek(file->tmpFd, 0, SEEK_END);
    if (end < 0) {
        CleanupTempFile(file);
        return -errno;
    }

    file->downloadedSize.store(end);
    file->size = end;
    file->tmpFileCreated = true;
    return 0;
}

static int SaveCameraFileToTemp(File *file, const char *path)
{
    int ret = PrepareTempFileForSave(file, path);
    if (ret < 0) {
        return ret;
    }
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        CleanupTempFile(file);
        return -EIO;
    }
    std::string dirName(GphotoDirName(path));
    std::string fileName(GphotoBaseName(path));
    if (dirName.empty() || fileName.empty()) {
        CleanupTempFile(file);
        return -EINVAL;
    }
    CameraFile *cf = nullptr;
    ret = CreateCameraFileFromTmpFd(file, &cf);
    if (ret < 0) {
        return ret;
    }
    std::thread progressThread;
    TryStartProgressThread(file, progressThread);
    int gpResult = WithCameraLocked([&ctx, &dirName, &fileName, &cf] {
        return gp_camera_file_get(ctx->camera(), dirName.c_str(), fileName.c_str(),
                                  GP_FILE_TYPE_NORMAL, cf, ctx->context());
    });
    file->downloading.store(false);
    if (progressThread.joinable()) {
        progressThread.join();
    }
    gp_file_unref(cf);
    if (gpResult != GP_OK) {
        CleanupTempFile(file);
        return GpresultToErrno(gpResult);
    }

    return FinalizeSavedTempFile(file);
}

static inline bool IsThumbPath(const char *path)
{
    if (path == nullptr) {
        return false;
    }
    const size_t pathLen = strlen(path);
    const size_t thumbFlagLen = strlen(GPHOTO_THM_FLAG);
    if (pathLen < thumbFlagLen) {
        return false;
    }
    const char *suffixPos = path + (pathLen - thumbFlagLen);
    return strcmp(suffixPos, GPHOTO_THM_FLAG) == 0;
}

static inline std::string StripThumbFlag(const std::string &thumbPath)
{
    const size_t thumbFlagLen = strlen(GPHOTO_THM_FLAG);
    if (thumbPath.size() < thumbFlagLen) {
        return thumbPath;
    }

    if (thumbPath.compare(thumbPath.size() - thumbFlagLen, thumbFlagLen, GPHOTO_THM_FLAG) != 0) {
        return thumbPath;
    }
    const size_t realPathLen = thumbPath.size() - thumbFlagLen;
    return thumbPath.substr(0, realPathLen);
}

static int FillThumbStat(struct stat *st, time_t mtime, unsigned long dataSize, Context *ctx)
{
    memset_s(st, sizeof(*st), 0, sizeof(*st));
    st->st_mode = S_IFREG | DEFAULT_FILE_MODE;
    st->st_mtime = NormalizeMtime(mtime);
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

static int GetThumbAttr(const char *path, struct stat *st, Context *ctx)
{
    std::string real = StripThumbFlag(path);
    if (!IsFilePathValid(real)) {
        LOGE("GetThumbAttr invalid path");
        return -EINVAL;
    }
    time_t mtime = 0;
    {
        File *file = FindFile(real, ctx);
        if (!file) {
            LOGE("GetThumbAttr file not found");
            return -ENOENT;
        }
        std::lock_guard<std::mutex> fileGuard(file->lock);
        mtime = file->mtime;
    }
    std::string dirName(GphotoDirName(real.c_str()));
    std::string fileName(GphotoBaseName(real.c_str()));
    CameraFile *thm = nullptr;
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

    const char *data = nullptr;
    unsigned long dataSize = 0;
    ret = WithCameraLocked([&thm, &data, &dataSize] {
        return gp_file_get_data_and_size(thm, &data, &dataSize);
    });
    if (ret != GP_OK) {
        LOGE("GetThumbAttr gp_file_get_data_and_size fail, gp_ret=%{public}d", ret);
        gp_file_unref(thm);
        return GpresultToErrno(ret);
    }
    ret = FillThumbStat(st, mtime, dataSize, ctx);
    gp_file_unref(thm);
    return ret;
}

static int FillDirStat(struct stat *st, Context *ctx)
{
    st->st_mode = S_IFDIR | DEFAULT_DIR_MODE;
    st->st_nlink = DIR_NLINK_COUNT;
    st->st_uid = ctx->uid();
    st->st_gid = ctx->gid();
    return 0;
}

static int FillFileStat(struct stat *st, off_t size, time_t mtime, Context *ctx)
{
    st->st_mode = S_IFREG | DEFAULT_FILE_MODE;
    st->st_nlink = FILE_NLINK_COUNT;
    st->st_size = size;
    st->st_blocks = SizeToBlocks(size);
    st->st_mtime = NormalizeMtime(mtime);
    st->st_uid = ctx->uid();
    st->st_gid = ctx->gid();
    return 0;
}

static int GetRegularAttr(const char *path, struct stat *st, Context *ctx)
{
    if (path == nullptr || st == nullptr || ctx == nullptr) {
        return -EINVAL;
    }
    std::string dirName = GphotoDirName(path);
    std::string baseName = GphotoBaseName(path);
    if (dirName.empty() || baseName.empty()) {
        LOGE("Getattr dir/base name empty");
        return -EINVAL;
    }

    Dir *parent = FindDir(dirName, ctx);
    if (parent == nullptr) {
        return -ENOENT;
    }
    if (parent->GetDir(baseName) != nullptr) {
        return FillDirStat(st, ctx);
    }
    off_t size = 0;
    time_t mtime = 0;
    {
        File *file = parent->GetFile(baseName);
        if (file == nullptr) {
            LOGE("Getattr: path not found");
            return -ENOENT;
        }
        std::lock_guard<std::mutex> lockGuard(file->lock);
        size = file->size;
        mtime = file->mtime;
    }
    FillFileStat(st, size, mtime, ctx);
    return 0;
}

static int Getattr(const char *path, struct stat *st, struct fuse_file_info *fi)
{
    if (path == nullptr || st == nullptr) {
        LOGE("Getattr path/st is null");
        return -EINVAL;
    }
    LOGI("gphoto getattr enter");
    Context *ctx = GetContext();
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
    memset_s(st, sizeof(*st), 0, sizeof(*st));
    if (strcmp(path, "/") == 0) {
        return FillDirStat(st, ctx);
    }
    return GetRegularAttr(path, st, ctx);
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

    td->buf = static_cast<char *>(malloc(td->size));
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

    CameraFile *thm = nullptr;
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

    const char *data = nullptr;
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

static int OpenThumb(const char *path, struct fuse_file_info *fi)
{
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("OpenThumb ctx nullptr");
        return -EIO;
    }

    std::string real = StripThumbFlag(path);
    if (!IsFilePathValid(real)) {
        LOGE("OpenThumb invalid path");
        return -EINVAL;
    }

    std::string dirName(GphotoDirName(real.c_str()));
    std::string fileName(GphotoBaseName(real.c_str()));

    auto *td = new (std::nothrow) ThumbDesc();
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

static int ReadThumb(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    if (buf == nullptr) {
        LOGE("ReadThumb: buf is null");
        return -EINVAL;
    }
    if (!fi) {
        LOGE("ReadThumb fi is null");
        return -EIO;
    }
    auto *td = FhToPtr<ThumbDesc>(fi->fh);
    if (!td) {
        LOGE("ReadThumb td nullptr");
        return -EIO;
    }

    std::lock_guard<std::mutex> lockGuard(td->lock);
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

static int ReleaseThumb(const char *path, struct fuse_file_info *fi)
{
    auto *td = FhToPtr<ThumbDesc>(fi->fh);
    if (td) {
        std::lock_guard<std::mutex> lockGuard(td->lock);
        delete td;
        fi->fh = 0;
    }
    return 0;
}

static int InitializeNewFile(File *file, const char *path, Context *ctx)
{
    if (file->camFile == nullptr) {
        std::string dirName(GphotoDirName(path));
        std::string fileName(GphotoBaseName(path));
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

static int CheckCreateCommon(const char *path, Context *&ctx)
{
    if (g_readOnlyMode) {
        LOGE("Create: operation denied in read-only mode");
        return -EROFS;
    }

    if (!IsFilePathValid(path)) {
        LOGE("Create: Invalid path");
        return -EINVAL;
    }

    ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("Create ctx nullptr");
        return -EIO;
    }

    return 0;
}

static int Create(const char *path, mode_t mode,
                  struct fuse_file_info *fileInfo)
{
    Context *ctx = nullptr;
    int ret = CheckCreateCommon(path, ctx);
    if (ret != 0) {
        return ret;
    }
    std::string dirName(GphotoDirName(path));
    std::string fileName(GphotoBaseName(path));

    Dir *dir = FindDir(dirName, ctx);
    if (dir == nullptr || dir->GetFile(fileName) != nullptr) {
        LOGE("Create: parent directory failed");
        return dir == nullptr ? -ENOENT : -EEXIST;
    }

    File *file = new (std::nothrow) File(fileName);
    if (file == nullptr) {
        LOGE("gphoto create failed to allocate File");
        return -ENOMEM;
    }
    {
        std::lock_guard<std::mutex> fileGuard(file->lock);
        file->size = 0;
        file->changed = true;
    }
    
    ret = InitializeNewFile(file, path, ctx);
    if (ret < 0) {
        delete file;
        LOGE("gphoto create InitializeNewFile failed ret=%{public}d", ret);
        return ret;
    }

    FileDesc *fd = new (std::nothrow) FileDesc();
    if (fd == nullptr) {
        LOGE("gphoto create failed to allocate FileDesc");
        delete file;
        return -ENOMEM;
    }
    {
        std::lock_guard<std::mutex> fileGuard(file->lock);
        fd->writable = true;
        fd->file = file;
        file->ref++;
    }
    dir->AddFile(file);
    fileInfo->direct_io = true;
    fileInfo->fh = PtrToFh(fd);
    return 0;
}

static Dir *FindDirNoFetch(const std::string &path, Context *ctx)
{
    if (!IsFilePathValid(path)) {
        LOGE("FindDirNoFetch invalid path");
        return nullptr;
    }
    Dir *dir = &ctx->root();
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
            return dir ? dir->GetDir(name) : nullptr;
        }
        std::string name = normalizedPath.substr(last + 1, next - last - 1);
        if (name.empty()) {
            last = next;
            continue;
        }

        dir = dir ? dir->GetDir(name) : nullptr;
        if (!dir) {
            return nullptr;
        }
        last = next;
    }
    LOGE("FindDirNoFetch unexpected loop exit");
    return nullptr;
}

static int IsDirFetchedGphoto(const std::string &path, Context *ctx, char *out, size_t size)
{
    auto writeBool = [&out, &size](bool boolValue)->int {
        const char *value = boolValue ? "true" : "false";
        size_t len = strlen(value);

        if (out == nullptr || size == 0) {
            return static_cast<int>(len);
        }

        if (size < len) {
            return -ERANGE;
        }
        int ret = memcpy_s(out, size, value, len);
        if (ret != EOK) {
            LOGE("IsDirFetchedGphoto memcpy_s failed, ret=%{public}d", ret);
            return -EIO;
        }
        return static_cast<int>(len);
    };

    if (path == "/") {
        return writeBool(true);
    }

    Dir *dir = FindDirNoFetch(path, ctx);
    if (!dir) {
        return writeBool(false);
    }
    return writeBool(dir->GetListed());
}

static int Open(const char *path, struct fuse_file_info *fileInfo)
{
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
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        return -EIO;
    }

    File *file = FindFile(path, ctx);
    if (file == nullptr) {
        LOGE("Open: file not found");
        return -ENOENT;
    }
    FileDesc *fd = new (std::nothrow) FileDesc();
    if (fd == nullptr) {
        LOGE("gphoto open failed to allocate FileDesc");
        return -ENOMEM;
    }

    std::lock_guard<std::mutex> fileGuard(file->lock);
    fd->writable = mode == O_RDONLY ? false : true;
    fd->file = file;
    file->ref++;
    fileInfo->fh = PtrToFh(fd);
    LOGI("gphoto open success");
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
    std::string dirName(GphotoDirName(path));
    std::string fileName(GphotoBaseName(path));

    CameraFile *upload = nullptr;
    int dupFd = dup(file->tmpFd);
    if (dupFd < 0) {
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
        return ReleaseThumb(path, fileInfo);
    }

    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("Release: ctx is null");
        return -EIO;
    }

    FileDesc *fd = FhToPtr<FileDesc>(fileInfo->fh);
    if (fd == nullptr) {
        LOGE("Release: file descriptor is null");
        return -EIO;
    }
    File *file = fd->file;
    if (file == nullptr) {
        delete fd;
        fileInfo->fh = 0;
        LOGI("Release: file is null");
        return -EIO;
    }
    {
        std::lock_guard<std::mutex> fileGuard(file->lock);
        ret = UploadChangedFile(file, path, ctx);
        delete fd;
        fileInfo->fh = 0;
        if (file->ref <= 0) {
            LOGE("Release: invalid ref count %{public}d", file->ref);
            return -EIO;
        }
        file->ref--;
        if (ret != 0) {
            return ret;
        }
        CleanupTempFile(file);
    }
    LOGI("gphoto release success");
    return 0;
}

static File *ValidateAndGetFile(struct fuse_file_info *fileInfo, const char *path)
{
    Context *ctx = GetContext();
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
    if (g_loadOnGoing.load()) {
        LOGE("loadongoing, not allow read full");
        return -EBUSY;
    }

    File *file = ValidateAndGetFile(fileInfo, path);
    if (!file) {
        return -EIO;
    }
    std::lock_guard<std::mutex> lockGuard(file->lock);
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

    LOGI("gphoto read return size = %{public}zd, want size/offset=%{public}zu/%{public}lld",
        bytesRead, size, static_cast<long long>(offset));
    return static_cast<int>(bytesRead);
}

static File *ValidateFileDesc(struct fuse_file_info *fileInfo, const char *path)
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
    if (!file || !path) {
        return -EINVAL;
    }
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
    if (buf == nullptr) {
        LOGE("Write: buf is null");
        return -EINVAL;
    }
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

    Context *ctx = GetContext();
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
    std::lock_guard<std::mutex> lockGuard(file->lock);
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
    Context *ctx = GetContext();
    if (!ctx) {
        LOGE("gphoto Truncate ctx nullptr");
        return -EIO;
    }
    File *file = FindFile(path, ctx);
    if (file == nullptr) {
        LOGE("gphoto truncate file not found");
        return -ENOENT;
    }
    std::lock_guard<std::mutex> lockGuard(file->lock);
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

    LOGI("gphoto truncate success");
    return 0;
}

static int FillOneEntry(FillState *state, const char *name, const struct stat *st)
{
    if (state->idx < state->offset) {
        state->idx++;
        return 0;
    }

    int ret = state->filler(state->buf, name, st, state->idx + 1, state->fillFlags);
    if (ret != 0) {
        return ret;
    }
    state->idx++;
    return 0;
}

static int FillDirSubentries(Dir *dir, FillState *state, Context *ctx)
{
    for (const auto& [unused, subDir] : dir->dirs) {
        struct stat st = {};
        st.st_mode = S_IFDIR | DEFAULT_DIR_MODE;
        st.st_nlink = DIR_NLINK_COUNT;
        st.st_uid = ctx->uid();
        st.st_gid = ctx->gid();

        int ret = FillOneEntry(state, subDir->name.c_str(), &st);
        if (ret == 1) {
            return 0;
        }
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}

static int FillDirFiles(Dir *dir, FillState *state, Context *ctx)
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

        int ret = FillOneEntry(state, file->name.c_str(), &st);
        if (ret == 1) {
            return 0;
        }
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}

static int Readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                   off_t offset, struct fuse_file_info *fileInfo, FuseReaddirFlags flags)
{
    static std::atomic<uint64_t> g_rd_seq{0};
    uint64_t seq = ++g_rd_seq;

    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto Readdir invalid ");
        return -EINVAL;
    }

    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("gphoto readdir return by ctx = null");
        return -EIO;
    }

    Dir *dir = FindDir(path, ctx);
    if (dir == nullptr) {
        LOGE("gphoto readdir return by finddir = null");
        return -ENOENT;
    }
    if (dir->GetDirty()) {
        LOGI("dir is dirty, not show");
        return 0;
    }

    LOGI("readdir seq=%{public}llu listed=%{public}d dirs=%{public}zu files=%{public}zu",
        static_cast<unsigned long long>(seq), dir->GetListed(), dir->dirs.size(), dir->files.size());

    std::lock_guard<std::mutex> fileGuard(dir->lock);

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
    return 0;
}

static int CheckUnlinkCommon(const char *path, Context *&ctx)
{
    LOGI("gphoto unlink enter");
    if (!IsFilePathValid(path)) {
        LOGE("Unlink: Invalid path");
        return -EINVAL;
    }
    ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("unlink ctx nullptr");
        return -EIO;
    }
    return 0;
}

static int Unlink(const char *path)
{
    Context *ctx = nullptr;
    int ret = CheckUnlinkCommon(path, ctx);
    if (ret != 0) {
        return ret;
    }
    std::string dirName(GphotoDirName(path));
    std::string fileName(GphotoBaseName(path));

    Dir *dir = FindDir(dirName, ctx);
    File *file = FindFile(path, ctx);
    if (dir == nullptr || file == nullptr) {
        LOGE("Unlink: path not found");
        return -ENOENT;
    }
    {
        std::lock_guard<std::mutex> fileGuard(file->lock);
        if (g_gphotoLockDepth.load() > 0) {
            return -EBUSY;
        }
        if (file->ref > 0) {
            LOGE("Unlink: file is still open, ref=%{public}d", file->ref);
            return -EBUSY;
        }
        if (file->downloading.load()) {
            LOGE("Unlink: file is downloading");
            return -EBUSY;
        }
    }

    ret = WithCameraLocked([&ctx, &dirName, &fileName] {
        return gp_camera_file_delete(ctx->camera(), dirName.c_str(), fileName.c_str(),
            ctx->context());
    });
    if (ret != GP_OK) {
        LOGE("gphoto unlink return err by ret = %{public}d", ret);
        return GpresultToErrno(ret);
    }

    dir->RemoveFile(file);
    delete file;
    LOGI("gphoto unlink success");
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
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("mkdir ctx nullptr");
        return -EIO;
    }
    std::string parentName(GphotoDirName(path));
    std::string dirName(GphotoBaseName(path));
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
    parent->AddDir(dir);
    LOGI("gphoto mkdir success");
    return 0;
}

static int Rmdir(const char *path)
{
    LOGI("gphoto rmdir enter");
    if (!IsFilePathValid(path)) {
        LOGE("Rmdir: Invalid path");
        return -EINVAL;
    }
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("rmdir ctx nullptr");
        return -EIO;
    }

    std::string parentName(GphotoDirName(path));
    std::string dirName(GphotoBaseName(path));

    Dir *parent = FindDir(parentName, ctx);
    Dir *dir = FindDir(path, ctx);
    if (parent == nullptr || dir == nullptr) {
        LOGE("gphoto rmdir return by parent or dir null");
        return -ENOENT;
    }
    if (!dir->Empty()) {
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
    LOGI("gphoto rmdir success");
    parent->RemoveDir(dir);
    delete dir;
    return 0;
}

static void *Init(struct fuse_conn_info *conn, struct fuse_config *cfg)
{
    LOGI("gphoto init enter");
    g_running.store(true);
    return new Context();
}

static void Destroy(void *voidContext)
{
    LOGI("gphoto destroy enter");
    g_running.store(false);
    Context *context = static_cast<Context *>(voidContext);
    delete context;
    LOGI("gphoto destroy end");
}

static void FillStatfsFromStorageInfo(struct statvfs *stat, const CameraStorageInformation *storageInfo)
{
    if (stat == nullptr) {
        LOGE("FillStatfsFromStorageInfo: stat is null");
        return;
    }
    if (storageInfo == nullptr) {
        LOGE("FillStatfsFromStorageInfo: storageInfo is null");
        return;
    }
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
    if (stat == nullptr) {
        LOGE("Statfs: stat is null");
        return -EINVAL;
    }
    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto Statfs invalid path");
        return -EINVAL;
    }

    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("Statfs: ctx is null");
        return -EINVAL;
    }
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
    static std::atomic<uint64_t> g_rd_seq{0};
    uint64_t seq = ++g_rd_seq;
    if (!IsFilePathValid(std::string(path))) {
        LOGE("gphoto gp_opendir invalid ");
        return -EINVAL;
    }
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("gphoto gp_opendir return by ctx = null");
        return -EIO;
    }
    Dir *dir = FindDir(path, ctx);
    if (dir == nullptr) {
        LOGE("gphoto gp_opendir return by finddir = null");
        return -ENOENT;
    }
    LOGI("gp_opendir seq=%{public}llu listed=%{public}d dirs=%{public}zu files=%{public}zu",
        static_cast<unsigned long long>(seq), dir->GetListed(), dir->dirs.size(), dir->files.size());
    g_loadOnGoing.store(true);
    int ret = ListFolderPartial(path, dir, ctx, false);
    if (!ret && dir->GetDirty()) {
        dir->SetDirty(false);
    }
    if (ret) {
        dir->SetListed(true);
        dir->SetDirty(true);
        LOGE("gphoto gp_opendir return 0 err = %{public}d", ret);
    }
    g_loadOnGoing.store(!dir->GetListed());
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
    return -EINVAL;
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
    return -EROFS;
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
    const char *value = boolValue ? "true" : "false";
    const int len = boolValue ? UPLOAD_RECORD_TRUE_LEN : UPLOAD_RECORD_FALSE_LEN;
    if (out == nullptr || size == 0) {
        return len;
    }

    if (size < static_cast<size_t>(len)) {
        LOGE("gphoto GpWriteBoolXattr enter size = %{public}d,", size);
        return -ERANGE;
    }

    if (out != nullptr && value != nullptr && len > 0) {
        int ret = memcpy_s(out, size, value, len);
        if (ret != EOK) {
            LOGE("GpWriteBoolXattr memcpy_s failed, ret=%{public}d", ret);
            return -EIO;
        }
    }
    LOGI("gphoto GpWriteBoolXattr enter out = %{public}s, bool = %{public}s, len = %{public}d", out, value, len);
    return len;
}

static int GpDirFetched(const char *path, const char *in, char *out, size_t size)
{
    Context *ctx = GetContext();
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

    return GpWriteBoolXattr(dir->GetListed(), out, size);
}

static int GpDirToFlush(const char *path)
{
    Context *ctx = GetContext();
    if (ctx == nullptr) {
        LOGE("GpDirToFlush return ctx is null");
        return -EIO;
    }

    Dir *dir = FindDirNoFetch(std::string(path), ctx);
    if (dir == nullptr) {
        return 0;
    }
    if (dir->GetListed() && g_gphotoLockDepth.load() == 0) {
        dir->SetListed(false);
        dir->SetNextOffset(0);
        dir->SetRefresh(true);
    }
    LOGI("GpDirToFlush, nextoffset = %{public}d, g_gphotoLockDepth = %{public}d",
         dir->GetNextOffset(), g_gphotoLockDepth.load());
    return 0;
}

static int QueryPtpIsInUse(char *out, size_t size)
{
    if (g_gphotoLockDepth.load() > 0) {
        return GpWriteBoolXattr(true, out, size);
    }
    return GpWriteBoolXattr(false, out, size);
}

static int GpGetxattr(const char *path, const char *in, char *out, size_t size)
{
    if (path == nullptr || in == nullptr) {
        LOGE("gphoto gp_getxattr invalid args");
        return -EINVAL;
    }
    LOGI("gphoto gp_getxattr enter, in = %{public}s", in);
    if (!IsFilePathValid(path)) {
        LOGE("Getxattr: invalid path");
        return -EINVAL;
    }

    if (strcmp(in, "user.isDirFetched") == 0) {
        LOGI("gphoto gp_getxattr: user.isDirFetched");
        return GpDirFetched(path, in, out, size);
    } else if (strcmp(in, "user.queryMtpIsInUse") == 0) {
        return QueryPtpIsInUse(out, size);
    }
    return 0;
}

static int GpSetxattr(const char *path, const char *in, const char *out, size_t size, int flags)
{
    if (!path || !in) {
        return -EINVAL;
    }
    LOGI("gphoto gp_setxattr enter in = %{public}s, size = %{public}zu, flags = %{public}d",
        in, size, flags);
    if (strcmp(in, "user.fetchcontent") == 0) {
        return GpDirToFlush(path);
    }
    return -EROFS;
}

static int GpListxattr(const char *path, char *out, size_t size)
{
    LOGI("gphoto gp_listxattr enter size = %{public}zu", size);
    return 0;
}

static int GpRemovexattr(const char *path, const char *in)
{
    LOGI("gphoto gp_removexattr enter in = %{public}s", in);
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