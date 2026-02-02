/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include <iomanip>
#include <openssl/sha.h>
#include <sstream>
#include <scsi/sg.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>

#include "securec.h"
#include "storage_service_errno.h"
#include "storage_service_log.h"
#include "utils/disk_utils.h"
#include "utils/file_utils.h"
#include "utils/storage_radar.h"

namespace OHOS {
namespace StorageDaemon {
using namespace std;
using namespace OHOS::StorageService;
constexpr int32_t NODE_PERM = 0660;
constexpr int32_t MIN_UUID_LENGTH = 1;
constexpr int32_t MAX_UUID_LENGTH = 40;
constexpr int32_t DEF_TIMEOUT = 120000;
constexpr int32_t SENSE_BUFF_LEN = 64;
constexpr int32_t READ_DISC_INFO_OPCODE = 0x51;
constexpr int32_t READ_DISC_INFO_SECTOR = 0x46;
constexpr uint8_t DISC_TYPE_OFFSET_HIGH = 6;
constexpr uint8_t DISC_TYPE_OFFSET_LOW = 7;
constexpr uint8_t DISC_TYPE_OFFSET = 8;
constexpr int32_t CDB_ALLOCATION_LENGTH_HIGH = 7;
constexpr int32_t CDB_ALLOCATION_LENGTH_LOW = 8;
constexpr int32_t MAX_ALLOC_LEN = 0xFFFF;
constexpr int32_t READ_DISC_INFO_CDB_LEN = 10;
constexpr const char *MMC_MAX_VOLUMES_PATH = "/sys/module/mmcblk/parameters/perdev_minors";
constexpr size_t INT32_SHORT_ID_LENGTH = 20;
constexpr size_t INT32_PLAINTEXT_LENGTH = 4;
constexpr size_t INT32_MIN_ID_LENGTH = 3;
constexpr size_t SHA256_DIGEST_BIT_MASK = 0x0f;
constexpr size_t SHA256_DIGEST_VERSION = 0x50;
constexpr size_t SHA256_VARIANT_MASK = 0x3f;
constexpr size_t SHA256_IETF_VARIANT = 0x80;
constexpr uint8_t UUID_NAMESPACE_RAW_SIZE = 32;
constexpr uint8_t UUID_DIGEST_BYTE_OFFSET = 6;
constexpr uint8_t UUID_VARIANT_BYTE_OFFSET = 8;
constexpr uint8_t UUID_TIME_LO_FIELD_WIDTH = 8;
constexpr uint8_t UUID_TIME_MID_FIELD_WIDTH = 4;
constexpr uint8_t UUID_TIME_HI_VERSION_FIELD_WIDTH = 4;
constexpr uint8_t UUID_CLOCK_SEQ_FIELD_WIDTH = 4;
constexpr uint8_t UUID_NODE_ID_FIELD_WIDTH = 12;
constexpr uint8_t UUID_DIGEST_TIME_MID_OFFSET = 4;
constexpr uint8_t UUID_DIGEST_TIME_HI_VERSION_OFFSET = 6;
constexpr uint8_t UUID_DIGEST_CLOCK_SEQ_OFFSET = 8;
constexpr uint8_t UUID_DIGEST_NODE_ID_OFFSET = 10;

int CreateDiskNode(const std::string &path, dev_t dev)
{
    if (mknod(path.c_str(), NODE_PERM | S_IFBLK, dev) < 0) {
        LOGE("create disk node failed");
        return E_ERR;
    }
    return E_OK;
}

int DestroyDiskNode(const std::string &path)
{
    if (TEMP_FAILURE_RETRY(unlink(path.c_str())) < 0) {
        return E_ERR;
    }
    return E_OK;
}

int GetDevSize(const std::string &path, uint64_t *size)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f == nullptr) {
        LOGE("open %{private}s failed", path.c_str());
        return E_ERR;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("open %{private}s failed", path.c_str());
        (void)fclose(f);
        return E_ERR;
    }

    if (ioctl(fd, BLKGETSIZE64, size)) {
        LOGE("get device %{private}s size failed", path.c_str());
        (void)fclose(f);
        return E_ERR;
    }

    (void)fclose(f);
    return E_OK;
}

int GetMaxVolume(dev_t device)
{
    unsigned int majorId = major(device);
    if (majorId == DISK_MMC_MAJOR) {
        std::string str;
        if (!ReadFile(MMC_MAX_VOLUMES_PATH, &str)) {
            LOGE("Get MmcMaxVolumes failed");
            return E_ERR;
        }
        return std::atoi(str.c_str());
    } else {
        return MAX_SCSI_VOLUMES;
    }
}

static bool IsAcceptableUuid(const std::string &uuid)
{
    if (uuid.empty()) {
        return false;
    }
    
    static const std::string forbidden = "/\\";
    if (uuid.find_first_of(forbidden) != std::string::npos) {
        return false;
    }

    if (uuid.length() < MIN_UUID_LENGTH || uuid.length() > MAX_UUID_LENGTH) {
        return false;
    }

    if (uuid == "." || uuid == "..") {
        return false;
    }

    return true;
}

