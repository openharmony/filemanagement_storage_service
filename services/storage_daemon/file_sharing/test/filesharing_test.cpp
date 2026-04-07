/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable by law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <sys/xattr.h>

#include "file_sharing/acl.h"
#include "file_sharing/file_sharing.h"
#include "parameter.h"
#include "init_param.h"
#include "utils/file_utils.h"

namespace OHOS::StorageManager {
class IParamMoc {
public:
    virtual ~IParamMoc() = default;
    virtual int GetParameter(const char *key, const char *def, char *value, uint32_t len) = 0;
    static inline std::shared_ptr<IParamMoc> paramMoc = nullptr;
};
class ParamMoc : public IParamMoc {
public:
    MOCK_METHOD4(GetParameter, int(const char *key, const char *def, char *value, uint32_t len));
};
}

int GetParameter(const char *key, const char *def, char *value, uint32_t len)
{
    if (OHOS::StorageManager::IParamMoc::paramMoc != nullptr) {
        return OHOS::StorageManager::IParamMoc::paramMoc->GetParameter(key, def, value, len);
    }
    
    static int defaultCallCount = 0;
    defaultCallCount++;
    std::cout << "GetParameter called without mock (call " << defaultCallCount << ")" << std::endl;
    
    if (defaultCallCount == 1) {
        return -1;
    }
    return -1;
}

namespace OHOS::StorageManager {

using namespace testing::ext;
using namespace testing;
using namespace OHOS::StorageDaemon;

namespace {
constexpr const char *FILE_SHARING_DIR = "/data/service/el1/public/storage_daemon/share";
constexpr const char *PUBLIC_DIR = "/data/service/el1/public/storage_daemon/share/public";
constexpr const char *SHARE_TOB_DIR = "/data/service/el1/public/storage_daemon/share_tob";
constexpr const char *TOB_SCENE = "2b_share";
constexpr const char *TOC_SCENE = "2c_share";
constexpr const char *TOD_SCENE = "2c2b_share";

class FileSharingTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
    
    std::shared_ptr<ParamMoc> paramMock_ = nullptr;
};

void FileSharingTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase Start";
}

void FileSharingTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase Start";
}

void FileSharingTest::SetUp(void)
{
    paramMock_ = std::make_shared<ParamMoc>();
    IParamMoc::paramMoc = paramMock_;
}

void FileSharingTest::TearDown(void)
{
    IParamMoc::paramMoc = nullptr;
    paramMock_.reset();
}

} // namespace

/**
 * @tc.name: FileSharingTest_001
 * @tc.desc: Verify that PrepareFileSharingDir() and SetupDirAcl() work as expected for TOB_SCENE
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_001 starts";

    int rc = PrepareFileSharingDir(TOB_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(SHARE_TOB_DIR));

    rc = SetupDirAcl(TOB_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(SHARE_TOB_DIR, Acl::ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(SHARE_TOB_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_001 ends";
}

/**
 * @tc.name: FileSharingTest_002
 * @tc.desc: Verify that PrepareFileSharingDir() and SetupDirAcl() work as expected for TOC_SCENE
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_002 starts";

    int rc = PrepareFileSharingDir(TOC_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(PUBLIC_DIR));
    EXPECT_TRUE(IsDir(FILE_SHARING_DIR));

    rc = SetupDirAcl(TOC_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(PUBLIC_DIR, Acl::ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(PUBLIC_DIR));
    EXPECT_TRUE(RmDirRecurse(FILE_SHARING_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_002 ends";
}

/**
 * @tc.name: FileSharingTest_003
 * @tc.desc: Verify that PrepareFileSharingDir() and SetupDirAcl() work as expected for TOD_SCENE
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_003 starts";

    int rc = PrepareFileSharingDir(TOD_SCENE);
    EXPECT_EQ(rc, 0);

    EXPECT_TRUE(IsDir(SHARE_TOB_DIR));
    EXPECT_TRUE(IsDir(PUBLIC_DIR));
    EXPECT_TRUE(IsDir(FILE_SHARING_DIR));

    rc = SetupDirAcl(TOD_SCENE);
    EXPECT_EQ(rc, 0);
    EXPECT_TRUE(getxattr(SHARE_TOB_DIR, Acl::ACL_XATTR_DEFAULT, nullptr, 0) > 0);
    EXPECT_TRUE(getxattr(PUBLIC_DIR, Acl::ACL_XATTR_DEFAULT, nullptr, 0) > 0);

    EXPECT_TRUE(RmDirRecurse(SHARE_TOB_DIR));
    EXPECT_TRUE(RmDirRecurse(PUBLIC_DIR));
    EXPECT_TRUE(RmDirRecurse(FILE_SHARING_DIR));

    GTEST_LOG_(INFO) << "FileSharingTest_003 ends";
}

/**
 * @tc.name: FileSharingTest_004
 * @tc.desc: Verify SetupFileSharingDir covers return branches and creates expected dirs by scene.
 * @tc.type: FUNC
 */
HWTEST_F(FileSharingTest, file_sharing_test_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSharingTest_004 starts";

    int rc = SetupFileSharingDir();
    EXPECT_EQ(rc, 0);

    std::string fsShareParam = GetFileShareDefineParameter();
    if (fsShareParam == TOB_SCENE || fsShareParam == TOD_SCENE) {
        EXPECT_TRUE(IsDir(SHARE_TOB_DIR));
        EXPECT_TRUE(RmDirRecurse(SHARE_TOB_DIR));
    }
    if (fsShareParam == TOC_SCENE || fsShareParam == TOD_SCENE) {
        EXPECT_TRUE(IsDir(PUBLIC_DIR));
        EXPECT_TRUE(IsDir(FILE_SHARING_DIR));
        EXPECT_TRUE(RmDirRecurse(PUBLIC_DIR));
        EXPECT_TRUE(RmDirRecurse(FILE_SHARING_DIR));
    }

    GTEST_LOG_(INFO) << "FileSharingTest_004 ends";
}
}