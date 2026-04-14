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

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <getopt.h>
#include <iomanip>
#include <linux/cdrom.h>
#include <openssl/sha.h>
#include <sstream>
#include <cinttypes>
#include <scsi/sg.h>
#include <sys/ioctl.h>
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
constexpr uint8_t GET_CAPACITY_CMD_BUF_LEN = 16;
constexpr uint8_t GET_CAPACITY_DATA_BUF_LEN = 48;
constexpr uint8_t GET_CD_USED_CAPACITY_CMD_LEN = 10;
constexpr uint8_t GET_CD_USED_CAPACITY_DATA_LEN = 28;
constexpr uint8_t GET_CD_TOTAL_CAPACITY_CMD_LEN = 10;
constexpr uint8_t GET_CD_TOTAL_CAPACITY_DATA_LEN = 4;
constexpr uint8_t GET_DVD_USED_CAPACITY_CMD_LEN = 10;
constexpr uint8_t GET_DVD_USED_CAPACITY_DATA_LEN = 8;
constexpr uint8_t GET_DVD_TOTAL_CAPACITY_CMD_LEN = 12;
constexpr uint8_t GET_DVD_TOTAL_CAPACITY_DATA_LEN = 36;
constexpr uint16_t ODD_LOGICAL_SECTOR_SIZE = 2048;
constexpr uint8_t CD_SECTORS_PER_SECOND = 75;
constexpr uint8_t SECONDS_PER_MINUTES = 60;

