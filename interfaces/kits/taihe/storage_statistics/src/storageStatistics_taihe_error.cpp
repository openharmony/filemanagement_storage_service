/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "storageStatistics_taihe_error.h"
#include "storage_manager_connect.h"
#include "n_error.h"
#include "n_val.h"
#include "uv.h"
#include <cstring>

namespace OHOS {

static int ConvertUVCode2ErrCode(int errCode)
{
    if (errCode >= 0) {
        return errCode;
    }
    auto uvCode = std::string_view(uv_err_name(errCode));
    if (FileManagement::LibN::uvCode2ErrCodeTable.find(uvCode) != FileManagement::LibN::uvCode2ErrCodeTable.end()) {
        return FileManagement::LibN::uvCode2ErrCodeTable.at(uvCode);
    }
    return FileManagement::LibN::UNKROWN_ERR;
}

int32_t StorageTaiheError::Convert(const int errCode)
{
    int genericCode = ConvertUVCode2ErrCode(errCode);
    auto it = STORAGE_TAIHE_ERR_MAP.find(genericCode);
    if (it != STORAGE_TAIHE_ERR_MAP.end()) {
        return it->second.errorCode;
    } else {
        return FileManagement::LibN::UNKROWN_ERR;
    }
}

std::string StorageTaiheError::ToMessage(const int errCode)
{
    int genericCode = ConvertUVCode2ErrCode(errCode);
    auto it = STORAGE_TAIHE_ERR_MAP.find(genericCode);
    if (it != STORAGE_TAIHE_ERR_MAP.end()) {
        return it->second.errorMsg;
    } else {
        return "Unknown error. Possible causes: 1.Insufficient memory."
            "2.Memory operation error. 3.Null pointer. 4.Failed to obtain expected resources.";
    }
}

void StorageTaiheError::SetStorageTaiheError(const int error_code) {
    taihe::set_business_error(
        Convert(OHOS::StorageManager::Convert2JsErrNum(error_code)),
        ToMessage(OHOS::StorageManager::Convert2JsErrNum(error_code)));
}
}
