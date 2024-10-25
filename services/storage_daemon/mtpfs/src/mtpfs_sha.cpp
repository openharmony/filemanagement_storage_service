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

#include "mtpfs_sha.h"

#include <iomanip>
#include <sstream>

#include "storage_service_log.h"

const int32_t DIGEST_TWO = 2;
const int32_t DIGEST_THREE = 3;
const int32_t DIGEST_FOUR = 4;
const int32_t BLOCK_BYTES_EIGHT = 8;
const int32_t BLOCK_INTS_TWO = 2;
const int32_t SETW_EIGHT = 8;
const int32_t BIT_OFFSET = 32;
const int32_t BUFFER_FACTOR = 4;
const int32_t OFFSET_EIGHT = 8;
const int32_t OFFSET_SIXTEEN = 16;
const int32_t OFFSET_TWENTYFOUR = 24;
const int32_t BUFFER_THREE = 3;
const int32_t BUFFER_TWO = 2;

MtpFsSha::MtpFsSha()
{
    Reset();
}

void MtpFsSha::Update(const std::string &s)
{
    std::istringstream is(s);
    Update(is);
}

void MtpFsSha::Update(std::istream &is)
{
    std::string restOfBuffer;
    Read(is, restOfBuffer, BLOCK_BYTES - buffer_.size());
    buffer_ += restOfBuffer;

    while (is) {
        uint64 block[BLOCK_INTS];
        BufferToBlock(buffer_, block);
        Transform(block);
        Read(is, buffer_, BLOCK_BYTES);
    }
}

std::string MtpFsSha::Final()
{
    uint64 totalBits = (transforms_ * BLOCK_BYTES + buffer_.size()) * 8;
    buffer_ += static_cast<char>(0x80);
    unsigned int origSize = buffer_.size();
    while (buffer_.size() < BLOCK_BYTES) {
        buffer_ += static_cast<char>(0x00);
    }

    uint64 block[BLOCK_INTS];
    BufferToBlock(buffer_, block);

    if (origSize > BLOCK_BYTES - BLOCK_BYTES_EIGHT) {
        LOGI("MtpFsSha: origSize oversize BLOCK_BYTES, Transform block");
        Transform(block);
        for (unsigned int i = 0; i < BLOCK_INTS - BLOCK_INTS_TWO; ++i) {
            block[i] = 0;
        }
    } else {
        block[BLOCK_INTS - 1] = totalBits;
        block[BLOCK_INTS - BLOCK_INTS_TWO] = (totalBits >> BIT_OFFSET);
        LOGI("MtpFsSha: Transform block");
        Transform(block);
    }

    std::ostringstream result;
    for (unsigned int i = 0; i < DIGEST_INTS; ++i) {
        result << std::hex << std::setfill('0') << std::setw(SETW_EIGHT);
        result << (digest_[i] & 0xffffffff);
    }

    Reset();
    return result.str();
}

void MtpFsSha::Reset()
{
    digest_[0] = 0x67452301;
    digest_[1] = 0xefcdab89;
    digest_[DIGEST_TWO] = 0x98badcfe;
    digest_[DIGEST_THREE] = 0x10325476;
    digest_[DIGEST_FOUR] = 0xc3d2e1f0;

    transforms_ = 0;
    buffer_ = "";
}

void MtpFsSha::Transform(uint64 block[BLOCK_BYTES])
{
    uint64 a = digest_[0];
    uint64 b = digest_[1];
    uint64 c = digest_[DIGEST_TWO];
    uint64 d = digest_[DIGEST_THREE];
    uint64 e = digest_[DIGEST_FOUR];

#define rol(value, bits) (((value) << (bits)) | (((value)&0xffffffff) >> (BIT_OFFSET - (bits))))
#define blk(i) \
    (block[(i) & 15] = rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i & 15], 1))

#define R0(v, w, x, y, z, i)                                      \
    z += (((w) & ((x) ^ (y))) ^ (y)) + block[i] + 0x5a827999 + rol(v, 5); \
    w = rol((w), 30)
#define R1(v, w, x, y, z, i)                                    \
    z += (((w) & ((x) ^ (y))) ^ (y)) + blk(i) + 0x5a827999 + rol(v, 5); \
    w = rol((w), 30)
#define R2(v, w, x, y, z, i)                            \
    z += ((w) ^ (x) ^ (y)) + blk(i) + 0x6ed9eba1 + rol(v, 5); \
    w = rol((w), 30)
#define R3(v, w, x, y, z, i)                                          \
    z += ((((w) | (x)) & (y)) | ((w) & (x))) + blk(i) + 0x8f1bbcdc + rol(v, 5); \
    w = rol((w), 30)
#define R4(v, w, x, y, z, i)                            \
    z += ((w) ^ (x) ^ (y)) + blk(i) + 0xca62c1d6 + rol(v, 5); \
    w = rol((w), 30)

    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);

    digest_[0] += a;
    digest_[1] += b;
    digest_[DIGEST_TWO] += c;
    digest_[DIGEST_THREE] += d;
    digest_[DIGEST_FOUR] += e;

    transforms_++;
}

void MtpFsSha::BufferToBlock(const std::string &buffer, uint64 block[BLOCK_BYTES])
{
    for (unsigned int i = 0; i < BLOCK_INTS; ++i) {
        block[i] = (buffer[BUFFER_FACTOR * i + BUFFER_THREE] & 0xff) |
            ((buffer[BUFFER_FACTOR * i + BUFFER_TWO] & 0xff) << OFFSET_EIGHT) |
            ((buffer[BUFFER_FACTOR * i + 1] & 0xff) << OFFSET_SIXTEEN) |
            ((buffer[BUFFER_FACTOR * i + 0] & 0xff) << OFFSET_TWENTYFOUR);
    }
}

void MtpFsSha::Read(std::istream &is, std::string &s, const int max)
{
    char sbuf[BLOCK_BYTES];
    is.read(sbuf, max);
    s.assign(sbuf, is.gcount());
}

std::string MtpFsSha::SumString(const std::string &str)
{
    MtpFsSha sha1;
    sha1.Update(str);
    return sha1.Final();
}
