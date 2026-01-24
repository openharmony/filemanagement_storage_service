/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_STOREGE_SERVICE_MESSAGE_PARCEL_MOCK_H
#define OHOS_STOREGE_SERVICE_MESSAGE_PARCEL_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "iremote_broker.h"
#include "message_parcel.h"

namespace OHOS::StorageManager {
class StorageMessageParcel {
public:
    virtual ~StorageMessageParcel() = default;

public:
    virtual bool WriteInterfaceToken(std::u16string name) = 0;
    virtual std::u16string ReadInterfaceToken() = 0;
    virtual bool WriteParcelable(const Parcelable *object) = 0;
    virtual bool WriteInt32(int32_t value) = 0;
    virtual int32_t ReadInt32() = 0;
    virtual bool ReadInt32(int32_t &value) = 0;
    virtual bool WriteRemoteObject(const Parcelable *object) = 0;
    virtual bool WriteRemoteObject(const sptr<IRemoteObject> &object) = 0;
    virtual sptr<IRemoteObject> ReadRemoteObject() = 0;
    virtual bool ReadBool();
    virtual bool ReadBool(bool &value) = 0;
    virtual bool WriteBool(bool value) = 0;
    virtual bool WriteString(const std::string &value) = 0;
    virtual bool WriteCString(const char *value) = 0;
    virtual bool WriteFileDescriptor(int fd) = 0;
    virtual const std::string ReadString() = 0;
    virtual bool ReadString(std::string &value) = 0;
    virtual int ReadFileDescriptor() = 0;
    virtual bool ReadStringVector(std::vector<std::string> *value) = 0;
    virtual bool ReadUint32(uint32_t &value) = 0;
    virtual bool WriteUint64(uint64_t value) = 0;
    virtual bool WriteUint16(uint16_t value) = 0;
    virtual bool ReadUint64(uint64_t &value) = 0;
    virtual bool WriteStringVector(const std::vector<std::string> &val) = 0;
    virtual bool WriteUint32(uint32_t value) = 0;
    virtual bool WriteRawData(const void *data, size_t size) = 0;
    virtual const void *ReadRawData(size_t size) = 0;
    virtual uint32_t ReadUint32() = 0;
    virtual uint32_t ReadUint16() = 0;
    virtual const char *ReadCString() = 0;
    virtual bool ReadInt64Vector(std::vector<int64_t> *val) = 0;
    virtual bool WriteUint8(uint8_t value) = 0;
    virtual bool ParseFrom(uintptr_t data, size_t size) = 0;

public:
    static inline std::shared_ptr<StorageMessageParcel> messageParcel = nullptr;
};

class MessageParcelMock : public StorageMessageParcel {
public:
    MOCK_METHOD1(WriteInterfaceToken, bool(std::u16string name));
    MOCK_METHOD0(ReadInterfaceToken, std::u16string());
    MOCK_METHOD1(WriteParcelable, bool(const Parcelable *object));
    MOCK_METHOD1(WriteInt32, bool(int32_t value));
    MOCK_METHOD0(ReadInt32, int32_t());
    MOCK_METHOD1(ReadInt32, bool(int32_t &value));
    MOCK_METHOD1(WriteRemoteObject, bool(const Parcelable *object));
    MOCK_METHOD1(WriteRemoteObject, bool(const sptr<IRemoteObject> &object));
    MOCK_METHOD0(ReadRemoteObject, sptr<IRemoteObject>());
    MOCK_METHOD0(ReadBool, bool());
    MOCK_METHOD1(ReadBool, bool(bool &value));
    MOCK_METHOD1(WriteBool, bool(bool value));
    MOCK_METHOD1(WriteString, bool(const std::string &value));
    MOCK_METHOD1(WriteCString, bool(const char *value));
    MOCK_METHOD1(WriteFileDescriptor, bool(int fd));
    MOCK_METHOD0(ReadString, const std::string());
    MOCK_METHOD1(ReadString, bool(std::string &value));
    MOCK_METHOD0(ReadFileDescriptor, int());
    MOCK_METHOD1(ReadStringVector, bool(std::vector<std::string> *value));
    MOCK_METHOD1(ReadUint32, bool(uint32_t &value));
    MOCK_METHOD1(WriteUint64, bool(uint64_t value));
    MOCK_METHOD1(WriteUint16, bool(uint16_t value));
    MOCK_METHOD1(ReadUint64, bool(uint64_t &value));
    MOCK_METHOD1(WriteStringVector, bool(const std::vector<std::string> &val));
    MOCK_METHOD1(WriteUint32, bool(uint32_t value));
    MOCK_METHOD2(WriteRawData, bool(const void *data, size_t size));
    MOCK_METHOD1(ReadRawData, const void *(size_t size));
    MOCK_METHOD0(ReadUint32, uint32_t());
    MOCK_METHOD0(ReadUint16, uint32_t());
    MOCK_METHOD0(ReadCString, const char *());
    MOCK_METHOD1(ReadInt64Vector, bool(std::vector<int64_t> *val));
    MOCK_METHOD1(WriteUint8, bool(uint8_t value));
    MOCK_METHOD2(ParseFrom, bool(uintptr_t data, size_t size));
};
} // namespace OHOS::StorageManager
#endif