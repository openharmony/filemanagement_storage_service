/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "init_utils.h"

#include <string.h>

#include "fscrypt_log.h"
#include "securec.h"

#define MAX_FILE_LEN 102400    // max init.cfg size 100KB

char *ReadFileToBuf(const char *configFile)
{
    char *buffer = NULL;
    FILE *fd = NULL;
    struct stat fileStat = {0};
    FSCRYPT_CHECK_RETURN_VALUE(configFile != NULL && *configFile != '\0', NULL);
    do {
        if (stat(configFile, &fileStat) != 0 ||
            fileStat.st_size <= 0 || fileStat.st_size > MAX_FILE_LEN) {
            LOGE("Unexpected config file \" %{public}s \", check if it exist. if exist, check file size", configFile);
            break;
        }
        fd = fopen(configFile, "r");
        if (fd == NULL) {
            LOGE("Open %{public}s failed. err = %{public}d", configFile, errno);
            break;
        }
        buffer = (char*)malloc((size_t)(fileStat.st_size + 1));
        if (buffer == NULL) {
            LOGE("Failed to allocate memory for config file, err = %{public}d", errno);
            break;
        }

        if (fread(buffer, fileStat.st_size, 1, fd) != 1) {
            free(buffer);
            buffer = NULL;
            LOGE("Failed to read config file, err = %{public}d", errno);
            break;
        }
        buffer[fileStat.st_size] = '\0';
    } while (0);

    if (fd != NULL) {
        (void)fclose(fd);
        fd = NULL;
    }
    return buffer;
}

int SplitString(char *srcPtr, const char *del, char **dstPtr, int maxNum)
{
    FSCRYPT_CHECK_RETURN_VALUE(srcPtr != NULL && dstPtr != NULL && del != NULL, -1);
    char *buf = NULL;
    dstPtr[0] = strtok_r(srcPtr, del, &buf);
    int counter = 0;
    while ((counter < maxNum) && (dstPtr[counter] != NULL)) {
        counter++;
        if (counter >= maxNum) {
            break;
        }
        dstPtr[counter] = strtok_r(NULL, del, &buf);
    }
    return counter;
}

void FreeStringVector(char **vector, int count)
{
    if (vector != NULL) {
        for (int i = 0; i < count; i++) {
            if (vector[i] != NULL) {
                free(vector[i]);
                vector[i] = NULL;
            }
        }
        free(vector);
        vector = NULL;
    }
}

char **SplitStringExt(char *buffer, const char *del, int *returnCount, int maxItemCount)
{
    FSCRYPT_CHECK_RETURN_VALUE((maxItemCount >= 0) && (buffer != NULL) && (del != NULL) && (returnCount != NULL), NULL);
    // Why is this number?
    // Now we use this function to split a string with a given delimiter
    // We do not know how many sub-strings out there after splitting.
    // 50 is just a guess value.
    const int defaultItemCounts = 50;
    int itemCounts = maxItemCount;

    if (maxItemCount > defaultItemCounts) {
        itemCounts = defaultItemCounts;
    }
    char **items = (char **)malloc(sizeof(char*) * itemCounts);
    FSCRYPT_ERROR_CHECK(items != NULL, free(items);
        return NULL, "No enough memory to store items");
    char *rest = NULL;
    char *p = strtok_r(buffer, del, &rest);
    int count = 0;
    while (p != NULL) {
        if (count > itemCounts - 1) {
            itemCounts += (itemCounts / 2) + 1; // 2 Request to increase the original memory by half.
            LOGD("Too many items,expand size");

            char **expand = (char **)malloc(sizeof(char*) * itemCounts);
            FSCRYPT_ERROR_CHECK(expand != NULL, FreeStringVector(items, count);
                return NULL, "Failed to expand memory");
            int ret = memcpy_s(expand, sizeof(char *) * itemCounts, items, sizeof(char *) * count);
            if (ret != 0) {
                FreeStringVector(items, count);
                FreeStringVector(expand, itemCounts);
                LOGD("Too many items,expand size");
                return NULL;
            }
            free(items);
            items = expand;
        }
        size_t len = strlen(p);
        items[count] = (char *)malloc(len + 1);
        FSCRYPT_CHECK(items[count] != NULL, FreeStringVector(items, count);
            return NULL);
        if (strncpy_s(items[count], len + 1, p, len) != EOK) {
            LOGE("Copy string failed");
            FreeStringVector(items, count);
            return NULL;
        }
        items[count][len] = '\0';
        count++;
        p = strtok_r(NULL, del, &rest);
    }
    *returnCount = count;
    return items;
}