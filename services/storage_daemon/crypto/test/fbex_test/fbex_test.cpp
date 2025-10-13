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

#include "fbex.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlfcn.h>
#include <sys/ioctl.h>
#include <cstdio>
#include <cstdarg>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

#include "openssl_crypto.h"
#include "common_utils_mock.h"

namespace {
constexpr uint32_t FILE_ENCRY_ERROR_UECE_ALREADY_CREATED = 0xFBE30031;
constexpr uint32_t FILE_ENCRY_ERROR_NOT_FOUND_UECE = 0xFBE30033;
constexpr uint32_t UNLOCK_STATUS = 0x2;
}

namespace OHOS::StorageDaemon::Test {
class IFuncMock {
public:
    virtual ~IFuncMock() = default;

    virtual FILE *fopen(const char *filename, const char *restrict_modes) = 0;
    virtual int open(const char *file, int oflag) = 0;
    virtual int fileno(FILE *stream) = 0;
    virtual int ioctl(int fd, int request) = 0;
    virtual int fclose(FILE *stream) = 0;
    virtual int close(int fd) = 0;
    virtual char *realpath(const char *path, char *resolved_path) = 0;

public:
    static inline std::shared_ptr<IFuncMock> iFuncMock_ = nullptr;
};

class FuncMock : public IFuncMock {
public:
    MOCK_METHOD(FILE *, fopen, (const char *, const char *));
    MOCK_METHOD(int, open, (const char *, int));
    MOCK_METHOD(int, fileno, (FILE *));
    MOCK_METHOD(int, ioctl, (int, int));
    MOCK_METHOD(int, fclose, (FILE *));
    MOCK_METHOD(int, close, (int));
    MOCK_METHOD(char *, realpath, (const char *, char *));
};

class FbexTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void MockFopenSuccess();
    void MockopenSuccess();

    static inline std::shared_ptr<FuncMock> funcMock_ = nullptr;
    static inline std::shared_ptr<CommonUtilsMock> commonUtilsMock_ = nullptr;
    static inline FILE f_;
    static inline KeyBlob iv_;
    static inline int fd_ = 0xffff;
};
} // namespace OHOS::StorageDaemon::Test

using namespace OHOS::StorageDaemon::Test;
typedef FILE *(*fopenFuncT)(const char *, const char *);
FILE *fopen64(const char *filename, const char *restrict_modes)
{
    if (strcmp(filename, "/dev/fbex_uece") != 0 && strcmp(filename, "/dev/fbex_cmd") != 0) {
        fopenFuncT originalFopen = reinterpret_cast<fopenFuncT>(dlsym(RTLD_NEXT, "fopen"));
        if (originalFopen == nullptr) {
            return nullptr;
        }
        return originalFopen(filename, restrict_modes);
    }
    if (IFuncMock::iFuncMock_ == nullptr) {
        return nullptr;
    }
    return IFuncMock::iFuncMock_->fopen(filename, restrict_modes);
}

typedef int (*openFuncT)(const char *, int);
int open(const char *file, int oflag, ...)
{
    if (strcmp(file, "/dev/fbex_uece") != 0 && strcmp(file, "/dev/fbex_cmd") != 0) {
        openFuncT originalOpen = reinterpret_cast<openFuncT>(dlsym(RTLD_NEXT, "open"));
        if (originalOpen == nullptr) {
            return -1;
        }
        return originalOpen(file, oflag);
    }
    if (IFuncMock::iFuncMock_ == nullptr) {
        return 0;
    }
    return IFuncMock::iFuncMock_->open(file, oflag);
}

int fileno(FILE *stream)
{
    if (IFuncMock::iFuncMock_ == nullptr) {
        return -1;
    }
    return IFuncMock::iFuncMock_->fileno(stream);
}

int ioctl(int fd, int request, ...)
{
    if (IFuncMock::iFuncMock_ == nullptr) {
        return -1;
    }
    return IFuncMock::iFuncMock_->ioctl(fd, request);
}

