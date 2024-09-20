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
        uint32 block[BLOCK_INTS];
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

    uint32 block[BLOCK_INTS];
    BufferToBlock(buffer_, block);

    if (origSize > BLOCK_BYTES - BLOCK_BYTES_EIGHT) {
        Transform(block);
        for (unsigned int i = 0; i < BLOCK_INTS - BLOCK_INTS_TWO; ++i) {
            block[i] = 0;
        }
    }

    block[BLOCK_INTS - 1] = totalBits;
    block[BLOCK_INTS - BLOCK_INTS_TWO] = (totalBits >> BIT_OFFSET);
    Transform(block);

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

void MtpFsSha::Transform(uint32 block[BLOCK_BYTES])
{
    uint32 a = digest_[0];
    uint32 b = digest_[1];
    uint32 c = digest_[DIGEST_TWO];
    uint32 d = digest_[DIGEST_THREE];
    uint32 e = digest_[DIGEST_FOUR];

#define rol(value, bits) (((value) << (bits)) | (((value)&0xffffffff) >> (BIT_OFFSET - (bits))))
#define blk(i) \
    (block[i & 15] = rol(block[(i + 13) & 15] ^ block[(i + 8) & 15] ^ block[(i + 2) & 15] ^ block[i & 15], 1))

#define R0(v, w, x, y, z, i)                                      \
    z += ((w & (x ^ y)) ^ y) + block[i] + 0x5a827999 + rol(v, 5); \
    w = rol(w, 30);
#define R1(v, w, x, y, z, i)                                    \
    z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5a827999 + rol(v, 5); \
    w = rol(w, 30);
#define R2(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0x6ed9eba1 + rol(v, 5); \
    w = rol(w, 30);
#define R3(v, w, x, y, z, i)                                          \
    z += (((w | x) & y) | (w & x)) + blk(i) + 0x8f1bbcdc + rol(v, 5); \
    w = rol(w, 30);
#define R4(v, w, x, y, z, i)                            \
    z += (w ^ x ^ y) + blk(i) + 0xca62c1d6 + rol(v, 5); \
    w = rol(w, 30);

    R0(a, b, c, d, e, 0);
    R0(e, a, b, c, d, 1);
    R0(d, e, a, b, c, 2);
    R0(c, d, e, a, b, 3);
    R0(b, c, d, e, a, 4);
    R0(a, b, c, d, e, 5);
    R0(e, a, b, c, d, 6);
    R0(d, e, a, b, c, 7);
    R0(c, d, e, a, b, 8);
    R0(b, c, d, e, a, 9);
    R0(a, b, c, d, e, 10);
    R0(e, a, b, c, d, 11);
    R0(d, e, a, b, c, 12);
    R0(c, d, e, a, b, 13);
    R0(b, c, d, e, a, 14);
    R0(a, b, c, d, e, 15);
    R1(e, a, b, c, d, 16);
    R1(d, e, a, b, c, 17);
    R1(c, d, e, a, b, 18);
    R1(b, c, d, e, a, 19);
    R2(a, b, c, d, e, 20);
    R2(e, a, b, c, d, 21);
    R2(d, e, a, b, c, 22);
    R2(c, d, e, a, b, 23);
    R2(b, c, d, e, a, 24);
    R2(a, b, c, d, e, 25);
    R2(e, a, b, c, d, 26);
    R2(d, e, a, b, c, 27);
    R2(c, d, e, a, b, 28);
    R2(b, c, d, e, a, 29);
    R2(a, b, c, d, e, 30);
    R2(e, a, b, c, d, 31);
    R2(d, e, a, b, c, 32);
    R2(c, d, e, a, b, 33);
    R2(b, c, d, e, a, 34);
    R2(a, b, c, d, e, 35);
    R2(e, a, b, c, d, 36);
    R2(d, e, a, b, c, 37);
    R2(c, d, e, a, b, 38);
    R2(b, c, d, e, a, 39);
    R3(a, b, c, d, e, 40);
    R3(e, a, b, c, d, 41);
    R3(d, e, a, b, c, 42);
    R3(c, d, e, a, b, 43);
    R3(b, c, d, e, a, 44);
    R3(a, b, c, d, e, 45);
    R3(e, a, b, c, d, 46);
    R3(d, e, a, b, c, 47);
    R3(c, d, e, a, b, 48);
    R3(b, c, d, e, a, 49);
    R3(a, b, c, d, e, 50);
    R3(e, a, b, c, d, 51);
    R3(d, e, a, b, c, 52);
    R3(c, d, e, a, b, 53);
    R3(b, c, d, e, a, 54);
    R3(a, b, c, d, e, 55);
    R3(e, a, b, c, d, 56);
    R3(d, e, a, b, c, 57);
    R3(c, d, e, a, b, 58);
    R3(b, c, d, e, a, 59);
    R4(a, b, c, d, e, 60);
    R4(e, a, b, c, d, 61);
    R4(d, e, a, b, c, 62);
    R4(c, d, e, a, b, 63);
    R4(b, c, d, e, a, 64);
    R4(a, b, c, d, e, 65);
    R4(e, a, b, c, d, 66);
    R4(d, e, a, b, c, 67);
    R4(c, d, e, a, b, 68);
    R4(b, c, d, e, a, 69);
    R4(a, b, c, d, e, 70);
    R4(e, a, b, c, d, 71);
    R4(d, e, a, b, c, 72);
    R4(c, d, e, a, b, 73);
    R4(b, c, d, e, a, 74);
    R4(a, b, c, d, e, 75);
    R4(e, a, b, c, d, 76);
    R4(d, e, a, b, c, 77);
    R4(c, d, e, a, b, 78);
    R4(b, c, d, e, a, 79);

    digest_[0] += a;
    digest_[1] += b;
    digest_[DIGEST_TWO] += c;
    digest_[DIGEST_THREE] += d;
    digest_[DIGEST_FOUR] += e;

    transforms_++;
}

void MtpFsSha::BufferToBlock(const std::string &buffer, uint32 block[BLOCK_BYTES])
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
