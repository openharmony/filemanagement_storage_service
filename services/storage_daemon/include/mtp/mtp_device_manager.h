/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef MTP_STATS_H
#define MTP_STATS_H

#include "parcel.h"

namespace OHOS {
namespace StorageManager {
class MtpStats  final : public Parcelable {
public:
    MtpStats();
    MtpStats(std::string id, std::string path);
    
    std::string GetId();
    std::string GetPath();
  
    bool Marshalling(Parcel &parcel) const override;
    static std::unique_ptr<MtpStats> Unmarshalling(Parcel &parcel);

private:
    std::string id_;
    std::string path_;
    std::string devlinks_;
};
} // namespace StorageManager
} // namespace OHOS

#endif
\ No newline at end of file
 +37 services/storage_daemon/include/mtp/mtp_device_manager.h  0 -> 100644
/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef MTP_DEVICE_MANAGER_H
#define MTP_DEVICE_MANAGER_H
#include <string>
#include "singleton.h"
#include <sys/types.h>
namespace OHOS {
namespace StorageDaemon {
class MtpDeviceManager : public Singleton<MtpDeviceManager> {
public:
    bool MountMtp(std::string id, std::string devlinks, std::string path);
    bool UnMountMtp(std::string id, std::string path);
    bool UnMountMtpAll(std::string path);

private:
    bool isMounting = false;
    bool Exec(const std::string &devlinks, const std::string &path);
    uid_t FILE_MANAGER_UID = 1006;
    gid_t FILE_MANAGER_GID = 1006;
    mode_t PUBLIC_DIR_MODE = 0770;
};
} // namespace StorageDaemon
} // namespace OHOS
#endif