typedef int (*fcloseFuncT)(FILE *);
int fclose(FILE *stream)
{
    if (IFuncMock::iFuncMock_ == nullptr || stream != &FbexTest::f_) {
        fcloseFuncT originalFclose = reinterpret_cast<fcloseFuncT>(dlsym(RTLD_NEXT, "fclose"));
        if (originalFclose == nullptr) {
            return -1;
        }
        return originalFclose(stream);
    }
    return IFuncMock::iFuncMock_->fclose(stream);
}

typedef int (*closeFuncT)(int);
int close(int fd)
{
    if (IFuncMock::iFuncMock_ == nullptr || fd != FbexTest::fd_) {
        closeFuncT originalClose = reinterpret_cast<closeFuncT>(dlsym(RTLD_NEXT, "close"));
        if (originalClose == nullptr) {
            return -1;
        }
        return originalClose(fd);
    }
    return IFuncMock::iFuncMock_->close(fd);
}

char *realpath(const char *path, char *resolved_path)
{
    if (IFuncMock::iFuncMock_ == nullptr) {
        return nullptr;
    }
    return IFuncMock::iFuncMock_->realpath(path, resolved_path);
}

namespace OHOS::StorageDaemon::Test {
using namespace testing::ext;
using namespace testing;
void FbexTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "FbexTest SetUpTestCase";
}

void FbexTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "FbexTest TearDownTestCase";
}

void FbexTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    funcMock_ = std::make_shared<FuncMock>();
    FuncMock::iFuncMock_ = funcMock_;
    commonUtilsMock_ = std::make_shared<CommonUtilsMock>();
    CommonUtilsMock::utils = commonUtilsMock_;

    ON_CALL(*funcMock_, fileno(_)).WillByDefault(Return(0));
    ON_CALL(*funcMock_, fclose(_)).WillByDefault(Return(0));
    ON_CALL(*funcMock_, close(_)).WillByDefault(Return(0));

    iv_ = KeyBlob(FBEX_IV_SIZE);
}

void FbexTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    FuncMock::iFuncMock_ = nullptr;
    funcMock_ = nullptr;
    CommonUtilsMock::utils = nullptr;
    commonUtilsMock_ = nullptr;

    iv_.Clear();
}

inline void FbexTest::MockFopenSuccess()
{
    EXPECT_CALL(*funcMock_, fopen(_, _)).WillOnce(Return(&f_));
    EXPECT_CALL(*funcMock_, fclose(_)).WillOnce(Return(1));
}

