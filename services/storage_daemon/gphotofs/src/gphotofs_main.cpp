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
#include "gphotofs_fuse.h"
#include <string>
#include "storage_service_log.h"
#include <clocale>
#include "utils.h"

using namespace std;

int main(int argc, char **argv)
{
    LOGI("gphotofs main enter");
    if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
        LOGE("Failed to set locale to en_US.UTF-8");
        return 1;
    }

    bool ret = GphotoFileSystem::GetInstance().ParseOptions(argc, argv);
    if (!ret) {
        LOGE("gphotofs main return ParseOptions ret false");
        return 1;
    }
    ret = GphotoFileSystem::GetInstance().Exec();
    if (!ret) {
        LOGE("gphotofs main return exec ret false");
        return 1;
    }
    LOGI("gphotofs main exec ret success");
    return 0;
}