/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_STORAGE_MANAGER_STATISTIC_INFO_H
#define OHOS_STORAGE_MANAGER_STATISTIC_INFO_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class NextDqBlk : public Parcelable {
public:
    NextDqBlk() = default;

    NextDqBlk(uint64_t hardLimit,
              uint64_t bSoftLimit,
              uint64_t curSpace,
              uint64_t iHardLimit,
              uint64_t iSoftLimit,
              uint64_t curInodes,
              uint64_t bTime,
              uint64_t iTime,
              uint32_t valid,
              uint32_t id)
        : dqbHardLimit(hardLimit),
          dqbBSoftLimit(bSoftLimit),
          dqbCurSpace(curSpace),
          dqbIHardLimit(iHardLimit),
          dqbISoftLimit(iSoftLimit),
          dqbCurInodes(curInodes),
          dqbBTime(bTime),
          dqbITime(iTime),
          dqbValid(valid),
          dqbId(id)
    {
    }

    /* Absolute limit on disk quota blocks alloc */
    uint64_t dqbHardLimit = 0;
    /* Preferred limit on disk quota blocks */
    uint64_t dqbBSoftLimit = 0;
    /* Current occupied space(in bytes) */
    uint64_t dqbCurSpace = 0;
    /* Maximum number of allocated inodes */
    uint64_t dqbIHardLimit = 0;
    /* Preferred inode limit */
    uint64_t dqbISoftLimit = 0;
    /* Current number of allocated inodes */
    uint64_t dqbCurInodes = 0;
    /* Time limit for excessive disk use */
    uint64_t dqbBTime = 0;
    /* Time limit for excessive files */
    uint64_t dqbITime = 0;
    /* Bit mask of QIF_* constants */
    uint32_t dqbValid = 0;
    /* the next ID greater than or equal to id that has a quota set */
    uint32_t dqbId = 0;

    bool Marshalling(Parcel &parcel) const override;
    static NextDqBlk *Unmarshalling(Parcel &parcel);
};

class DirSpaceInfo : public Parcelable {
public:
    DirSpaceInfo() = default;

    DirSpaceInfo(const std::string &p, uint32_t u, int64_t s)
        : path(p), uid(u), size(s) {}

    std::string path = "";
    uint32_t uid = 0;
    int64_t size = 0;

    bool Marshalling(Parcel &parcel) const override;
    static DirSpaceInfo *Unmarshalling(Parcel &parcel);
};
} // namespace StorageManager
} // namespace OHOS
#endif // OHOS_STORAGE_MANAGER_STATISTIC_INFO_H
