//
//  MLNKVBase.cpp
//  MLNKV
//
//  Copyright © 2019 . All rights reserved.
//

#include "MLNKVBase.h"
#include "MLNKVUtility.h"
#include "MLNKVLog.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <set>
#include <assert.h>

#define MLNKVLock_mutex_lock(rLock) lock_guard<std::mutex> lock(rLock)

//通用工具
static inline bool mln_is_file_exist(const char *filePath) {
    if (strlen(filePath) == 0) {
        return false;
    }

    struct stat temp;
    return lstat(filePath, &temp) == 0;
}

static inline void mln_kv_mkdirs(const char *muldir) {
    size_t i,len;
    len = strlen(muldir);
    char str[len];
    strncpy(str, muldir, len);

    for(i = 1; i < len; i++) {
        if(str[i] == '/' ) {
            str[i] = '\0';
            if(access(str, 0) != 0) {
                if (strlen(str) == len) {
                    str[i]='/';
                    return;
                }
                mkdir(str, 0777);
            }
            str[i]='/';
        }
    }
}

static inline bool mln_create_file(const char *filePath) {
    if (strlen(filePath) == 0) {
        return false;
    }
    mln_kv_mkdirs(filePath);
    FILE *fp = fopen(filePath, "w+");
    bool success = fp != NULL;
    if (fp) {
        fclose(fp);
    }
    return success;
}

//static inline bool mln_remove_file(const char *filePath) {
//    if (strlen(filePath) == 0) {
//        return false;
//    }
//
//    int ret = unlink(filePath);
//    return ret == 0;
//}

/**
 *length 3
 *1 符号位 | 2 keylength bytes | 3 valuelength bytes
 */
static const uint16_t kMLNKVExtendLength = 3;

//Class
using namespace std;

MLNKVBase::~MLNKVBase() {
    if (filemmap != MAP_FAILED && filemmap != nullptr) {
        munmap(filemmap, fileSize);
        filemmap = nullptr;
    }
    if (file > 0) {
        close(file);
        file = -1;
    }
    keysMap.clear();
    unavailablesMap.clear();

    if (filePath != nullptr) {
        free(filePath);
        filePath = nullptr;
    }
}

MLNKVBase::MLNKVBase(const std::string &path):
        filePath(nullptr),
        fileSize(0),
        file(-1),
        filemmap(nullptr),
        usedSize(0),
        unavailabledSize(0) {
    assert(!path.empty());
    if (path.empty()) {
        MLNKVError("file path is nil...");
        return;
    }

    MLNKVLock_mutex_lock(mutex);

    filePath = (char *)malloc(sizeof(char) * (path.length() +1));
    std::strcpy(filePath, path.c_str());

    if (!mln_is_file_exist(filePath)) {
        if (!mln_create_file(filePath)) {
            MLNKVError("create file error: %s", strerror(errno));
            return;
        }
    }
    this->loadDataFromFile();
}

#pragma mark - set
bool MLNKVBase::setBytes(const void *value, const size_t size, const std::string &key) {
    return this->writeBytes(value, size, MLNKVValueType_Bytes, key);
}

bool MLNKVBase::setString(const std::string &value, const std::string &key) {
    return this->writeBytes(value.c_str(), value.length(), MLNKVValueType_String, key);
}

bool MLNKVBase::setBool(bool value, const std::string &key) {
    return this->writeUInt32((uint32_t)(value == true?1:0), MLNKVValueType_Bool, key);
}

bool MLNKVBase::setInt32(int32_t value, const std::string &key) {
    return this->writeUInt32(MLNKVInt32ToUInt32(value), MLNKVValueType_Int32, key);
}

bool MLNKVBase::setInt64(int64_t value, const std::string &key) {
    return this->writeUInt64(MLNKVInt64ToUInt64(value), MLNKVValueType_Int64, key);
}

bool MLNKVBase::setFloat(float value, const std::string &key) {
    return this->writeUInt32(MLNKVFloatToUInt32(value), MLNKVValueType_Float, key);
}

bool MLNKVBase::setDouble(double value, const std::string &key) {
    return this->writeUInt64(MLNKVDoubleToUInt64(value), MLNKVValueType_Double, key);
}

#pragma mark - get
bool MLNKVBase::getBytesForKey(const std::string &key, void* &value, size_t &size) {
    MLNKVValueType valueType = MLNKVValueType_None;
    return this->getValueBytes(key, valueType, value, size);
}

