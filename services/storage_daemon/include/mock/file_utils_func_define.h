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

#ifndef OHOS_STORAGE_DAEMON_FILE_UTILS_FUNC_DEFINE_H
#define OHOS_STORAGE_DAEMON_FILE_UTILS_FUNC_DEFINE_H

// Only redirect pipe and fork — these are used in the parent process path
// and can be mocked without affecting other functions in file_utils.cpp.
// open/close/dup2/_exit are NOT redirected because:
//   1. #define open/close would break std::ifstream::open() and other calls
//   2. Child process branches (open/dup2 failures) run after fork() and
//      call _exit(), which bypasses gcov cleanup, so coverage is not captured

#define pipe PipeMock
#define fork ForkMock

#endif // OHOS_STORAGE_DAEMON_FILE_UTILS_FUNC_DEFINE_H