inline void FbexTest::MockopenSuccess()
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(fd_));
    EXPECT_CALL(*funcMock_, close(_)).WillOnce(Return(1));
}
/**
 * @tc.name: Fbex_IsFBEXSupported_001
 * @tc.desc: Should returns false when LoadStringFromFile failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_IsFBEXSupported_001, TestSize.Level1)
{
    EXPECT_CALL(*commonUtilsMock_, LoadStringFromFile(_, _)).WillOnce(Return(false));

    EXPECT_FALSE(FBEX::IsFBEXSupported());
}

/**
 * @tc.name: Fbex_IsFBEXSupported_002
 * @tc.desc: Should returns false when rpath not find FBEX_UFS_INLINE_SUPPORT_PREFIX.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_IsFBEXSupported_002, TestSize.Level1)
{
    char testPath[] = "testpath";
    EXPECT_CALL(*commonUtilsMock_, LoadStringFromFile(_, _)).WillOnce(DoAll(SetArgReferee<1>(""), Return(true)));
    EXPECT_CALL(*funcMock_, realpath(_, _)).WillOnce(DoAll(WithArgs<1>(Invoke([](char *resolved_path) {
        char testPath[] = "testpath";
        strcpy_s(resolved_path, PATH_MAX, testPath);
    })), Return(testPath)));

    EXPECT_FALSE(FBEX::IsFBEXSupported());
}

/**
 * @tc.name: Fbex_IsFBEXSupported_003
 * @tc.desc: Should returns false when the second LoadStringFromFile failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_IsFBEXSupported_003, TestSize.Level1)
{
    char testPath[] = "testpath";
    EXPECT_CALL(*commonUtilsMock_, LoadStringFromFile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(true))).WillOnce(Return(false));
    EXPECT_CALL(*funcMock_, realpath(_, _)).WillOnce(DoAll(
        WithArgs<0, 1>(Invoke([](const char *path, char *resolved_path) {
            strcpy_s(resolved_path, PATH_MAX, path);
        })), Return(testPath)));

    EXPECT_FALSE(FBEX::IsFBEXSupported());
}

/**
 * @tc.name: Fbex_IsFBEXSupported_004
 * @tc.desc: Should returns true when all operations succeed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_IsFBEXSupported_004, TestSize.Level1)
{
    char testPath[] = "testpath";
    EXPECT_CALL(*commonUtilsMock_, LoadStringFromFile(_, _))
        .WillOnce(DoAll(SetArgReferee<1>(""), Return(true)))
        .WillOnce(DoAll(SetArgReferee<1>("3\n"), Return(true)));
    EXPECT_CALL(*funcMock_, realpath(_, _)).WillOnce(DoAll(
        WithArgs<0, 1>(Invoke([](const char *path, char *resolved_path) {
            strcpy_s(resolved_path, PATH_MAX, path);
        })), Return(testPath)));

    EXPECT_TRUE(FBEX::IsFBEXSupported());
}

/**
 * @tc.name: Fbex_InstallEL5KeyToKernel_001
 * @tc.desc: Should returns -errno and closes the file when open < 0 returns -EACCES.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallEL5KeyToKernel_001, TestSize.Level1)
{
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::InstallEL5KeyToKernel(1, 1, 1, isSupport, isNeedEncryptClassE), -EACCES);
    EXPECT_TRUE(isSupport);
}

/**
 * @tc.name: Fbex_InstallEL5KeyToKernel_002
 * @tc.desc: Should returns -errno and closes the file when open < 0 returns -EACCES.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallEL5KeyToKernel_002, TestSize.Level1)
{
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::InstallEL5KeyToKernel(1, 1, 1, isSupport, isNeedEncryptClassE), 0);
    EXPECT_FALSE(isSupport);
}

/**
 * @tc.name: Fbex_InstallEL5KeyToKernel_003
 * @tc.desc: Should returns 0 and sets isNeedEncryptClassE to false when ioctl returns ERROR_UECE_ALREADY_CREATED.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallEL5KeyToKernel_003, TestSize.Level1)
{
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(FILE_ENCRY_ERROR_UECE_ALREADY_CREATED));

    EXPECT_EQ(FBEX::InstallEL5KeyToKernel(1, 1, 1, isSupport, isNeedEncryptClassE), 0);
    EXPECT_FALSE(isNeedEncryptClassE);
}

/**
 * @tc.name: Fbex_InstallEL5KeyToKernel_004
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallEL5KeyToKernel_004, TestSize.Level1)
{
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(FILE_ENCRY_ERROR_UECE_ALREADY_CREATED + 1));
    errno = EACCES;

    EXPECT_EQ(FBEX::InstallEL5KeyToKernel(1, 1, 1, isSupport, isNeedEncryptClassE), -EACCES);
    EXPECT_TRUE(isSupport);
}

/**
 * @tc.name: Fbex_InstallEL5KeyToKernel_005
 * @tc.desc: Should returns -errno when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallEL5KeyToKernel_005, TestSize.Level1)
{
    bool isSupport = true;
    bool isNeedEncryptClassE = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::InstallEL5KeyToKernel(1, 1, 1, isSupport, isNeedEncryptClassE), 0);
    EXPECT_TRUE(isSupport);
    EXPECT_TRUE(isNeedEncryptClassE);
}

/**
 * @tc.name: Fbex_InstallKeyToKernel_001
 * @tc.desc: Should return -EINVAL when iv is empty or invalid
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallKeyToKernel_001, TestSize.Level1)
{
    KeyBlob iv;
    KeyBlob authToken;

    EXPECT_EQ(FBEX::InstallKeyToKernel(1, 1, iv, 1, authToken), -EINVAL);

    iv = KeyBlob(1);
    EXPECT_EQ(FBEX::InstallKeyToKernel(1, 1, iv, 1, authToken), -EINVAL);
}

/**
 * @tc.name: Fbex_InstallKeyToKernel_002
 * @tc.desc: Should return -errno when fopen failed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallKeyToKernel_002, TestSize.Level1)
{
    KeyBlob authToken;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::InstallKeyToKernel(1, 1, iv_, 1, authToken), -EACCES);
}

/**
 * @tc.name: Fbex_InstallKeyToKernel_003
 * @tc.desc: Should return ret when ioctl failed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallKeyToKernel_003, TestSize.Level1)
{
    KeyBlob authToken;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));

    EXPECT_EQ(FBEX::InstallKeyToKernel(1, 1, iv_, 1, authToken), -1);
}

/**
 * @tc.name: Fbex_InstallKeyToKernel_004
 * @tc.desc: Should return ret when all operations succeed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallKeyToKernel_004, TestSize.Level1)
{
    KeyBlob authToken;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::InstallKeyToKernel(1, 1, iv_, 1, authToken), 0);
}

/**
 * @tc.name: Fbex_InstallDoubleDeKeyToKernel_001
 * @tc.desc: Should return -EINVAL when iv is empty or invalid
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallDoubleDeKeyToKernel_001, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob iv;
    KeyBlob authToken;
    EXPECT_EQ(FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv, 1, authToken), -EINVAL);

    iv = KeyBlob(1);
    EXPECT_EQ(FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv, 1, authToken), -EINVAL);
}

/**
 * @tc.name: Fbex_InstallDoubleDeKeyToKernel_002
 * @tc.desc: Should return -errno when open failed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallDoubleDeKeyToKernel_002, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob authToken;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv_, 1, authToken), -EACCES);
}

/**
 * @tc.name: Fbex_InstallDoubleDeKeyToKernel_003
 * @tc.desc: Should return 0 when ioctl failed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallDoubleDeKeyToKernel_003, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob authToken;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(fd_));
    EXPECT_CALL(*funcMock_, close(_)).WillOnce(Return(1));
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));

    EXPECT_EQ(FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv_, 1, authToken), -1);
}

/**
 * @tc.name: Fbex_InstallDoubleDeKeyToKernel_004
 * @tc.desc: Should return 0 when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_InstallDoubleDeKeyToKernel_004, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob authToken;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(fd_));
    EXPECT_CALL(*funcMock_, close(_)).WillOnce(Return(1));

    EXPECT_EQ(FBEX::InstallDoubleDeKeyToKernel(userIdToFbe, iv_, 1, authToken), 0);
}

/**
 * @tc.name: Fbex_UninstallOrLockUserKeyToKernel_001
 * @tc.desc: Should return -EINVAL when iv is empty
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UninstallOrLockUserKeyToKernel_001, TestSize.Level1)
{
    uint8_t *iv = nullptr;

    EXPECT_EQ(FBEX::UninstallOrLockUserKeyToKernel(1, 1, iv, 1, true), -EINVAL);
}

/**
 * @tc.name: Fbex_UninstallOrLockUserKeyToKernel_002
 * @tc.desc: Should return -errno when fopen failed
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UninstallOrLockUserKeyToKernel_002, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::UninstallOrLockUserKeyToKernel(1, 1, iv_.data.get(), iv_.size, true), -EACCES);
}

/**
 * @tc.name: Fbex_UninstallOrLockUserKeyToKernel_004
 * @tc.desc: Should return ret when ioctl faile and not return FILE_ENCRY_ERROR_NOT_FOUND_UECE
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UninstallOrLockUserKeyToKernel_004, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(FILE_ENCRY_ERROR_NOT_FOUND_UECE + 1));

    int ret = FBEX::UninstallOrLockUserKeyToKernel(1, 1, iv_.data.get(), iv_.size, true);

    EXPECT_EQ(ret, FILE_ENCRY_ERROR_NOT_FOUND_UECE + 1);
}

/**
 * @tc.name: Fbex_UninstallOrLockUserKeyToKernel_005
 * @tc.desc: Should return 0 when ioctl return FILE_ENCRY_ERROR_NOT_FOUND_UECE
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UninstallOrLockUserKeyToKernel_005, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(FILE_ENCRY_ERROR_NOT_FOUND_UECE));

    EXPECT_EQ(FBEX::UninstallOrLockUserKeyToKernel(1, 1, iv_.data.get(), iv_.size, true), 0);
}

/**
 * @tc.name: Fbex_UninstallOrLockUserKeyToKernel_006
 * @tc.desc: Should return 0 when ioctl success
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UninstallOrLockUserKeyToKernel_006, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::UninstallOrLockUserKeyToKernel(1, 1, iv_.data.get(), iv_.size, true), 0);
}

/**
 * @tc.name: Fbex_DeleteClassEPinCode_001
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_DeleteClassEPinCode_001, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::DeleteClassEPinCode(1, 1), 0);
}

/**
 * @tc.name: Fbex_DeleteClassEPinCode_002
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_DeleteClassEPinCode_002, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::DeleteClassEPinCode(1, 1), -EACCES);
}

/**
 * @tc.name: Fbex_DeleteClassEPinCode_003
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_DeleteClassEPinCode_003, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::DeleteClassEPinCode(1, 1), -EACCES);
}

/**
 * @tc.name: Fbex_DeleteClassEPinCode_004
 * @tc.desc: Should returns 0 when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_DeleteClassEPinCode_004, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::DeleteClassEPinCode(1, 1), 0);
}

/**
 * @tc.name: Fbex_ChangePinCodeClassE_001
 * @tc.desc: Should returns 0 when open failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ChangePinCodeClassE_001, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::ChangePinCodeClassE(1, 1, isFbeSupport), 0);
    EXPECT_FALSE(isFbeSupport);
}

/**
 * @tc.name: Fbex_ChangePinCodeClassE_002
 * @tc.desc: Should returns -errno when open failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ChangePinCodeClassE_002, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::ChangePinCodeClassE(1, 1, isFbeSupport), -EACCES);
}

/**
 * @tc.name: Fbex_ChangePinCodeClassE_003
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ChangePinCodeClassE_003, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(fd_));
    EXPECT_CALL(*funcMock_, close(_)).WillOnce(Return(0));
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::ChangePinCodeClassE(1, 1, isFbeSupport), -EACCES);
}

/**
 * @tc.name: Fbex_ChangePinCodeClassE_004
 * @tc.desc: Should returns 0 when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ChangePinCodeClassE_004, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(fd_));
    EXPECT_CALL(*funcMock_, close(_)).WillOnce(Return(0));
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::ChangePinCodeClassE(1, 1, isFbeSupport), 0);
}

/**
 * @tc.name: Fbex_UpdateClassEBackUp_001
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UpdateClassEBackUp_001, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::UpdateClassEBackUp(1, 1), 0);
}

/**
 * @tc.name: Fbex_UpdateClassEBackUp_002
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UpdateClassEBackUp_002, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::UpdateClassEBackUp(1, 1), -EACCES);
}

/**
 * @tc.name: Fbex_UpdateClassEBackUp_003
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UpdateClassEBackUp_003, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::UpdateClassEBackUp(1, 1), -EACCES);
}

/**
 * @tc.name: Fbex_UpdateClassEBackUp_004
 * @tc.desc: Should returns 0 when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UpdateClassEBackUp_004, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::UpdateClassEBackUp(1, 1), 0);
}

/**
 * @tc.name: Fbex_LockScreenToKernel_001
 * @tc.desc: Should returns -errno when fopen failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockScreenToKernel_001, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::LockScreenToKernel(1), -EACCES);
}

/**
 * @tc.name: Fbex_LockScreenToKernel_002
 * @tc.desc: Should returns ret when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockScreenToKernel_002, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));

    EXPECT_EQ(FBEX::LockScreenToKernel(1), -1);
}

/**
 * @tc.name: Fbex_LockScreenToKernel_003
 * @tc.desc: Should returns ret when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockScreenToKernel_003, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::LockScreenToKernel(1), 0);
}

/**
 * @tc.name: Fbex_GenerateAppkey_001
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GenerateAppkey_001, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::GenerateAppkey(userIdToFbe, 1, iv_.data, iv_.size), 0);
    EXPECT_EQ(iv_.data, nullptr);
}

/**
 * @tc.name: Fbex_GenerateAppkey_002
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GenerateAppkey_002, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::GenerateAppkey(userIdToFbe, 1, iv_.data, iv_.size), -EACCES);
}

/**
 * @tc.name: Fbex_GenerateAppkey_003
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GenerateAppkey_003, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::GenerateAppkey(userIdToFbe, 1, iv_.data, iv_.size), -EACCES);
}

/**
 * @tc.name: Fbex_GenerateAppkey_004
 * @tc.desc: Should returns 0 when when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GenerateAppkey_004, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::GenerateAppkey(userIdToFbe, 1, iv_.data, iv_.size), 0);
}

/**
 * @tc.name: Fbex_LockUece_001
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockUece_001, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::LockUece(1, 1, isFbeSupport), 0);
    EXPECT_FALSE(isFbeSupport);
}

/**
 * @tc.name: Fbex_LockUece_002
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockUece_002, TestSize.Level1)
{
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::LockUece(1, 1, isFbeSupport), -EACCES);
    EXPECT_TRUE(isFbeSupport);
}

/**
 * @tc.name: Fbex_LockUece_003
 * @tc.desc: Should returns ret when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockUece_003, TestSize.Level1)
{
    bool isFbeSupport = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));

    EXPECT_EQ(FBEX::LockUece(1, 1, isFbeSupport), -1);
}

/**
 * @tc.name: Fbex_LockUecey_004
 * @tc.desc: Should returns ret when when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_LockUecey_004, TestSize.Level1)
{
    bool isFbeSupport = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::LockUece(1, 1, isFbeSupport), 0);
}

/**
 * @tc.name: Fbex_UnlockScreenToKernel_001
 * @tc.desc: Should returns -EINVAL when iv is not vaild.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UnlockScreenToKernel_001, TestSize.Level1)
{
    uint8_t *iv = nullptr;
    KeyBlob authToken;

    EXPECT_EQ(FBEX::UnlockScreenToKernel(1, 1, iv, 1, authToken), -EINVAL);
}

/**
 * @tc.name: Fbex_UnlockScreenToKernel_002
 * @tc.desc: Should returns -errno when fopen failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UnlockScreenToKernel_002, TestSize.Level1)
{
    KeyBlob authToken;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::UnlockScreenToKernel(1, 1, iv_.data.get(), iv_.size, authToken), -EACCES);
}

/**
 * @tc.name: Fbex_UnlockScreenToKernel_003
 * @tc.desc: Should returns ret when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UnlockScreenToKernel_003, TestSize.Level1)
{
    KeyBlob authToken;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));

    EXPECT_EQ(FBEX::UnlockScreenToKernel(1, 1, iv_.data.get(), iv_.size, authToken), -1);
}

/**
 * @tc.name: Fbex_UnlockScreenToKernel_004
 * @tc.desc: Should returns ret when when all operations succeed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UnlockScreenToKernel_004, TestSize.Level1)
{
    KeyBlob authToken;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::UnlockScreenToKernel(1, 1, iv_.data.get(), iv_.size, authToken), 0);
}

/**
 * @tc.name: Fbex_ReadESecretToKernel_001
 * @tc.desc: Should returns -EINVAL when eBuffer is invaild.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ReadESecretToKernel_001, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob eBuffer;
    KeyBlob authToken;
    bool isFbeSupport = true;

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, 1, eBuffer, authToken, isFbeSupport), -EINVAL);

    eBuffer = KeyBlob(1);

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, 1, eBuffer, authToken, isFbeSupport), -EINVAL);
}

/**
 * @tc.name: Fbex_ReadESecretToKernel_002
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ReadESecretToKernel_002, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);
    KeyBlob authToken;
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, 1, iv_, authToken, isFbeSupport), 0);
    EXPECT_FALSE(isFbeSupport);
}

/**
 * @tc.name: Fbex_ReadESecretToKernel_003
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ReadESecretToKernel_003, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(GCM_NONCE_BYTES + AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES);
    KeyBlob authToken;
    bool isFbeSupport = true;

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, UNLOCK_STATUS, iv_, authToken, isFbeSupport), -EACCES);
    EXPECT_TRUE(isFbeSupport);
}

/**
 * @tc.name: Fbex_ReadESecretToKernel_004
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ReadESecretToKernel_004, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);
    KeyBlob authToken;
    bool isFbeSupport = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, 1, iv_, authToken, isFbeSupport), -EACCES);
}

/**
 * @tc.name: Fbex_ReadESecretToKernel_005
 * @tc.desc: Should returns 0 when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_ReadESecretToKernel_005, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);
    KeyBlob authToken;
    bool isFbeSupport = true;

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::ReadESecretToKernel(userIdToFbe, 1, iv_, authToken, isFbeSupport), 0);
}

/**
 * @tc.name: Fbex_UnlockSendSecret_001
 * @tc.desc: Should bufferSize is set correctly when status is UNLOCK_STATU.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_UnlockSendSecret_001, TestSize.Level1)
{
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES);
    uint8_t opseBuffer[AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES + GCM_NONCE_BYTES] = {0x0};

    EXPECT_EQ(FBEX::UnlockSendSecret(UNLOCK_STATUS, 1, iv_.size, iv_.data, opseBuffer), 0);
}

/**
 * @tc.name: Fbex_WriteESecretToKernel_001
 * @tc.desc: Should returns -EINVAL when eBuffer is invaild.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_WriteESecretToKernel_001, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    KeyBlob eBuffer(1);

    EXPECT_EQ(FBEX::WriteESecretToKernel(userIdToFbe, UNLOCK_STATUS, eBuffer.data.get(), eBuffer.size), -EINVAL);
}

/**
 * @tc.name: Fbex_WriteESecretToKernel_002
 * @tc.desc: Should returns 0 when fopen failed with ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_WriteESecretToKernel_002, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = ENOENT;

    EXPECT_EQ(FBEX::WriteESecretToKernel(userIdToFbe, UNLOCK_STATUS, iv_.data.get(), iv_.size), 0);
}

/**
 * @tc.name: Fbex_WriteESecretToKernel_003
 * @tc.desc: Should returns -errno when fopen failed with an error other than ENOENT.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_WriteESecretToKernel_003, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(GCM_NONCE_BYTES + AES_256_HASH_RANDOM_SIZE + GCM_MAC_BYTES);

    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::WriteESecretToKernel(userIdToFbe, UNLOCK_STATUS + 1, iv_.data.get(), iv_.size), -EACCES);
}

/**
 * @tc.name: Fbex_WriteESecretToKernel_004
 * @tc.desc: Should returns -errno when ioctl failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_WriteESecretToKernel_004, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::WriteESecretToKernel(userIdToFbe, UNLOCK_STATUS, iv_.data.get(), iv_.size), -EACCES);
}

/**
 * @tc.name: Fbex_WriteESecretToKernel_005
 * @tc.desc: Should returns -errno when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_WriteESecretToKernel_005, TestSize.Level1)
{
    UserIdToFbeStr userIdToFbe;
    iv_ = KeyBlob(AES_256_HASH_RANDOM_SIZE);

    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::WriteESecretToKernel(userIdToFbe, UNLOCK_STATUS, iv_.data.get(), iv_.size), 0);
}

/**
 * @tc.name: Fbex_GetStatus_001
 * @tc.desc: Should returns -errno when fopen failed.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GetStatus_001, TestSize.Level1)
{
    EXPECT_CALL(*funcMock_, open(_, _)).WillOnce(Return(-1));
    errno = EACCES;

    EXPECT_EQ(FBEX::GetStatus(), -EACCES);
}

/**
 * @tc.name: Fbex_GetStatus_002
 * @tc.desc: Should returns -errno when ioctl success.
 * @tc.type: FUNC
 * @tc.require: AR000GK0BP
 */
HWTEST_F(FbexTest, Fbex_GetStatus_002, TestSize.Level1)
{
    MockopenSuccess();
    EXPECT_CALL(*funcMock_, ioctl(_, _)).WillOnce(Return(0));

    EXPECT_EQ(FBEX::GetStatus(), 0);
}
} // OHOS::StorageDaemon