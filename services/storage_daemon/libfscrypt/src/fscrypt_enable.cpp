/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fscrypt_enable.h"
#include "v1_1/ifactory_interface.h"

#include <string>

constexpr int RETRY_TIMES = 3;

constexpr int OEMINFO_FILE_ENCRYPTION_SIZE = 5;
constexpr int OEMINFO_FILE_ENCRYPTION_STATUS_MAIN_ID = 211;
constexpr int OEMINFO_FILE_ENCRYPTION_STATUS_SUB_ID = 24;

std::string CRYPTED_DISABLE = "false";
std::string CRYPTED_NOT_ENABLE = "0";
std::string CRYPTED_FORCE_DISABLE = "2";

static FS_CRYPT_ENABLE_STATUS fsCryptEnable = UNDEFINED;
extern "C" {
int IsFsCryptEnableByOemInfo()
{
    if (fsCryptEnable != UNDEFINED) {
        return fsCryptEnable;
    }

    int retryTime = RETRY_TIMES;
    while (fsCryptEnable == UNDEFINED && retryTime > 0) {
        retryTime--;
        OHOS::sptr<OHOS::HDI::Factory::V1_0::IFactoryInterface> factoryInterfaceImpl =
            OHOS::HDI::Factory::V1_1::IFactoryInterface::Get("factory_interface_service", true);
        if (factoryInterfaceImpl == nullptr) {
            fsCryptEnable = UNDEFINED;
            continue;
        }
        std::string readData;
        int readRet = factoryInterfaceImpl->OeminfoReadReused(OEMINFO_FILE_ENCRYPTION_STATUS_MAIN_ID,
            OEMINFO_FILE_ENCRYPTION_STATUS_SUB_ID,
            OEMINFO_FILE_ENCRYPTION_SIZE,
            readData);
        if (readRet == 0) {
            if (readData == CRYPTED_DISABLE || readData == CRYPTED_NOT_ENABLE || readData == CRYPTED_FORCE_DISABLE) {
                fsCryptEnable = DISABLE;
            } else {
                fsCryptEnable = ENABLE;
            }
        }
    }
    return fsCryptEnable;
}
}
