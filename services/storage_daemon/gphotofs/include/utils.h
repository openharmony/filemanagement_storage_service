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
#ifndef GPHOTOFS2_UTILS_H
#define GPHOTOFS2_UTILS_H

#include <string>

constexpr size_t SIZE_TO_BLOCK = 512;
constexpr const char *TMP_FILE_PREFIX = "gphoto_";
constexpr const char *TMP_FILE_SUFFIX = ".tmp";
constexpr const char *TMP_FULL_PATH = "/data/local/gphoto_tmp";

int Now();
void Error(const std::string& msg);
void Warn(const std::string& msg);
void Debug(const std::string& msg);
off_t SizeToBlocks(off_t size);
int GpresultToErrno(int result);
std::string SmtpfsDirName(const std::string &path);
std::string SmtpfsBaseName(const std::string &path);
bool IsFilePathValid(const std::string &filePath);
bool SmtpfsCreateDir(const std::string &dirName);
bool CreateTmpDir();
bool RemoveTmpDir();
bool SmtpfsRemoveDir(const std::string &dirName);
bool SmtpfsCheckDir(const std::string &path);
void DelTemp(const std::string &path);
std::string SmtpfsGetTmpDir();

#endif // GPHOTOFS2_UTILS_H