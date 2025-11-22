/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILE_VOLUMEMANAGER_IMPL_H
#define OHOS_FILE_VOLUMEMANAGER_IMPL_H
#include "ohos.file.volumeManager.proj.hpp"
#include "ohos.file.volumeManager.impl.hpp"
#include "storage_manager_connect.h"
#include "storage_service_errno.h"
#include "taihe/runtime.hpp"

namespace ANI::VolumeManager {
ohos::file::volumeManager::Volume GetVolumeByUuidSync(taihe::string_view uuid);
taihe::array<ohos::file::volumeManager::Volume> GetAllVolumesSync();
} // namespace ANI::VolumeManager
#endif // OHOS_FILE_VOLUMEMANAGER_IMPL_H