int32_t ReadMetadata(const std::string &devPath, std::string &uuid, std::string &type, std::string &label)
{
    uuid = GetBlkidData(devPath, "UUID");
    type = GetBlkidData(devPath, "TYPE");
    label = GetBlkidData(devPath, "LABEL");
    LOGI("ReadMetadata, fsUuid=%{public}s, fsType=%{public}s, fsLabel=%{public}s.", GetAnonyString(uuid).c_str(),
        type.c_str(), label.c_str());
    if (type.empty() || !IsAcceptableUuid(uuid)) {
        LOGE("External volume ReadMetadata error.");
        return E_READMETADATA;
    }
    return E_OK;
}

int32_t ReadVolumeUuid(const std::string &devPath, std::string &uuid)
{
    uuid = GetBlkidData(devPath, "UUID");
    LOGI("ReadMetadata, fsUuid=%{public}s", GetAnonyString(uuid).c_str());
    if (uuid.empty()) {
        LOGE("External volume ReadMetadata error.");
        return E_READMETADATA;
    }
    return E_OK;
}

std::string GetBlkidData(const std::string &devPath, const std::string &type)
{
    std::vector<std::string> cmd;
    cmd = {
        "blkid",
        "-n",
        "mdraid",
        "-s",
        type,
        "-o",
        "value",
        devPath
    };
    return GetBlkidDataByCmd(cmd);
}

std::string GetBlkidDataByCmd(std::vector<std::string> &cmd)
{
    std::vector<std::string> output;

    int32_t err = ForkExec(cmd, &output);
    if (err) {
        StorageRadar::ReportVolumeOperation("ForkExec", err);
        return "";
    }

    if (output.size() > 0) {
        size_t sep = output[0].find_first_of("\n");
        if (sep != string::npos)
            output[0].resize(sep);
        return output[0];
    }
    return "";
}

std::string GenerateRandomUuid(const std::string &diskPath, const std::string &uuidFormat)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctxSeed;
    SHA256_Init(&ctxSeed);
    SHA256_Update(&ctxSeed, uuidFormat.c_str(), uuidFormat.length());
    SHA256_Final(hash, &ctxSeed);
 
    unsigned char namespaceRaw[UUID_NAMESPACE_RAW_SIZE];
    std::copy(hash, hash + UUID_NAMESPACE_RAW_SIZE, namespaceRaw);
 
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, namespaceRaw, sizeof(namespaceRaw));
    SHA256_Update(&ctx, diskPath.c_str(), diskPath.length());
    SHA256_Final(digest, &ctx);
 
    digest[UUID_DIGEST_BYTE_OFFSET] &= SHA256_DIGEST_BIT_MASK;
    digest[UUID_DIGEST_BYTE_OFFSET] |= SHA256_DIGEST_VERSION;
    digest[UUID_VARIANT_BYTE_OFFSET] &= SHA256_VARIANT_MASK;
    digest[UUID_VARIANT_BYTE_OFFSET] |= SHA256_IETF_VARIANT;
 
    std::ostringstream uuidStream;
    uuidStream << std::hex << std::setfill('0') << std::uppercase
        << std::setw(UUID_TIME_LO_FIELD_WIDTH) << std::hex << *reinterpret_cast<uint32_t*>(digest) << '-'
        << std::setw(UUID_TIME_MID_FIELD_WIDTH) << *reinterpret_cast<uint16_t*>(digest +
        UUID_DIGEST_TIME_MID_OFFSET) << '-'
        << std::setw(UUID_TIME_HI_VERSION_FIELD_WIDTH) << *reinterpret_cast<uint16_t*>(digest +
        UUID_DIGEST_TIME_HI_VERSION_OFFSET) << '-'
        << std::setw(UUID_CLOCK_SEQ_FIELD_WIDTH) << *reinterpret_cast<uint16_t*>(digest +
        UUID_DIGEST_CLOCK_SEQ_OFFSET) << '-'
        << std::setw(UUID_NODE_ID_FIELD_WIDTH) << *reinterpret_cast<uint64_t*>(digest +
        UUID_DIGEST_NODE_ID_OFFSET);
 
    return uuidStream.str();
}

std::string GetAnonyString(const std::string &value)
{
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < INT32_MIN_ID_LENGTH) {
        return tmpStr;
    }

    if (strLen <= INT32_SHORT_ID_LENGTH) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, INT32_PLAINTEXT_LENGTH);
        res += tmpStr;
        res.append(value, strLen - INT32_PLAINTEXT_LENGTH, INT32_PLAINTEXT_LENGTH);
    }

    return res;
}

