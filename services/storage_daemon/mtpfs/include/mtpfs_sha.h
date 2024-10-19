/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef MTPFS_SHA_H
#define MTPFS_SHA_H

#include <string>

class MtpFsSha {
public:
    MtpFsSha();
    void Update(const std::string &s);
    void Update(std::istream &is);
    std::string Final();

    static std::string SumString(const std::string &str);

private:
    typedef unsigned long int uint32;  /* just needs to be at least 32bit */
    typedef unsigned long long uint64; /* just needs to be at least 64bit */

    static const unsigned int DIGEST_INTS = 5; /* number of 32bit integers per MtpFsSha digest */
    static const unsigned int BLOCK_INTS = 16; /* number of 32bit integers per MtpFsSha block */
    static const unsigned int BLOCK_BYTES = BLOCK_INTS * 4;

    uint64 digest_[DIGEST_INTS];
    std::string buffer_;
    uint64 transforms_;

    void Reset();
    void Transform(uint64 block[BLOCK_BYTES]);

    static void BufferToBlock(const std::string &buffer, uint64 block[BLOCK_BYTES]);
    static void Read(std::istream &is, std::string &s, const int max);
};

#endif
