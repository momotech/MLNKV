//
//  MLNKVUtility.hpp
//  MLNKV
//
//  Copyright © 2019 . All rights reserved.
//

#ifndef MLNKVUtility_hpp
#define MLNKVUtility_hpp

#include <stdio.h>
#include <cstdint>


template <typename T, typename P>
union MLNKVConverter {
    static_assert(sizeof(T) == sizeof(P), "size not match");
    T first;
    P second;
};

static inline uint64_t MLNKVInt64ToUInt64(int64_t v) {
    MLNKVConverter<int64_t, uint64_t> converter;
    converter.first = v;
    return converter.second;
}

static inline int64_t MLNKVUInt64ToInt64(uint64_t v) {
    MLNKVConverter<int64_t, uint64_t> converter;
    converter.second = v;
    return converter.first;
}

static inline uint32_t MLNKVInt32ToUInt32(int32_t v) {
    MLNKVConverter<int32_t, uint32_t> converter;
    converter.first = v;
    return converter.second;
}

static inline int32_t MLNKVUInt32ToInt32(uint32_t v) {
    MLNKVConverter<int32_t, uint32_t> converter;
    converter.second = v;
    return converter.first;
}

static inline uint32_t MLNKVFloatToUInt32(float v){
    MLNKVConverter<float, uint32_t> converter;
    converter.first = v;
    return converter.second;
}

static inline float MLNKVUInt32ToFloat(uint32_t v) {
    MLNKVConverter<uint32_t, float> converter;
    converter.first = v;
    return converter.second;
}

static inline uint64_t MLNKVDoubleToUInt64(double v) {
    MLNKVConverter<double, uint64_t> converter;
    converter.first = v;
    return converter.second;
}

static inline double MLNKVUInt64ToDouble(uint64_t v) {
    MLNKVConverter<uint64_t, double> converter;
    converter.first = v;
    return converter.second;
}


static inline uint8_t MLNKVRawVarUInt32Size(uint32_t value) {
    if ((value & (0xffffffff << 8)) == 0) {
        return 1;
    } else if ((value & (0xffffffff << 16)) == 0) {
        return 2;
    } else if ((value & (0xffffffff << 24)) == 0) {
        return 3;
    }
    return 4;
}

static inline uint8_t MLNKVRawVarUInt64Size(uint64_t value) {
    if ((value & (0xffffffffffffffffL << 8)) == 0) {
        return 1;
    } else if ((value & (0xffffffffffffffffL << 16)) == 0) {
        return 2;
    } else if ((value & (0xffffffffffffffffL << 24)) == 0) {
        return 3;
    } else if ((value & (0xffffffffffffffffL << 32)) == 0) {
        return 4;
    } else if ((value & (0xffffffffffffffffL << 40)) == 0) {
        return 5;
    } else if ((value & (0xffffffffffffffffL << 48)) == 0) {
        return 6;
    } else if ((value & (0xffffffffffffffffL << 56)) == 0) {
        return 7;
    }
    return 8;
}

//constexpr int16_t MLNKVBoolSize(bool value) {
//    return 1;
//}

static const uint8_t MLNKV32Size = 4;

static const uint8_t MLNKV64Size = 8;
//
////// 判断机器模式 大小端
/////**
//// 1) Little-Endian：就是低位字节排放在内存的低地址端，高位字节排放在内存的高地址端。
//// 2) Big-Endian：就是高位字节排放在内存的低地址端，低位字节排放在内存的高地址端。
//// */
//static inline bool MLNKVIsLittleEdian() {
//    union {
//        unsigned int a;
//        unsigned char b;
//    } c;
//    c.a = 1;
//    return (c.b == 1);
//}

static inline uint32_t MLNKVReadUInt32(uint8_t* ptr, uint8_t size) {
    uint32_t value = 0;
    for (uint8_t i = 0; i < size; i++) {
        value |= (uint32_t)(ptr[i] & 0xFF) << (8 * i);
    }
    return value;
}

static inline uint64_t MLNKVReadUInt64(uint8_t* ptr, uint8_t size) {
    uint64_t value = 0;
    for (uint8_t i = 0; i < size; i++) {
        value |= (uint64_t)(ptr[i] & 0xFF) << (8 * i);
    }
    return value;
}

static inline bool MLNKVWriteUInt32(uint8_t* ptr, uint32_t value, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        ptr[i] = (uint8_t)(value & 0xFF);
        value = value >> 8;
    }
    return true;
}

static inline bool MLNKVWriteUInt64(uint8_t* ptr, uint64_t value, uint8_t size) {
    for (uint8_t i = 0; i < size; i++) {
        ptr[i] = (uint8_t)(value & 0xFF);
        value = value >> 8;
    }
    return true;
}


#endif /* MLNKVUtility_hpp */
