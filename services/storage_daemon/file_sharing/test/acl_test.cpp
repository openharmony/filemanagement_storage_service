/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
#include <gtest/gtest.h>
#include "file_sharing/acl.h"

using namespace testing::ext;
using namespace OHOS::StorageDaemon;

namespace {
class AclTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AclTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AclTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AclTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AclTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: acl_entry_valid_test
 * @tc.desc: Verify that AclXattrEntry::IsValid() works as expected.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_entry_valid_test, TestSize.Level1)
{
    AclXattrEntry entry {
        .tag = ACL_TAG::UNDEFINED,
        .perm = 6,
        .id = 0,
    };
    EXPECT_TRUE(!entry.IsValid());

    entry = {
        .tag = ACL_TAG::USER,
        .perm = 5,
        .id = ACL_UNDEFINED_ID,
    };
    EXPECT_TRUE(!entry.IsValid());
    entry.id = 0;
    EXPECT_TRUE(entry.IsValid());

    entry = {
        .tag = ACL_TAG::GROUP,
        .perm = 4,
        .id = ACL_UNDEFINED_ID,
    };
    EXPECT_TRUE(!entry.IsValid());
    entry.id = 0;
    EXPECT_TRUE(entry.IsValid());
}

/**
 * @tc.name: acl_empty_test
 * @tc.desc: Verify that Acl::IsEmpty() works as expected.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_empty_test, TestSize.Level1)
{
    Acl acl;
    EXPECT_TRUE(acl.IsEmpty());

    ACL_PERM perm;
    perm.SetR();
    perm.SetE();

    /* failed insertion */
    int rc = acl.InsertEntry(
        { .tag = ACL_TAG::UNDEFINED,
          .perm = perm,
          .id = 0, }
    );
    EXPECT_TRUE(rc != 0 && acl.IsEmpty());

    /* successful insertion */
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::USER,
          .perm = perm,
          .id = 1, }
    );
    EXPECT_TRUE(rc == 0 && !acl.IsEmpty());
}

/**
 * @tc.name: acl_valid_test
 * @tc.desc: Verify that Acl::IsValid() works as expected.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_valid_test, TestSize.Level1)
{
    Acl acl;
    EXPECT_TRUE(!acl.IsValid());

    /* valid only when all necessary entries are added */
    int rc;
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::USER_OBJ,
          .perm = 6,
          .id = ACL_UNDEFINED_ID, }
    );
    EXPECT_TRUE(rc == 0 && !acl.IsValid()); // invalid
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::GROUP_OBJ,
          .perm = 6,
          .id = ACL_UNDEFINED_ID, }
    );
    EXPECT_TRUE(rc == 0 && !acl.IsValid()); // still invalid
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::OTHER,
          .perm = 0,
          .id = ACL_UNDEFINED_ID, }
    );
    EXPECT_TRUE(rc == 0 && acl.IsValid()); // valid!

    /* still valid after inserting optional entries */
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::USER,
          .perm = 4,
          .id = 0, }
    );
    EXPECT_TRUE(rc == 0 && acl.IsValid());
}

/**
 * @tc.name: acl_insert_test
 * @tc.desc: Verify that Acl::InsertEntry() works as expected.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_insert_test, TestSize.Level1)
{
    Acl acl;
    ACL_PERM perm;
    perm.SetR();
    perm.SetE();

    /* failed insertion */
    int rc = acl.InsertEntry(
            { .tag = ACL_TAG::UNDEFINED,
              .perm = perm,
              .id = 0, }
            );
    EXPECT_TRUE(rc != 0);

    /*
     * successful insertions
     * first 99 operations should succeed
     */
    constexpr size_t ENTRIES_MAX_NUM = 100; // check Acl::ENTRIES_MAX_NUM
    for (size_t i = 1; i <= (ENTRIES_MAX_NUM - 1); ++i) {
        rc = acl.InsertEntry(
                { .tag = ACL_TAG::USER,
                  .perm = perm,
                  .id = i, }
                );
        EXPECT_TRUE(rc == 0);
    }
    /* the last one should fail */
    rc = acl.InsertEntry(
            { .tag = ACL_TAG::USER,
              .perm = perm,
              .id = ENTRIES_MAX_NUM, }
            );
    EXPECT_TRUE(rc != 0);
}

