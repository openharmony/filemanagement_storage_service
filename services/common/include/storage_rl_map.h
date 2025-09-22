/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#ifndef STORAGE_RL_MAP_H
#define STORAGE_RL_MAP_H

#include <map>
#include <nocopyable.h>
#include <unordered_map>
#include "rwlock.h"

namespace OHOS {
namespace StorageService {
template <typename K, typename V>
class StorageRlMap : public NoCopyable {
public:
    StorageRlMap() {}
    ~StorageRlMap() {}
    V ReadVal(const K& key)
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_[key];
    }
    void Erase(const K& key)
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        map_.erase(key);
    }
    bool Insert(const K& key, const V& value)
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        auto result = map_.insert(std::pair<K, V>(key, value));
        return result.second;
    }
    void Clear()
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        map_.clear();
    }
    void Empty()
    {
        Utils::UniqueReadGuard<Utils::RWLock> volumeGuard(this->rl_);
        map_.empty();
    }
    bool Contains(const K& key)
    {
        Utils::UniqueReadGuard<Utils::RWLock> mapGuard(this->rl_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            return true;
        }
        return false;
    }
    V GetShared(const K& key)
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        auto it = map_.find(key);
        if (it != map_.end()) {
            return it->second;
        }
        return nullptr;
    }
    typename std::map<K, V>::iterator Find(const K& key)
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.find(key);
    }
    typename std::map<K, V>::const_iterator Find(const K& key) const
    {
        Utils::UniqueReadGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.find(key);
    }
    size_t Size()
    {
        Utils::UniqueReadGuard<Utils::RWLock> volumeGuard(this->rl_);
        return map_.size();
    }
    typename std::map<K, V>::iterator Begin()
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.begin();
    }
    typename std::map<K, V>::const_iterator Begin() const
    {
        Utils::UniqueReadGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.begin();
    }
    typename std::map<K, V>::iterator End()
    {
        Utils::UniqueWriteGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.end();
    }
    typename std::map<K, V>::const_iterator End() const
    {
        Utils::UniqueReadGuard<Utils::RWLock> mapGuard(this->rl_);
        return map_.end();
    }

private:
    OHOS::Utils::RWLock rl_;
    std::map<K, V> map_;
};
}
}
#endif // STORAGE_RL_MAP_H