bool MLNKVBase::getStringForKey(const std::string &key, std::string &result) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        char keys[size + 1];
        memcpy(keys, value, size);
        keys[size] = '\0';
        result = string(keys);
        return true;
    }
    return false;
}

bool MLNKVBase::getBoolForKey(const std::string &key, bool defaultValue) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        int16_t va = ((uint8_t *)value)[0];
        return va == 1;
    }
    return defaultValue;
}

int32_t MLNKVBase::getInt32ForKey(const std::string &key, int32_t defaultValue) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        uint32_t va = MLNKVReadUInt32((uint8_t *)value, size);
        return MLNKVUInt32ToInt32(va);
    }
    return defaultValue;
}

int64_t MLNKVBase::getInt64ForKey(const std::string &key, int64_t defaultValue) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        uint64_t va = MLNKVReadUInt64((uint8_t *)value, size);
        return MLNKVUInt64ToInt64(va);
    }
    return defaultValue;
}

float MLNKVBase::getFloatForKey(const std::string &key, float defaultValue) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        uint32_t va = MLNKVReadUInt32((uint8_t *)value, size);
        return MLNKVUInt32ToFloat(va);
    }
    return defaultValue;
}

double MLNKVBase::getDoubleForKey(const std::string &key, double defaultValue) {
    MLNKVValueType valueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, valueType, value, size)) {
        uint64_t va = MLNKVReadUInt64((uint8_t *)value, size);
        return MLNKVUInt64ToDouble(va);
    }
    return defaultValue;
}

#pragma mark - size

size_t MLNKVBase::getValueSizeForKey(const std::string &key, MLNKVValueType &valueType) {
    MLNKVValueType tempValueType = MLNKVValueType_None;
    void *value;
    size_t size = 0;
    if (this->getValueBytes(key, tempValueType, value, size)) {
        valueType = tempValueType;
        return size;
    }
    valueType = MLNKVValueType_None;
    return 0;
}

bool MLNKVBase::containsKey(const std::string &key) {
    MLNKVLock_mutex_lock(mutex);
    auto itera = keysMap.find(key);
    return itera != keysMap.end();
}

vector<string> MLNKVBase::allKeys() {
    MLNKVLock_mutex_lock(mutex);
    vector<string> keys;
    for(const auto &kv : keysMap) {
        keys.push_back(kv.first);
    }
    return keys;
}

size_t MLNKVBase::count() {
    MLNKVLock_mutex_lock(mutex);
    return keysMap.size();
}

size_t MLNKVBase::totalUsedSize() {
    MLNKVLock_mutex_lock(mutex);
    return usedSize;
}

#pragma mark - mark

bool MLNKVBase::remove(const std::string &key) {
    assert(key.empty() == false);
    if (key.empty()) {
        MLNKVError("key can't be empty...");
        return false;
    }

    MLNKVLock_mutex_lock(mutex);

    if (!this->isFileValid()) {
        MLNKVError("[%s] file is not valid...", filePath);
        return false;
    }

    return this->removeValue(key);
}

void MLNKVBase::sync(bool isSync) {

    MLNKVLock_mutex_lock(mutex);

    if (!this->isFileValid()) {
        return;
    }
    auto flag = isSync ? MS_SYNC : MS_ASYNC;
    if (msync(filemmap, usedSize, flag) != 0) {
        MLNKVError("fail to msync[%d] to file %s:%s", flag, filePath, strerror(errno));
    }
}

void MLNKVBase::clearAll() {
    MLNKVLock_mutex_lock(mutex);
    clear();
}

