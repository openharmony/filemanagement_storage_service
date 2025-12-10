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

#ifndef FILE_SYSTEM_CRYPT_ENABLE_CONFIG_WITH_OEMINFO
#define FILE_SYSTEM_CRYPT_ENABLE_CONFIG_WITH_OEMINFO
#ifdef __cplusplus
extern "C" {
#endif
enum FS_CRYPT_ENABLE_STATUS { UNDEFINED = -1, DISABLE = 0, ENABLE = 1 };
/**
 * @brief Checks whether the file system encryption (FS Crypt) is enabled
 *        based on the OEM information read from the factory interface.
 *
 * @details This function reads the encryption status from the OEMInfo database
 *          via the factory interface service. The result is cached in a static
 *          variable to avoid repeated reads.
 *
 * @return int One of the following values:
 *         - @b -1: The status is undefined or couldn't be determined.
 *         - @b  0: File system encryption is disabled.
 *         - @b  1: File system encryption is enabled.
 *
 * @see enum FS_CRYPT_ENABLE_STATUS for status definitions.
 */
int IsFsCryptEnableByOemInfo();
#ifdef __cplusplus
}
#endif

#endif
