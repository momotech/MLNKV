//
//  MLNKVValueInfo.hpp
//  MLNKV
//
//

#ifndef MLNKVValueInfo_hpp
#define MLNKVValueInfo_hpp

#include <stdio.h>
#include "MLNKVType.h"


class MLNKVValueInfo {
    
public:
    
    MLNKVValueType type;
    size_t offset;
    size_t extSize;
    size_t keySize;
    size_t valueSize;
    
    MLNKVValueInfo(MLNKVValueType type = MLNKVValueType_None): type(type) {}
    
    MLNKVValueInfo(MLNKVValueInfo &&other)noexcept :
    type(other.type),
    offset(other.offset),
    extSize(other.extSize),
    keySize(other.keySize),
    valueSize(other.valueSize) {
        other.type = MLNKVValueType_None;
        other.offset = 0;
        other.extSize = 0;
        other.keySize = 0;
        other.valueSize = 0;
    }
    MLNKVValueInfo &operator=(MLNKVValueInfo &&other) noexcept {
        std::swap(type, other.type);
        std::swap(offset, other.offset);
        std::swap(extSize, other.extSize);
        std::swap(keySize, other.keySize);
        std::swap(valueSize, other.valueSize);
        return *this;
    }
    
    ~MLNKVValueInfo() {}
    
private:
    MLNKVValueInfo(const MLNKVValueInfo &other) = delete;
    MLNKVValueInfo &operator=(const MLNKVValueInfo &other) = delete;
    
};

#endif /* MLNKVValueInfo_hpp */