int CreateValidBasicAcl(Acl &out)
{
    Acl acl;
    ACL_PERM perm;
    perm.SetR();
    perm.SetW();

    int rc = acl.InsertEntry(
        { .tag = ACL_TAG::USER_OBJ,
          .perm = perm,
          .id = ACL_UNDEFINED_ID, }
        );
    if (rc != 0) {
        return rc;
    }
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::GROUP_OBJ,
          .perm = perm,
          .id = ACL_UNDEFINED_ID, }
        );
    if (rc != 0) {
        return rc;
    }
    rc = acl.InsertEntry(
        { .tag = ACL_TAG::OTHER,
          .perm = perm,
          .id = ACL_UNDEFINED_ID, }
        );
    if (rc != 0) {
        return rc;
    }

    out = acl;
    return rc;
}

/**
 * @tc.name: acl_serialize_test
 * @tc.desc: Verify that Acl::Serialize/DeSerialize() works as expected.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_serialize_test, TestSize.Level1)
{
    Acl acl1;
    int rc = CreateValidBasicAcl(acl1);
    ASSERT_TRUE(rc == 0) << "creating basic ACL failed";

    ACL_PERM perm;
    perm.SetR();
    perm.SetW();
    constexpr size_t ENTRIES_TEST = 10;
    for (size_t i = 1; i <= ENTRIES_TEST; ++i) {
        rc = acl1.InsertEntry(
                { .tag = ACL_TAG::USER,
                  .perm = perm,
                  .id = i, }
                );
        ASSERT_TRUE(rc == 0) << "inserting USER entries failed";
    }
    for (size_t i = ENTRIES_TEST + 1; i <= ENTRIES_TEST*2; ++i) {
        rc = acl1.InsertEntry(
                { .tag = ACL_TAG::GROUP,
                  .perm = perm,
                  .id = i, }
                );
        ASSERT_TRUE(rc == 0) << "inserting GROUP entries failed";
    }

    size_t bufSize;
    char *buf = acl1.Serialize(bufSize);
    ASSERT_TRUE(buf != nullptr) << "serialization failed";

    rc = acl1.InsertEntry(
            { .tag = ACL_TAG::GROUP,
              .perm = perm,
              .id = ENTRIES_TEST*2 + 1, }
            );
    buf = acl1.Serialize(bufSize);
    ASSERT_TRUE(buf != nullptr) << "post-update serialization failed";

    Acl acl2;
    rc = acl2.DeSerialize(buf, bufSize);
    EXPECT_TRUE(rc == 0) << "de-serialization failed";
}

/**
 * @tc.name: acl_serialize_test_2
 * @tc.desc: Covering Acl::Serialize()/DeSerialize()'s exception branches.
 * @tc.type: FUNC
 * @tc.require: AR000H09ML
 */
HWTEST_F(AclTest, acl_serialize_test_2, TestSize.Level1)
{
    /* Invalid ACL for serialization */
    {
        Acl acl;
        size_t bufSize;
        char *buf = acl.Serialize(bufSize);
        EXPECT_TRUE(buf == nullptr) << "invalid ACL shouldn't serialize";
    }

    /* Invalid buffer size for Acl::DeSerialize(): case 1 */
    {
        Acl acl;
        size_t bufSize = 0;
        char *buf = nullptr;
        int rc = acl.DeSerialize(buf, bufSize);
        EXPECT_TRUE(rc != 0) << "invalid binary shouldn't de-serialized";
    }

    /* Invalid buffer size for Acl::DeSerialize(): case 2 */
    {
        Acl acl1;
        int rc = CreateValidBasicAcl(acl1);
        ASSERT_TRUE(rc == 0) << "creating basic ACL failed";

        size_t bufSize;
        char *buf = acl1.Serialize(bufSize);
        ASSERT_TRUE(buf != nullptr) << "serialization failed";

        Acl acl2;
        size_t bias = 1;
        rc = acl2.DeSerialize(buf, bufSize + bias);
        EXPECT_TRUE(rc != 0) << "de-serializing with wrong bufSize should fail";
    }
}
}
