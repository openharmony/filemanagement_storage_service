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

#include "disk_manager/volume/volume_operator_factory.h"
#include "disk_manager/volume/vfat_operator.h"
#include "disk_manager/volume/ntfs_operator.h"
#include "disk_manager/volume/exfat_operator.h"
#include "storage_service_log.h"

#include <mutex>

namespace OHOS {
namespace StorageDaemon {

static std::map<std::string, std::shared_ptr<IVolumeOperator>> g_operators;
static std::once_flag g_initFlag;

void VolumeOperatorFactory::RegisterOperators(
    std::map<std::string, std::shared_ptr<IVolumeOperator>>& operators)
{
    LOGI("VolumeOperatorFactory::RegisterOperators start");

    operators["vfat"] = std::make_shared<VfatOperator>();
    operators["ntfs"] = std::make_shared<NtfsOperator>();
    operators["exfat"] = std::make_shared<ExfatOperator>();

    LOGI("VolumeOperatorFactory::RegisterOperators registered %{public}zu operators",
         operators.size());
}

static void EnsureOperatorsInit()
{
    std::call_once(g_initFlag, [&]() { VolumeOperatorFactory::RegisterOperators(g_operators); });
}

std::shared_ptr<IVolumeOperator> VolumeOperatorFactory::CreateOperator(const std::string& fsType)
{
    LOGI("VolumeOperatorFactory::CreateOperator fsType=%{public}s", fsType.c_str());
    EnsureOperatorsInit();

    auto it = g_operators.find(fsType);
    if (it != g_operators.end()) {
        LOGI("VolumeOperatorFactory::CreateOperator found operator for %{public}s",
             fsType.c_str());
        return it->second;
    }

    LOGW("VolumeOperatorFactory::CreateOperator no operator found for %{public}s",
         fsType.c_str());
    return nullptr;
}

std::shared_ptr<IVolumeOperator> VolumeOperatorFactory::GetGenericOperator()
{
    LOGI("VolumeOperatorFactory::GetGenericOperator");
    EnsureOperatorsInit();

    auto it = g_operators.find("vfat");
    if (it != g_operators.end()) {
        return it->second;
    }

    LOGE("VolumeOperatorFactory::GetGenericOperator no vfat operator registered");
    return nullptr;
}

} // namespace StorageDaemon
} // namespace OHOS