int SendScsiCmd(int fd, uint8_t *cdb, int cdbLen, uint8_t *dxferp, int dxferLen)
{
    sg_io_hdr_t ioHdr;
    uint8_t sense[SENSE_BUFF_LEN];
    if (memset_s(&ioHdr, sizeof(ioHdr), 0, sizeof(ioHdr)) != 0) {
        LOGE("ioHdr memset_s used failed.");
        return E_ERR;
    }
    if (memset_s(sense, sizeof(sense), 0, sizeof(sense)) != 0) {
        LOGE("sense memset_s used failed.");
        return E_ERR;
    }
    ioHdr.interface_id = 'S';
    ioHdr.dxfer_direction = SG_DXFER_FROM_DEV;
    ioHdr.cmdp = cdb;
    ioHdr.cmd_len = cdbLen;
    ioHdr.dxferp = dxferp;
    ioHdr.dxfer_len = static_cast<unsigned int>(dxferLen);
    ioHdr.mx_sb_len = sizeof(sense);
    ioHdr.sbp = sense;
    ioHdr.timeout = DEF_TIMEOUT;
    if (ioctl(fd, SG_IO, &ioHdr) < 0) {
        return E_ERR;
    }
    if ((ioHdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
        return E_ERR;
    }
    return E_OK;
}

int ReadDiscInfo(const std::string &diskPath, int32_t cmdIndex, uint8_t *buf, int len)
{
    char realPath[PATH_MAX] = { 0 };
    if (realpath(diskPath.c_str(), realPath) == nullptr) {
        LOGE("realpath faild.");
        return E_ERR;
    }
    FILE* file = fopen(diskPath.c_str(), "rb");
    if (file == nullptr) {
        LOGE("fopen errno: %{public}d", errno);
        return E_ERR;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("fileno error: %{public}d", errno);
        (void)fclose(file);
        return E_ERR;
    }
    uint8_t cdb[READ_DISC_INFO_CDB_LEN] = { cmdIndex };
    cdb[CDB_ALLOCATION_LENGTH_HIGH] = static_cast<uint8_t>(static_cast<uint32_t>(len) >> CDB_ALLOCATION_LENGTH_LOW);
    cdb[CDB_ALLOCATION_LENGTH_LOW] = static_cast<uint8_t>(static_cast<uint32_t>(len) & MAX_ALLOC_LEN);

    int ret = SendScsiCmd(fd, cdb, sizeof(cdb), buf, len);
    if (ret != 0) {
        LOGE("SendScsiCmd faild: %{public}d", ret);
        (void)fclose(file);
        return ret;
    }
    (void)fclose(file);
    return E_OK;
}

void IsExistCD(const std::string &diskPath, bool &isExistCD)
{
    std::vector<std::string> cmd;
    cmd = {
        "dd",
        "if=" + diskPath,
        "of=/dev/null",
        "bs=2048",
        "count=1",
        "status=none"
    };
    std::vector<std::string> output;
    int32_t err = ForkExec(cmd, &output);
    if (err != 0 || !output.empty()) {
        isExistCD = false;
        return;
    }
    isExistCD = true;
    return;
}

int IsBlankCD(const std::string &diskPath, bool &isBlankCD)
{
    uint8_t buf[MAX_BUF];
    if (ReadDiscInfo(diskPath, READ_DISC_INFO_OPCODE, buf, sizeof(buf)) == 0) {
        uint8_t discStatus = buf[DISC_STATUS_BYTE_INDEX] & DISC_STATUS_MASK;
        isBlankCD = (discStatus == 0);
        return E_OK;
    } else {
        LOGE("Unable to read disc information.");
    }
    return E_OK;
}

std::string DiskType2Str(uint8_t diskType)
{
    switch (diskType) {
        case 0x08: // CD-ROM (read only)
            return "CD-ROM";
        case 0x09:
            return "CD-R";
        case 0x0A:
            return "CD-RW";
        case 0x10:
            return "DVD-ROM";
        case 0x11: // DVD-R sequential
            return "DVD-R";
        case 0x12:
            return "DVD-RAM";
        case 0x13: // DVD-RW restricted overwrite
        case 0x14: // DVD-RW sequential
            return "DVD-RW";
        case 0x1A:
            return "DVD+RW";
        case 0x1B: // DVD+R
        case 0x1C: // DVD+R dual layer
            return "DVD+R";
        case 0x1D: // DVD+RW dual layer
            return "DVD+RW";
        default:
            return "";
    }
}

std::string GetCDType(const std::string &diskPath)
{
    uint8_t buf[MAX_BUF];
    if (ReadDiscInfo(diskPath, READ_DISC_INFO_SECTOR, buf, sizeof(buf)) == 0) {
        return DiskType2Str((buf[DISC_TYPE_OFFSET_HIGH] << DISC_TYPE_OFFSET) | buf[DISC_TYPE_OFFSET_LOW]);
    } else {
        LOGE("Unable to read disc information.");
    }
    return "";
}

int Eject(const std::string &devPath)
{
    std::vector<std::string> output;
    std::vector<std::string> cmd = {
        "eject",
        "-s",
        devPath
    };
    int res = ForkExec(cmd, &output);
    if (res != E_OK) {
        LOGE("ForkExec eject failed, %{public}d", res);
        return res;
    }
    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
