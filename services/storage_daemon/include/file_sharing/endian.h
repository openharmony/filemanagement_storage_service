/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef OHOS_STORAGE_DAEMON_ENDIAN_H
#define OHOS_STORAGE_DAEMON_ENDIAN_H

inline bool IsLe()
{
    int tmp = 0x01;
    return *(char *)&tmp;
}

inline bool IsBe()
{
    int tmp = 0x01;
    return *((char *)&tmp + sizeof(tmp) - 1);
}

/* We haven't meet any BE machine, so leave it until we really need it. */
template <typename T>
inline T LeToCpu(T x)
{
    return x;
}

template <typename T>
inline T CpuToLe(T x)
{
    return x;
}

#endif // OHOS_STORAGE_DAEMON_ENDIAN_H
