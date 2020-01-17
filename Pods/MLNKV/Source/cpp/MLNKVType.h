//
//  MLNKVType.h
//  
//
//

#ifndef MLNKVType_h
#define MLNKVType_h

typedef enum : uint8_t {
    MLNKVValueType_None   = 0,
    MLNKVValueType_Used   = 1,
    MLNKVValueType_Bool   = 2,
    MLNKVValueType_Float  = 3,
    MLNKVValueType_Int32  = 4,
    MLNKVValueType_Double = 5,
    MLNKVValueType_Int64  = 6,
    MLNKVValueType_String = 7,
    MLNKVValueType_Bytes  = 8,
} MLNKVValueType;


#endif /* MLNKVType_h */
