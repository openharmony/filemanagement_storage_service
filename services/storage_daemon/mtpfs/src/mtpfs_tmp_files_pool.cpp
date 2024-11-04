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

#include "mtpfs_tmp_files_pool.h"

#include <openssl/sha.h>
#include <securec.h>
#include <sstream>

#include "mtpfs_util.h"
#include "storage_service_log.h"

MtpFsTmpFilesPool::MtpFsTmpFilesPool() : tmpDir_(SmtpfsGetTmpDir()), pool_() {}

MtpFsTmpFilesPool::~MtpFsTmpFilesPool() {}

std::string GetSha256Hash(const std::string &input)
{
    const int32_t sha256HashBitNum = 2;
    if (input.size() == 0) {
        LOGE("Input param invalied.");
        return "";
    }
    unsigned char hash[SHA256_DIGEST_LENGTH * sha256HashBitNum + 1] = "";
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input.c_str(), input.size());
    SHA256_Final(hash, &ctx);

    char res[SHA256_DIGEST_LENGTH * sha256HashBitNum + 1];
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        if (sprintf_s(&res[i * sha256HashBitNum], sizeof(res), "%02x", hash[i]) < 0) {
            LOGE("sprintf_s error");
            return "";
        }
    }
    return std::string(res);
}

void MtpFsTmpFilesPool::RemoveFile(const std::string &path)
{
    auto it = std::find(pool_.begin(), pool_.end(), path);
    if (it == pool_.end()) {
        return;
    }
    pool_.erase(it);
}

const MtpFsTypeTmpFile *MtpFsTmpFilesPool::GetFile(const std::string &path) const
{
    auto it = std::find(pool_.begin(), pool_.end(), path);
    if (it == pool_.end()) {
        return nullptr;
    }
    return static_cast<const MtpFsTypeTmpFile *>(&*it);
}

std::string MtpFsTmpFilesPool::MakeTmpPath(const std::string &pathDevice) const
{
    static int cnt = 0;
    std::stringstream ss;
    ss << pathDevice << ++cnt;
    return tmpDir_ + std::string("/") + GetSha256Hash(pathDevice);
}

bool MtpFsTmpFilesPool::CreateTmpDir()
{
    if (RemoveTmpDir()) {
        return SmtpfsCreateDir(tmpDir_);
    }
    return false;
}

bool MtpFsTmpFilesPool::RemoveTmpDir()
{
    if (!tmpDir_.empty()) {
        return SmtpfsRemoveDir(tmpDir_);
    }
    return false;
}