void MLNKVBase::trim() {
    MLNKVLock_mutex_lock(mutex);
    if (usedSize == 0) {
        clear();
        return;
    }else if (fileSize <= getpagesize()) {
        return;
    }

    this->sortUnavailableMemory(true);

    auto oldSize = fileSize;
    while (fileSize > usedSize * 2) {
        fileSize /= 2;
    }
    if (oldSize == fileSize) {
        MLNKVLog("there's no need to trim %s with size %zu, usedSize %zu", filePath, fileSize, usedSize);
        return;
    }

    if (ftruncate(file, fileSize) != 0) {
        MLNKVError("fail to truncate [%s] to size %zu, %s", filePath, fileSize, strerror(errno));
        fileSize = oldSize;
        return;
    }
    if (munmap(filemmap, oldSize) != 0) {
        MLNKVError("fail to munmap [%s], %s", filePath, strerror(errno));
    }
    filemmap = (char *)mmap(filemmap, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    if (filemmap == MAP_FAILED) {
        MLNKVError("fail to mmap [%s], %s", filePath, strerror(errno));
    }
}

string MLNKVBase::getFilePath() {
    return std::string(filePath);
}

size_t MLNKVBase::getFileSize() {
    MLNKVLock_mutex_lock(mutex);
    return fileSize;
}

// private

#pragma mark - write private
bool MLNKVBase::writeBytes(const void *value, const size_t size, MLNKVValueType valueType, const std::string &key) {
    assert(!key.empty());
    if (key.empty()) {
        MLNKVError("key can't be empty...");
        return false;
    }

    MLNKVLock_mutex_lock(mutex);

    if (!this->isFileValid()) {
        MLNKVError("[%s] file is not valid...", filePath);
        return false;
    }

    if (size == 0 || value == nullptr) {
        return this->removeValue(key);
    }
    this->removeValue(key);

    uint32_t keyLength = static_cast<uint32_t>(key.length());
    int16_t keyLengthSize = MLNKVRawVarUInt32Size(keyLength);
    int16_t valueLengthSize = MLNKVRawVarUInt64Size(size);

    size_t segmentSize = kMLNKVExtendLength + keyLengthSize + keyLength + valueLengthSize + size;
    size_t offset = 0;
    MLNKVReusedAvailableStatus availableStatus = this->getAvailableOffset(segmentSize, offset);
    if (availableStatus > MLNKVReusedAvailableStatusNone) {
        MLNKVValueInfo obj(valueType);
        obj.offset = offset;
        obj.extSize = static_cast<size_t>(kMLNKVExtendLength + keyLengthSize +valueLengthSize);
        obj.keySize = keyLength;
        obj.valueSize = size;
        keysMap[key] = std::move(obj);

        filemmap[offset ++] = valueType;
        filemmap[offset ++] = static_cast<char>(keyLengthSize);
        filemmap[offset ++] = static_cast<char>(valueLengthSize);
        MLNKVWriteUInt32((uint8_t *)filemmap + offset, keyLength, keyLengthSize);
        offset += keyLengthSize;
        MLNKVWriteUInt64((uint8_t *)filemmap + offset, size, valueLengthSize);
        offset += valueLengthSize;
        memcpy(filemmap + offset, key.c_str(), keyLength);
        offset += keyLength;
        memcpy(filemmap + offset, value, size);
        offset += size;

        if (availableStatus == MLNKVReusedAvailableStatusAppend) {
            usedSize = offset;
            if (offset < fileSize - 1) {
                memset(filemmap + offset, MLNKVValueType_None, 1);
            }
        }
        return true;
    }
    MLNKVError("can't set value for key:%s",key.c_str());
    return false;
}



bool MLNKVBase::writeUInt32(uint32_t value, MLNKVValueType valueType, const std::string &key) {
    int16_t valueSize = MLNKVRawVarUInt32Size(value);
    uint8_t valuePtr[valueSize];
    MLNKVWriteUInt32(valuePtr, value, valueSize);
    return this->writeBytes(valuePtr, static_cast<size_t>(valueSize), valueType, key);
}

bool MLNKVBase::writeUInt64(uint64_t value, MLNKVValueType valueType, const std::string &key) {
    int16_t valueSize = MLNKVRawVarUInt64Size(value);
    uint8_t valuePtr[valueSize];
    MLNKVWriteUInt64(valuePtr, value, valueSize);
    return this->writeBytes(valuePtr,  static_cast<size_t>(valueSize), valueType, key);
}

MLNKVReusedAvailableStatus MLNKVBase::getAvailableOffset(size_t dataSize, size_t &offset) {
    if (this->reuseMemoryOffset(dataSize, offset)) {
        return MLNKVReusedAvailableStatusReused;
    }
    if (this->ensureMemorySize(dataSize)) {
        offset = usedSize;
        return MLNKVReusedAvailableStatusAppend;
    }
    return MLNKVReusedAvailableStatusNone;
}

#pragma mark - get private
bool MLNKVBase::getValueBytes(const std::string &key, MLNKVValueType &valueType, void* &value, size_t &size) {
    assert(key.empty() == false);
    if (key.empty()) {
        MLNKVError("key can't be empty...");
        return false;
    }

    MLNKVLock_mutex_lock(mutex);

    if (!this->isFileValid()) {
        MLNKVError("[%s] file is not valid...", filePath);
        return false;
    }
    auto itera = keysMap.find(key);
    if (itera == keysMap.end()) {
        MLNKVError("can't find key [%s]", key.c_str());
        return false;
    }
    auto& obj = itera->second;
    valueType = obj.type;
    size = obj.valueSize;
    value = filemmap + obj.offset + obj.extSize + obj.keySize;
    return size != 0;
}

#pragma mark - remove
bool MLNKVBase::removeValue(const std::string &key) {
    auto itera = keysMap.find(key);
    if (itera != keysMap.end()) {
        auto& obj = itera->second;
        this->markUnAvailable(obj.offset);

        size_t segmentSize = obj.extSize + obj.keySize + obj.valueSize;
        unordered_map<size_t, unordered_set<size_t>>::iterator segmentItera = unavailablesMap.find(segmentSize);

        if (segmentItera != unavailablesMap.end()) {
            (segmentItera->second).insert(obj.offset);
        }else {
            unordered_set<size_t> keySet = {obj.offset};
            unavailablesMap[segmentSize] = keySet;
        }
        unavailabledSize += segmentSize;

        keysMap.erase(key);
    }
    return true;
}

bool MLNKVBase::clear() {
    size_t pageSize = getpagesize();
    if (filemmap != nullptr && filemmap != MAP_FAILED) {
        size_t size = std::min<size_t>(pageSize, fileSize);
        memset(filemmap, MLNKVValueType_None, size);
        if (msync(filemmap, size, MS_SYNC) != 0) {
            MLNKVError("fail to msync [%s], %s", filePath, strerror(errno));
        }
        if (munmap(filemmap, fileSize) != 0) {
            MLNKVError("fail to munmap [%s], %s", filePath, strerror(errno));
        }
    }
    if (file >= 0) {
        if (fileSize != pageSize) {
            if (ftruncate(file, pageSize) != 0) {
                MLNKVError("fail to truncate [%s] to size %d, %s", filePath, pageSize, strerror(errno));
            }
        }
        if (close(file) != 0) {
            MLNKVError("fail to close [%s], %s", filePath, strerror(errno));
        }
    }

    this->loadDataFromFile();
    return true;
}

#pragma mark - private

bool MLNKVBase::loadDataFromFile(bool openFile) {

    usedSize = 0;
    unavailabledSize = 0;
    keysMap.clear();
    unavailablesMap.clear();

    if (openFile) {
        file = -1;
        filemmap = nullptr;
        fileSize = 0;

        file = open(filePath, O_RDWR|O_CREAT, S_IRWXU);
        if (file < 0) {
            MLNKVError("fail to open: %s, %s", filePath, strerror(errno));
            return false;
        }

        struct stat st = {};
        if (fstat(file, &st) != -1) {
            fileSize = (size_t) st.st_size;
        }

        int pageSize = getpagesize();
        if (fileSize < pageSize || (fileSize % pageSize != 0)) {
            fileSize = ((fileSize / pageSize) + 1) * pageSize;
            if (ftruncate(file, fileSize) != 0) {
                MLNKVError("fail to truncate file:%s to size %zi, %s", filePath, fileSize, strerror(errno));
                fileSize = (size_t) st.st_size;
                return false;
            }
        }

        filemmap = (char *)mmap(nullptr, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
        if (filemmap == MAP_FAILED) {
            MLNKVError("fail to mmap file:%s, %s", filePath, strerror(errno));
            close(file);
            return false;
        }
    }

    size_t offset = 0;
    size_t availableOffset = 0;
    size_t offsetMaxSize = fileSize - kMLNKVExtendLength - 2;

    uint8_t *filemmapPtr = (uint8_t *)filemmap;
    do {
        size_t tempOffset = offset;
        MLNKVValueType valid = (MLNKVValueType)filemmap[tempOffset ++];
        if (valid < MLNKVValueType_Used || valid > MLNKVValueType_Bytes) {
            break;
        }
        uint16_t keyLengthSize = filemmapPtr[tempOffset ++];
        uint16_t valueLengthSize = filemmapPtr[tempOffset ++];

        if (keyLengthSize == 0 ||
            valueLengthSize == 0 ||
            keyLengthSize > MLNKV32Size ||
            valueLengthSize > MLNKV64Size ||
            (tempOffset + keyLengthSize + valueLengthSize) >= fileSize) {
            MLNKVError("read file[%s] data key/value size error...", filePath);
            break;
        }
        uint32_t keyLength = MLNKVReadUInt32(filemmapPtr + tempOffset, keyLengthSize);
        tempOffset += keyLengthSize;
        uint64_t valueLength = MLNKVReadUInt64(filemmapPtr + tempOffset, valueLengthSize);
        tempOffset += valueLengthSize;

        if (keyLength == 0 ||
            valueLength == 0 ||
            (tempOffset + keyLength + valueLength) >= fileSize) {
            MLNKVError("read file[%s] data key/value length error...", filePath);
            break;
        }

        //数据错误检验 - 基本数据类型长度
        if (valid >= MLNKVValueType_Bool && valid <= MLNKVValueType_Int64) {
            if (valueLengthSize != 1 ||
                (valid <= MLNKVValueType_Int32 && valueLength > MLNKV32Size) ||
                (valid >= MLNKVValueType_Double && valueLength > MLNKV64Size)) {
                MLNKVError("verify file[%s] value type[%d] length[%d]  ", filePath, valid, valueLength);
                break;
            }
        }

        size_t segmentSize = static_cast<size_t>(kMLNKVExtendLength + keyLengthSize + valueLengthSize + keyLength + valueLength);
        if (valid != MLNKVValueType_Used) {
            char keys[keyLength + 1];
            memcpy(keys, filemmap + tempOffset, keyLength);
            keys[keyLength] = '\0';
            string key = string(keys);

            MLNKVValueInfo obj(valid);
            obj.offset = availableOffset;
            obj.extSize = static_cast<size_t>(kMLNKVExtendLength + keyLengthSize + valueLengthSize);
            obj.keySize = static_cast<size_t>(keyLength);
            obj.valueSize = static_cast<size_t>(valueLength);
            keysMap[key] = std::move(obj);

            if (availableOffset != offset) { // <<
                memcpy(filemmap + availableOffset, filemmap + offset, segmentSize);
            }

            // real offset
            availableOffset += segmentSize;
        }
        offset += segmentSize;
    } while (offset < offsetMaxSize);

    usedSize = availableOffset;

    if (usedSize != offset && fileSize > usedSize) {
        memset(filemmap + usedSize, MLNKVValueType_None, 1);
    }

    MLNKVLog("map size:%d", keysMap.size());

    return true;
}

bool MLNKVBase::isFileValid() {
    if (file > 0 && fileSize > 0 && filemmap != nullptr && filemmap != MAP_FAILED) {
        return true;
    }
    return false;
}

bool MLNKVBase::ensureMemorySize(size_t newSize) {
    if (!this->isFileValid()) {
        MLNKVError("[%s] file is not valid...", filePath);
        return false;
    }

    if (newSize > (fileSize - usedSize)) {
        if (newSize <= unavailabledSize && this->sortUnavailableMemory(false)) {
            return true;
        }
        size_t oldSize = fileSize;
        do {
            fileSize *= 2;
        } while (newSize > (fileSize - usedSize));
        MLNKVLog("extending [%s] file size from %zu to %zu, for new size %zu", filePath, oldSize, fileSize, newSize);

        if (ftruncate(file, fileSize) != 0) {
            MLNKVError("fail to truncate [%s] to size %zu, %s", filePath, fileSize, strerror(errno));
            fileSize = oldSize;
            return false;
        }

        if (munmap(filemmap, oldSize) != 0) {
            MLNKVError("fail to munmap [%@], %s", filePath, strerror(errno));
        }
        filemmap = (char *)mmap(filemmap, fileSize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
        if (filemmap == MAP_FAILED) {
            MLNKVError("fail to mmap file:%s, %s", filePath, strerror(errno));
        }
        if (!this->isFileValid()) {
            MLNKVError("[%s] file is not valid...", filePath);
            return false;
        }
    }
    return true;
}


bool MLNKVBase::markUnAvailable(size_t offset) {
    filemmap[offset] = MLNKVValueType_Used;
    return true;
}

bool MLNKVBase::reuseMemoryOffset(size_t size, size_t &offset) {
    if (size <= 0 || unavailabledSize < size) {
        return false;
    }
    unordered_map<size_t, unordered_set<size_t>>::iterator segmentItera = unavailablesMap.find(size);
    if (segmentItera == unavailablesMap.end()) {
        return false;
    }
    if (segmentItera->second.empty()) {
        return false;
    }
    size_t oset = *segmentItera->second.begin();
    if (filemmap[oset] != MLNKVValueType_Used) {
        return false;
    }
    segmentItera->second.erase(oset);
    unavailabledSize -= size;
    offset = oset;
    return true;
}

bool MLNKVBase::sortUnavailableMemory(bool force) {
    if (unavailabledSize == 0) {
        return false;
    }
    if (!force && unavailabledSize < getpagesize()) {
        return false;
    }
    return this->loadDataFromFile(false);
}

