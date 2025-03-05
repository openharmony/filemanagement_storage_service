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

#include "zip_util.h"
#include <securec.h>

#include "storage_service_log.h"

namespace OHOS::StorageDaemon {
namespace {
constexpr int READ_MORE_LENGTH = 100 * 1024;
constexpr int ERROR_MEMSET_STRUCT = 1001;
constexpr int ERROR_GET_HANDLE = 1002;
constexpr int ERROR_ZIP_OPEN = 1003;
};

zipFile ZipUtil::CreateZipFile(const std::string& zipPath, int32_t zipMode)
{
    return zipOpen(zipPath.c_str(), zipMode);
}

void ZipUtil::CloseZipFile(zipFile& zipfile)
{
    zipClose(zipfile, nullptr);
}

int ZipUtil::AddFileInZip(
    zipFile& zipfile, const std::string& srcFile, int keepParentPathStatus, const std::string& destFileName)
{
    zip_fileinfo zipInfo;
    errno_t result = memset_s(&zipInfo, sizeof(zipInfo), 0, sizeof(zipInfo));
    if (result != EOK) {
        LOGE("AddFileInZip memset_s error, file:%{public}s.", srcFile.c_str());
        return ERROR_MEMSET_STRUCT;
    }
    FILE *srcFp = GetFileHandle(srcFile);
    if (srcFp == nullptr) {
        LOGE("get file handle failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
        return ERROR_GET_HANDLE;
    }
    std::string srcFileName = GetDestFilePath(srcFile, destFileName, keepParentPathStatus);
    int err = zipOpenNewFileInZip(
        zipfile, srcFileName.c_str(), &zipInfo, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (err != ZIP_OK) {
        LOGE("Minizip failed to zipOpenNewFileInZip,file = %{public}s, errno: %{public}d.", srcFile.c_str(), err);
        (void)fclose(srcFp);
        return ERROR_ZIP_OPEN;
    }

    int errcode = 0;
    char buf[READ_MORE_LENGTH] = {0};
    while (!feof(srcFp)) {
        size_t numBytes = fread(buf, 1, sizeof(buf), srcFp);
        if (numBytes == 0) {
            break;
        }
        zipWriteInFileInZip(zipfile, buf, static_cast<unsigned int>(numBytes));
        if (ferror(srcFp)) {
            LOGE("zip file failed:%{public}s, errno: %{public}d.", srcFile.c_str(), errno);
            errcode = errno;
            break;
        }
    }
    (void)fclose(srcFp);
    zipCloseFileInZip(zipfile);
    return errcode;
}

FILE* ZipUtil::GetFileHandle(const std::string& file)
{
    char realPath[PATH_MAX] = {0};
    if (realpath(file.c_str(), realPath) == nullptr) {
        return nullptr;
    }
    return fopen(realPath, "rb");
}

std::string ZipUtil::GetDestFilePath(
    const std::string& srcFile, const std::string& destFilePath, int keepParentPathStatus)
{
    if (!destFilePath.empty()) {
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
    if (keepParentPathStatus == KEEP_ONE_PARENT_PATH) {
        // srcFileName with relative directory path
        result.insert(0, parentPathName);
    }
    return result;
}
}
