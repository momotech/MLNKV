//
//  MLNKVBase.hpp
//  MLNKV
//
//
//  Copyright © 2019 . All rights reserved.
//

#ifndef MLNKVManager_hpp
#define MLNKVManager_hpp

#include <stdio.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <mutex>

#include "MLNKVType.h"
#include "MLNKVValueInfo.hpp"


typedef enum : uint8_t {
    MLNKVReusedAvailableStatusNone = 0,
    MLNKVReusedAvailableStatusReused = 1,
    MLNKVReusedAvailableStatusAppend = 2,
} MLNKVReusedAvailableStatus;

class MLNKVBase {
    char *filePath;
    size_t fileSize;
    size_t usedSize;
    size_t unavailabledSize;
    int file;
    char *filemmap;
    std::unordered_map <std::string, MLNKVValueInfo> keysMap;
    std::unordered_map <size_t, std::unordered_set<size_t>> unavailablesMap;
    
    std::mutex mutex;
    
    bool loadDataFromFile(bool openFile = true);
    bool isFileValid();
    bool ensureMemorySize(size_t newSize);
    bool reuseMemoryOffset(size_t size, size_t &offset);
    bool sortUnavailableMemory(bool force);
    void markUnAvailable(MLNKVValueInfo &valueInfo);
    
    bool writeUInt32(uint32_t value, MLNKVValueType valueType, const std::string &key);
    bool writeUInt64(uint64_t value, MLNKVValueType valueType, const std::string &key);
    MLNKVReusedAvailableStatus getAvailableOffset(size_t dataSize, size_t &offset);
    
    bool removeValue(const std::string &key);
    bool removeValue(const std::string &key, std::unordered_map<std::string, MLNKVValueInfo>::iterator &valueItera, bool returnValue);
    bool clear();
    
public:
    MLNKVBase(const std::string &path);
    ~MLNKVBase();
    
    // write bytes
    bool writeBytes(const void *value, const size_t size, MLNKVValueType valueType, const std::string &key);
    // read bytes
    bool readValueBytes(const std::string &key, MLNKVValueType &valueType, void* &value, size_t &size);
    
    // set
    
    bool setBytes(const void *value, const size_t size, const std::string &key);
    
    bool setString(const std::string &value, const std::string &key);
    
    bool setBool(bool value, const std::string &key);
    
    bool setInt32(int32_t value, const std::string &key);
    
    bool setInt64(int64_t value, const std::string &key);
    
    bool setFloat(float value, const std::string &key);
    
    bool setDouble(double value, const std::string &key);
    
    // get
    
    bool getBytesForKey(const std::string &key, void* &value, size_t &size);
    
    bool getStringForKey(const std::string &key, std::string &result);
    
    bool getBoolForKey(const std::string &key, bool defaultValue = false);
    
    int32_t getInt32ForKey(const std::string &key, int32_t defaultValue = 0);
    
    int64_t getInt64ForKey(const std::string &key, int64_t defaultValue = 0);
    
    float getFloatForKey(const std::string &key, float defaultValue = 0);
    
    double getDoubleForKey(const std::string &key, double defaultValue = 0);
    
    
    // size
    size_t getValueSizeForKey(const std::string &key, MLNKVValueType &valueType);
    
    bool containsKey(const std::string &key);
    
    size_t count();
    
    size_t totalUsedSize();
    
    std::vector<std::string> allKeys();
    
    
    // remove
    bool remove(const std::string &key);
    void clearAll();
    void trim();
    
    // sync
    void sync(bool isSync);
    
    //
    std::string getFilePath();
    size_t getFileSize();

};

#endif /* MLNKVManager_hpp */
