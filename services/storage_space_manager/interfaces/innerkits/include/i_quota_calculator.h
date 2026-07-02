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

#pragma once

#include <string>
#include <map>
#include <vector>
#include <unordered_map>

namespace OHOS {
namespace StorageSpaceManager {

enum class CacheAutoCleanSwitch : int32_t {
    CLOSE = 0,
    OPEN = 1
};

class IQuotaCalculator {
public:
    virtual ~IQuotaCalculator() = default;
    virtual void Init() = 0;
    virtual int32_t ParseConfig(const std::string &configPath) = 0;
    virtual CacheAutoCleanSwitch GetCacheAutoCleanSwitch() const = 0;
    virtual int32_t GetAutoCacheCleanSpan() const = 0;
    virtual std::unordered_map<std::string, int32_t> GetSystemAppCacheSize() const = 0;
    virtual int32_t GetTopRankingHoursSpan() const = 0;
    virtual int32_t GetNoUseHoursForCleanAll() const = 0;
    virtual int32_t GetTopAppCount(int64_t totalSize, int32_t &topCount) const = 0;
    virtual int32_t GetQuotaByRank(int32_t appRank, int64_t totalStorage, int32_t &quota) = 0;
    virtual int32_t GetQuotasByRanks(const std::vector<int32_t> &appRanks,
                                     int64_t totalStorage,
                                     std::unordered_map<int32_t, int32_t> &quotas) = 0;
    virtual int32_t GetQuota(const std::string &tierName, int64_t totalStorage, int32_t &quota) = 0;
    virtual int32_t GetAllQuotas(int64_t totalStorage, std::map<std::string, int32_t> &quotas) = 0;
    virtual bool IsConfigLoaded() const = 0;
};

using CreateQuotaCalculatorFuncPtr = IQuotaCalculator *(*)(void);

} // namespace StorageSpaceManager
} // namespace OHOS