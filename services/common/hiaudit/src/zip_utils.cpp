
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

#include "zip_utils.h"

#include <securec.h>

#include "storage_service_log.h"

#include "utils/file_utils.h"
#include "storage_service_errno.h"

namespace OHOS::Storage::DistributedFile {
namespace {
constexpr int READ_MORE_LENGTH = 100 * 1024;
};

zipFile ZipUtil::CreateZipFile(const std::string& zipPath, int32_t zipMode)
{
    return zipOpen(zipPath.c_str(), zipMode);
}

void ZipUtil::CloseZipFile(zipFile& zipfile)
{
    if (zipfile == nullptr) {
        LOGE("CloseZipFile zipfile is nullptr.");
        return;
    }
    zipClose(zipfile, nullptr);
}

int ZipUtil::AddFileInZip(
    zipFile& zipfile, const std::string& srcFile, KeepStatus keepParentPathStatus, const std::string& destFileName)
{
    zip_fileinfo zipInfo;
    errno_t result = memset_s(&zipInfo, sizeof(zipInfo), 0, sizeof(zipInfo));
    if (result != EOK) {
        LOGE("AddFileInZip memset_s error, file:%{public}s.", srcFile.c_str());
        return E_MEMORY_OPERATION_ERR;
    }
    FILE *srcFp = GetFileHandle(srcFile);
    if (srcFp == nullptr) {
        LOGE("get file handle failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
        return E_OPEN_FAILED;
    }
    std::string srcFileName = GetDestFilePath(srcFile, destFileName, keepParentPathStatus);
    if (srcFileName.empty()) {
        LOGE("GetDestFilePath returned empty path, srcFile:%{public}s, destFileName:%{public}s.",
            srcFile.c_str(), destFileName.c_str());
        (void)fclose(srcFp);
        return E_FILE_PATH_INVALID;
    }
    int errcode = zipOpenNewFileInZip(
        zipfile, srcFileName.c_str(), &zipInfo, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (errcode != ZIP_OK) {
        LOGE("Failed to zipOpenNewFileInZip,file = %{public}s, errcode: %{public}d.", srcFile.c_str(), errcode);
        (void)fclose(srcFp);
        srcFp = nullptr;
        return E_OPEN_FAILED;
    }

    char buf[READ_MORE_LENGTH] = {0};
    size_t numBytes;
    while ((numBytes = fread(buf, 1, sizeof(buf), srcFp)) > 0) {
        errcode = zipWriteInFileInZip(zipfile, buf, static_cast<unsigned int>(numBytes));
        if (errcode != ZIP_OK) {
            LOGE("Failed to zipWriteInFileInZip, file = %{public}s, errcode: %{public}d.", srcFile.c_str(), errcode);
            break;
        }
        if (ferror(srcFp)) {
            LOGE("zip file failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
            errcode = errno;
            break;
        }
    }
    (void)fclose(srcFp);
    int zipCloseRet = zipCloseFileInZip(zipfile);
    if (zipCloseRet) {
        LOGE("zipCloseFileInZip failed, zipCloseRet: %{public}d, errno: %{public}d.", zipCloseRet, errno);
        return zipCloseRet;
    }
    return errcode;
}

FILE* ZipUtil::GetFileHandle(const std::string& file)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(file.c_str(), realPath) == nullptr) {
        LOGE("GetFileHandle realpath failed, errno: %{public}d.", errno);
        return nullptr;
    }
    return fopen(realPath, "rb");
}

std::string ZipUtil::GetDestFilePath(
    const std::string& srcFile, const std::string& destFilePath, KeepStatus keepParentPathStatus)
{
    if (!destFilePath.empty()) {
        if (StorageDaemon::IsFilePathInvalid(destFilePath)) {
            LOGE("Invalid destFilePath: %{public}s, contains path traversal characters", destFilePath.c_str());
            return "";
        }
        return destFilePath;
    }
    std::string file = srcFile;
    std::string result = file;
    std::string parentPathName;
    auto pos = file.rfind("/");
    if (pos != std::string::npos && pos != file.length() - 1) {
        result = file.substr(pos + 1);
        std::string parent = file.substr(0, pos);
        pos = parent.rfind("/");
        if (pos != std::string::npos && pos != parent.length() - 1) {
            parentPathName = parent.substr(pos + 1);
        } else {
            parentPathName = parent;
        }
    }
    parentPathName.append("/");
    if (keepParentPathStatus == KeepStatus::KEEP_ONE_PARENT_PATH) {
        // srcFileName with relative directory path
        result.insert(0, parentPathName);
    }
    return result;
}
}