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

// This wrapper compiles file_utils.cpp with system call redirects for mock testing.
// The #define redirects transform pipe/fork calls into mock functions
// (PipeMock/ForkMock) implemented in file_utils_mock_test.cpp.

#include "mock/file_utils_func_define.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wheader-hygiene"
#include "utils/file_utils.cpp"
#pragma GCC diagnostic pop
#include "mock/file_utils_func_undef.h"
