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

#ifndef FILEMANAGEMENT_DFS_ZIP_UTILS_H
#define FILEMANAGEMENT_DFS_ZIP_UTILS_H

#include <string>
#include <contrib/minizip/zip.h>

namespace OHOS::Storage::DistributedFile {
enum class KeepStatus {
    KEEP_NONE_PARENT_PATH,
    KEEP_ONE_PARENT_PATH,
};

class ZipUtil {
public:
    static zipFile CreateZipFile(const std::string& zipPath, int32_t zipMode = APPEND_STATUS_CREATE);
    static void CloseZipFile(zipFile& zipfile);
    static int AddFileInZip(zipFile& zipfile, const std::string& srcFile,
        KeepStatus keepParentPathStatus, const std::string& dstFileName = "");
    static std::string GetDestFilePath(
        const std::string& srcFile, const std::string& destFilePath, KeepStatus keepParentPathStatus);

private:
    static FILE* GetFileHandle(const std::string& file);
};
} // namespace OHOS::Storage::DistributedFile
#endif // FILEMANAGEMENT_DFS_ZIP_UTILS_H