int CreateDiskNode(const std::string &path, dev_t dev)
{
    LOGD("[L8:DiskUtils] CreateDiskNode: >>> ENTER <<< path=%{public}s", path.c_str());
    if (mknod(path.c_str(), NODE_PERM | S_IFBLK, dev) < 0) {
        LOGE("[L8:DiskUtils] CreateDiskNode: <<< EXIT FAILED <<< create disk node failed, errno=%{public}d", errno);
        return E_ERR;
    }
    LOGD("[L8:DiskUtils] CreateDiskNode: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int DestroyDiskNode(const std::string &path)
{
    LOGD("[L8:DiskUtils] DestroyDiskNode: >>> ENTER <<< path=%{public}s", path.c_str());
    if (TEMP_FAILURE_RETRY(unlink(path.c_str())) < 0) {
        LOGE("[L8:DiskUtils] DestroyDiskNode: <<< EXIT FAILED <<< unlink failed, errno=%{public}d", errno);
        return E_ERR;
    }
    LOGD("[L8:DiskUtils] DestroyDiskNode: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int GetDevSize(const std::string &path, uint64_t *size)
{
    LOGD("[L8:DiskUtils] GetDevSize: >>> ENTER <<< path=%{private}s", path.c_str());
    FILE *f = fopen(path.c_str(), "r");
    if (f == nullptr) {
        LOGE("[L8:DiskUtils] GetDevSize: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        return E_ERR;
    }
    int fd = fileno(f);
    if (fd < 0) {
        LOGE("[L8:DiskUtils] GetDevSize: <<< EXIT FAILED <<< open failed, errno=%{public}d", errno);
        (void)fclose(f);
        return E_ERR;
    }

    if (ioctl(fd, BLKGETSIZE64, size)) {
        LOGE("[L8:DiskUtils] GetDevSize: <<< EXIT FAILED <<< get device size failed, errno=%{public}d", errno);
        (void)fclose(f);
        return E_ERR;
    }

    (void)fclose(f);
    LOGD("[L8:DiskUtils] GetDevSize: <<< EXIT SUCCESS <<< size=%{public}" PRIu64, *size);
    return E_OK;
}

int GetMaxVolume(dev_t device)
{
    LOGD("[L8:DiskUtils] GetMaxVolume: >>> ENTER <<<");
    unsigned int majorId = major(device);
    if (majorId == DISK_MMC_MAJOR) {
        std::string str;
        if (!ReadFile(MMC_MAX_VOLUMES_PATH, &str)) {
            LOGE("[L8:DiskUtils] GetMaxVolume: <<< EXIT FAILED <<< Get MmcMaxVolumes failed");
            return E_ERR;
        }
        return std::atoi(str.c_str());
    } else {
        LOGD("[L8:DiskUtils] GetMaxVolume: <<< EXIT SUCCESS <<< MAX_SCSI_VOLUMES");
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
    LOGI("[L8:DiskUtils] ReadMetadata: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    uuid = GetBlkidData(devPath, "UUID");
    type = GetBlkidData(devPath, "TYPE");
    label = GetBlkidData(devPath, "LABEL");
    LOGI("[L8:DiskUtils] ReadMetadata: fsUuid=%{public}s, fsType=%{public}s, fsLabel=%{public}s",
        GetAnonyString(uuid).c_str(), type.c_str(), label.c_str());
    if (type.empty() || !IsAcceptableUuid(uuid)) {
        LOGE("[L8:DiskUtils] ReadMetadata: <<< EXIT FAILED <<< External volume ReadMetadata error");
        return E_READMETADATA;
    }
    LOGI("[L8:DiskUtils] ReadMetadata: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int32_t ReadVolumeUuid(const std::string &devPath, std::string &uuid)
{
    LOGI("[L8:DiskUtils] ReadVolumeUuid: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    uuid = GetBlkidData(devPath, "UUID");
    LOGI("[L8:DiskUtils] ReadVolumeUuid: fsUuid=%{public}s", GetAnonyString(uuid).c_str());
    if (uuid.empty()) {
        LOGE("[L8:DiskUtils] ReadVolumeUuid: <<< EXIT FAILED <<< External volume ReadMetadata error");
        return E_READMETADATA;
    }
    LOGI("[L8:DiskUtils] ReadVolumeUuid: <<< EXIT SUCCESS <<<");
    return E_OK;
}

std::string GetBlkidData(const std::string &devPath, const std::string &type)
{
    LOGD("[L8:DiskUtils] GetBlkidData: >>> ENTER <<< devPath=%{public}s, type=%{public}s",
        devPath.c_str(), type.c_str());
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
    for (auto str : output) {
        LOGI("GetBlkidDataByCmd output: %{public}s", str.c_str());
    }
    if (err) {
        StorageRadar::ReportVolumeOperation("ForkExec", err);
        LOGE("[L8:DiskUtils] GetBlkidDataByCmd: <<< EXIT FAILED <<< ForkExec failed, err=%{public}d", err);
        return "";
    }

    if (output.size() > 0) {
        size_t sep = output[0].find_first_of("\n");
        if (sep != string::npos)
            output[0].resize(sep);
        LOGD("[L8:DiskUtils] GetBlkidDataByCmd: <<< EXIT SUCCESS <<<");
        return output[0];
    }
    LOGD("[L8:DiskUtils] GetBlkidDataByCmd: <<< EXIT SUCCESS <<< empty result");
    return "";
}

std::string GenerateRandomUuid(const std::string &diskPath, const std::string &uuidFormat)
{
    LOGD("[L8:DiskUtils] GenerateRandomUuid: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());
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

    LOGD("[L8:DiskUtils] GenerateRandomUuid: <<< EXIT SUCCESS <<<");
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
    LOGD("[L8:DiskUtils] SendScsiCmd: >>> ENTER <<< fd=%{public}d", fd);
    sg_io_hdr_t ioHdr;
    uint8_t sense[SENSE_BUFF_LEN];
    if (memset_s(&ioHdr, sizeof(ioHdr), 0, sizeof(ioHdr)) != 0) {
        LOGE("[L8:DiskUtils] SendScsiCmd: <<< EXIT FAILED <<< ioHdr memset_s failed");
        return E_ERR;
    }
    if (memset_s(sense, sizeof(sense), 0, sizeof(sense)) != 0) {
        LOGE("[L8:DiskUtils] SendScsiCmd: <<< EXIT FAILED <<< sense memset_s failed");
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
        LOGE("[L8:DiskUtils] SendScsiCmd: <<< EXIT FAILED <<< ioctl SG_IO failed");
        return E_ERR;
    }
    if ((ioHdr.info & SG_INFO_OK_MASK) != SG_INFO_OK) {
        LOGE("[L8:DiskUtils] SendScsiCmd: <<< EXIT FAILED <<< SG_INFO not OK");
        return E_ERR;
    }
    LOGD("[L8:DiskUtils] SendScsiCmd: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int GetCDStatus(const char *device, int &status)
{
    char realPath[PATH_MAX] = { 0 };
    if (realpath(device, realPath) == nullptr) {
        LOGE("realpath failed.");
        return E_FILE_PATH_INVALID;
    }
    FILE* file = fopen(realPath, "rb");
    if (file == nullptr) {
        LOGE("fopen failed errStr:%{public}s errno:%{public}d", strerror(errno), errno);
        return E_SYS_KERNEL_ERR;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("fileno failed, errStr:%{public}s errno:%{public}d", strerror(errno), errno);
        (void)fclose(file);
        return E_SYS_KERNEL_ERR;
    }

    /* Get CD status by ioctl
     * CDS_NO_DISC - 1 - No disk.
     * CDS_TRAY_OPEN - 2 - Tray open.
     * CDS_DRIVE_NOT_READY - 3 - CD-Rom in tray but not ready, wait init.
     * CDS_DISC_OK - 4 - CD-Rom is ready.
     */
    auto startTime = StorageService::StorageRadar::RecordCurrentTime();
    int slot = INT_MAX;
    status = ioctl(fd, CDROM_DRIVE_STATUS, &slot);
    if (status < 0) {
        LOGE("CD status:%{public}d errStr:%{public}s errno:%{public}d", status, strerror(errno), errno);
        (void)fclose(file);
        return E_ERR;
    }
    (void)fclose(file);
    auto delay = StorageService::StorageRadar::ReportDuration("GET CD STATUS: CD ROM",
        startTime, StorageService::DELAY_TIME_THRESH_HIGH, 0);
    LOGW("SD_DURATION: GET CD STATUS: device=%{public}s, delay = %{public}s", device, delay.c_str());
    return E_OK;
}

int ReadDiscInfo(const std::string &diskPath, int32_t cmdIndex, uint8_t *buf, int len)
{
    LOGD("[L8:DiskUtils] ReadDiscInfo: >>> ENTER <<< diskPath=%{public}s, len=%{public}d", diskPath.c_str(), len);
    char realPath[PATH_MAX] = { 0 };
    if (realpath(diskPath.c_str(), realPath) == nullptr) {
        LOGE("[L8:DiskUtils] ReadDiscInfo: <<< EXIT FAILED <<< realpath failed");
        return E_ERR;
    }
    FILE* file = fopen(realPath, "rb");
    if (file == nullptr) {
        LOGE("[L8:DiskUtils] ReadDiscInfo: <<< EXIT FAILED <<< fopen failed, errno=%{public}d", errno);
        return E_ERR;
    }
    int fd = fileno(file);
    if (fd < 0) {
        LOGE("[L8:DiskUtils] ReadDiscInfo: <<< EXIT FAILED <<< fileno error=%{public}d", errno);
        (void)fclose(file);
        return E_ERR;
    }
    uint8_t cdb[READ_DISC_INFO_CDB_LEN] = { cmdIndex };
    cdb[CDB_ALLOCATION_LENGTH_HIGH] = static_cast<uint8_t>(static_cast<uint32_t>(len) >> CDB_ALLOCATION_LENGTH_LOW);
    cdb[CDB_ALLOCATION_LENGTH_LOW] = static_cast<uint8_t>(static_cast<uint32_t>(len) & MAX_ALLOC_LEN);

    int ret = SendScsiCmd(fd, cdb, sizeof(cdb), buf, len);
    if (ret != 0) {
        LOGE("[L8:DiskUtils] ReadDiscInfo: <<< EXIT FAILED <<< SendScsiCmd failed, err=%{public}d", ret);
        (void)fclose(file);
        return ret;
    }
    (void)fclose(file);
    LOGD("[L8:DiskUtils] ReadDiscInfo: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int IsExistCD(const std::string &diskPath, bool &isExistCD)
{
    LOGD("[L8:DiskUtils] IsExistCD: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());
    int status = 0;
    isExistCD = false;
    int ret = GetCDStatus(diskPath.c_str(), status);
    if (ret != E_OK) {
        LOGE("Unknown cd status !");
        return E_ERR;
    }
    isExistCD = (status == CDS_DRIVE_NOT_READY || status == CDS_DISC_OK);
    LOGI("[L8:DiskUtils] IsExistCD:Current CD statue, isExistCD: %{public}d", isExistCD);
    return E_OK;
}

int IsBlankCD(const std::string &diskPath, bool &isBlankCD)
{
    isBlankCD = false;
    bool isExistCD = false;
    int existRet = IsExistCD(diskPath, isExistCD);
    if (existRet != E_OK || !isExistCD) {
        LOGE("CD is not exist, please check ! existRet:%{public}d, isExistCD:%{public}d", existRet, isExistCD);
        return E_ERR;
    }

    uint8_t buf[MAX_BUF];
    if (ReadDiscInfo(diskPath, READ_DISC_INFO_OPCODE, buf, sizeof(buf)) == E_OK) {
        uint8_t discStatus = buf[DISC_STATUS_BYTE_INDEX] & DISC_STATUS_MASK;
        isBlankCD = (discStatus == 0);
        LOGI("[L8:DiskUtils] IsBlankCD: <<< EXIT SUCCESS <<< isBlank=%{public}d", isBlankCD);
        return E_OK;
    } else {
        LOGE("[L8:DiskUtils] IsBlankCD: <<< EXIT FAILED <<< Unable to read disc information");
    }
    LOGE("Unable to read disc information.");
    return E_ERR;
}

std::string DiskType2Str(uint8_t diskType)
{
    LOGD("[L8:DiskUtils] DiskType2Str: >>> ENTER <<< diskType=0x%{public}x", diskType);
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
            LOGW("[L8:DiskUtils] DiskType2Str: unknown diskType=0x%{public}x", diskType);
            return "";
    }
}

std::string GetCDType(const std::string &diskPath)
{
    LOGD("[L8:DiskUtils] GetCDType: >>> ENTER <<< diskPath=%{public}s", diskPath.c_str());
    uint8_t buf[MAX_BUF];
    if (ReadDiscInfo(diskPath, READ_DISC_INFO_SECTOR, buf, sizeof(buf)) == 0) {
        return DiskType2Str((buf[DISC_TYPE_OFFSET_HIGH] << DISC_TYPE_OFFSET) | buf[DISC_TYPE_OFFSET_LOW]);
    } else {
        LOGE("[L8:DiskUtils] GetCDType: <<< EXIT FAILED <<< Unable to read disc information");
    }
    return "";
}

int Eject(const std::string &devPath)
{
    LOGI("[L8:DiskUtils] Eject: >>> ENTER <<< devPath=%{public}s", devPath.c_str());
    std::vector<std::string> output;
    std::vector<std::string> cmd = {
        "eject",
        "-s",
        devPath
    };
    int res = ForkExec(cmd, &output);
    for (auto str : output) {
        LOGI("Eject output: %{public}s", str.c_str());
    }
    if (res != E_OK) {
        LOGE("[L8:DiskUtils] Eject: <<< EXIT FAILED <<< ForkExec eject failed, err=%{public}d", res);
        return res;
    }
    LOGI("[L8:DiskUtils] Eject: <<< EXIT SUCCESS <<<");
    return E_OK;
}

int GetCdTotalCapacity(int fd, int64_t &cdTotalCapacity)
{
    unsigned char cmd_buf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmd_len = GET_CD_TOTAL_CAPACITY_CMD_LEN;
    unsigned char data_buf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    unsigned int data_len = GET_CD_TOTAL_CAPACITY_DATA_LEN;
    unsigned int actual_len = 0;
    int ret = 0;
    double total_seconds = 0;
    /**
      * 功能：通过 SCSI READ TOC 指令获取cd光盘的 ATIP 信息。
      * ATIP 包含光盘介质类型、制造商及容量等底层参数。
      * 参数说明：
      * cmd_buf[2] = 0x04: 指定读取格式为 ATIP。
      * cmd_buf[7-8]: 定义设备返回数据的最大长度。
    */
    cmd_buf[0] = GPCMD_READ_TOC_PMA_ATIP;
    cmd_buf[2] = 0x04;
    cmd_buf[7] = (data_len >> 8) & 0xff;
    cmd_buf[8] = data_len & 0xff;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, data_len);
    if (ret != 0) {
        LOGE("get atip data len failed, ret val is %{public}d", ret);
        return E_ERR;
    }
    actual_len = (int)(data_buf[0] << 8) | data_buf[1];
    actual_len += 2;
    LOGI("actual_len: %{public}d", actual_len);
    cmd_buf[7] = (actual_len >> 8) & 0xff;
    cmd_buf[8] = actual_len & 0xff;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, actual_len);
    if (ret != 0) {
        LOGE("get atip data failed, ret is %{public}d", ret);
        return E_ERR;
    }
 
    total_seconds = (double)data_buf[12] * SECONDS_PER_MINUTES + data_buf[13] +
                    (double)data_buf[14] / CD_SECTORS_PER_SECOND;
    cdTotalCapacity = static_cast<int64_t>(total_seconds * CD_SECTORS_PER_SECOND * ODD_LOGICAL_SECTOR_SIZE);
    LOGI("cd total_seconds: %{public}f, total_capacity: %{public}" PRIu64, total_seconds, cdTotalCapacity);
    return E_OK;
}

int GetCdUsedCapacity(int fd, int64_t &cdUsedCapacity)
{
    unsigned char cmd_buf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmd_len = GET_CD_USED_CAPACITY_CMD_LEN;
    unsigned char data_buf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    unsigned int data_len = GET_CD_USED_CAPACITY_DATA_LEN;
    int ret = 0;
    /*
    * 使用 SCSI READ TRACK INFORMATION 指令 (0x52) 获取cd光盘轨道/逻辑分区信息
    * cmd_buf[0]: 指令操作码 0x52 (READ TRACK/RZONE INFORMATION)
    * cmd_buf[1]: 设置为 1，表示通过轨道编号 (Track Number) 寻址
    * cmd_buf[5]: 设置为 0xff，通常用于请求最后一条轨道 (Invisible Track) 或特定状态信息
    * cmd_buf[7-8]: 分别填充 data_len 的高8位和低8位，告知驱动器返回数据的长度限制
    */
    cmd_buf[0] = GPCMD_READ_TRACK_RZONE_INFO;
    cmd_buf[1] = 1;
    cmd_buf[5] = 0xff;
    cmd_buf[7] = (data_len >> 8) & 0xff;
    cmd_buf[8] = data_len & 0xff;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, data_len);
    if (ret != 0) {
        LOGE("get cd used capacity failed, ret val is %{public}d", ret);
        return E_ERR;
    }
 
    cdUsedCapacity = ((unsigned long long)data_buf[8] << 24) |
                     ((unsigned long long)data_buf[9] << 16) |
                     ((unsigned long long)data_buf[10] << 8) |
                     data_buf[11];
    cdUsedCapacity *= ODD_LOGICAL_SECTOR_SIZE;
    LOGI("cd used_capacity: %{public}" PRIu64, cdUsedCapacity);
    return E_OK;
}

int GetDvdTotalCapacity(int fd, int64_t &dvdTotalCapacity)
{
    unsigned char cmd_buf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmd_len = GET_DVD_TOTAL_CAPACITY_CMD_LEN;
    unsigned char data_buf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    int data_len = GET_DVD_TOTAL_CAPACITY_DATA_LEN;
    int ret = 0;
    unsigned long long phys_start = 0;
    unsigned long long phys_end = 0;
    int dvd_media = 0;
    ret = GetDvdConfiguration(fd, dvd_media);
    if (ret != 0) {
        LOGE("get dvd configuration failed,ret val is %{public}d", ret);
        return E_ERR;
    }
    /*
    * 使用 SCSI READ DVD STRUCTURE 指令 (0xAD) 获取光盘物理结构或容量信息
    * cmd_buf[0]: 操作码 0xAD (GPCMD_READ_DVD_STRUCTURE)。
    * cmd_buf[7]: 格式码 (Format Code)。
    *             如果 dvd_media <= 0x18 (通常指 DVD-ROM/R/RW)，设置为 16 (0x10)，
    *             这对应 "DVD Structure List"，用于列出支持的结构。
    *             否则设置为 0，通常对应 "Physical Format Information"，读取最基础的物理层信息。
    * cmd_buf[9]: 分配长度 (Allocation Length) 的低位。
    *             这里通常设置为数据结构的总长度 (GET_DVD_TOTAL_CAPACITY_DATA_LEN)。
    * cmd_buf[11]:保留位/控制字节，通常设为 0。
    */
    cmd_buf[0] = GPCMD_READ_DVD_STRUCTURE;
    cmd_buf[7] = dvd_media <= 0x18 ? 16 : 0;
    cmd_buf[9] = GET_DVD_TOTAL_CAPACITY_DATA_LEN;
    cmd_buf[11] = 0;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, data_len);
    if (ret != 0) {
        LOGE("get dvd total capacity failed, ret val is %{public}d", ret);
        return E_ERR;
    }
 
    phys_start = ((unsigned long long)(data_buf[9]) << 16) |
                 ((unsigned long long)(data_buf[10]) << 8) |
                 data_buf[11];
    if ((data_buf[6] & 0x60) == 0) {
        phys_end = ((unsigned long long)(data_buf[13]) << 16) |
                   ((unsigned long long)(data_buf[14]) << 8) |
                   data_buf[15];
    } else {
        phys_end = ((unsigned long long)(data_buf[17]) << 16) |
                    ((unsigned long long)(data_buf[18]) << 8) |
                    data_buf[19];
    }
    if (phys_end > phys_start) {
        dvdTotalCapacity = static_cast<int64_t>(phys_end - phys_start + 1) * ODD_LOGICAL_SECTOR_SIZE;
    } else {
        dvdTotalCapacity = 0;
        LOGI("phys_end is less than phys_start, error data");
    }
    
    LOGI("dvd dvdTotalCapacity: %{public}" PRIu64, dvdTotalCapacity);
    return E_OK;
}
 
int GetDvdUsedCapacity(int fd, int64_t &dvdUsedCapcity)
{
    unsigned char cmd_buf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmd_len = GET_DVD_USED_CAPACITY_CMD_LEN;
    unsigned char data_buf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    unsigned int data_len = GET_DVD_USED_CAPACITY_DATA_LEN;
    int ret = 0;
    unsigned int blk_cnt = 0;
    unsigned int blk_size = 0;
    /*
     * 使用 SCSI READ CAPACITY 指令 (0x25) 获取光盘介质的逻辑容量。
     * 字段详解:
     * cmd_buf[0]: 操作码 0x25 (GPCMD_READ_CDVD_CAPACITY)。
     *             用于请求光盘的最后逻辑块地址 (Last LBA) 和块大小 (Block Length)。
     * cmd_buf[7-8]: 分配长度 (Allocation Length)。
     *               告知驱动器返回数据的最大长度（通常为 8 字节）。
     *               高 8 位填入 cmd_buf[7]，低 8 位填入 cmd_buf[8]。
     * 注意：此指令返回的 LBA 需加 1 才是总扇区数。
     */
    cmd_buf[0] = GPCMD_READ_CDVD_CAPACITY;
    cmd_buf[7] = (data_len >> 8) & 0xff;
    cmd_buf[8] = data_len & 0xff;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, data_len);
    if (ret != 0) {
        LOGE("get dvd total capacity failed, ret val is %{public}d", ret);
        return E_ERR;
    }
 
    blk_cnt = ((unsigned int)data_buf[0] << 24) |
              ((unsigned int)data_buf[1] << 16) |
              ((unsigned int)data_buf[2] << 8) |
              data_buf[3];
    blk_size = ((unsigned int)data_buf[4] << 24) |
               ((unsigned int)data_buf[5] << 16) |
               ((unsigned int)data_buf[6] << 8) |
               data_buf[7];
    dvdUsedCapcity = static_cast<int64_t>(blk_cnt + 1) * blk_size;
    LOGI("dvd used_capacity: %{public}u * %{public}u = %{public}" PRIu64, blk_cnt + 1, blk_size, dvdUsedCapcity);
    return E_OK;
}
 
int GetDvdConfiguration(int fd, int &dvdMedia)
{
    unsigned char cmd_buf[GET_CAPACITY_CMD_BUF_LEN] = {0};
    int cmd_len = GET_DVD_USED_CAPACITY_CMD_LEN;
    unsigned char data_buf[GET_CAPACITY_DATA_BUF_LEN] = {0};
    unsigned int data_len = GET_DVD_USED_CAPACITY_DATA_LEN;
    int ret = 0;
    /*
     * 使用 SCSI GET CONFIGURATION 指令 (0x46) 获取光驱设备的功能列表。
     * 字段详解:
     * cmd_buf[0]: 操作码 0x46 (GPCMD_GET_CONFIGURATION)。
     * cmd_buf[1]: RT (Request Type) 字段。
     *             设置为 1 (01b) 表示请求“当前活动的功能”(Current Features Only)，
     *             即仅列出当前插入介质所支持的功能（如刻录、读取能力）。
     * cmd_buf[7-8]: 分配长度 (Allocation Length)。
     *               指定接收配置数据（Feature Descriptors）的最大缓冲区长度。
     *               高 8 位存入 cmd_buf[7]，低 8 位存入 cmd_buf[8]。
     * 用途:
     * 常用于探测光驱是否支持特定的 Profile（如 DVD-RW, BD-RE）以及当前光盘的写入模式。
     */
    cmd_buf[0] = GPCMD_GET_CONFIGURATION;
    cmd_buf[1] = 1;
    cmd_buf[7] = (data_len >> 8) & 0xff;
    cmd_buf[8] = data_len & 0xff;
    ret = SendScsiCmd(fd, cmd_buf, cmd_len, data_buf, data_len);
    if (ret != 0) {
        LOGE("get atip data len failed, ret val is %{public}d", ret);
        return E_ERR;
    }
 
    dvdMedia = static_cast<int>((unsigned int)data_buf[DISC_TYPE_OFFSET_HIGH] << DISC_TYPE_OFFSET) |
        data_buf[DISC_TYPE_OFFSET_LOW];
    LOGI("dvd_media: %{public}#x", dvdMedia);
    return E_OK;
}
} // namespace STORAGE_DAEMON
} // namespace OHOS
