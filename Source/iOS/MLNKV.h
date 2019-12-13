//
//  MLNKV.h
//  MLNKV
//
//
//
//  Copyright © 2019 All rights reserved.
//

#import <Foundation/Foundation.h>
#import "MLNKVMemoryCache.h"
#import "MLNKVType.h"

#define MLNKVDEFAULTPATH [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject stringByAppendingPathComponent:@".mlnkv"]

NS_ASSUME_NONNULL_BEGIN

typedef NSData *_Nullable(^MLNKVCustomArchiveBlock)(id obj);
typedef id _Nullable(^MLNKVCustomUnarchiveBlock)(NSData *data);

@class MLNKVMemoryCache;

@interface MLNKV : NSObject

+ (instancetype)defaultMLNKV;

+ (instancetype)mlnkvWithPath:(NSString *)path;

- (instancetype)initMLNKVWithPath:(NSString *)path;

@property (nonatomic, copy, readonly) NSString *path;
@property (nonatomic, strong, readonly) MLNKVMemoryCache *memoryCache;

@property (nullable, copy) MLNKVCustomArchiveBlock customArchiveBlock;
@property (nullable, copy) MLNKVCustomUnarchiveBlock customUnarchiveBlock;

// set

- (BOOL)setKVObject:(nullable NSObject<NSCoding> *)value forKey:(NSString *)aKey archiveBlock:(nullable MLNKVCustomArchiveBlock)archiveBlock;

- (BOOL)setKVObject:(nullable NSObject<NSCoding> *)value forKey:(NSString *)aKey;

- (BOOL)setKVData:(nullable NSData *)value forKey:(NSString *)aKey;

- (BOOL)setKVString:(nullable NSString *)value forKey:(NSString *)aKey;

- (BOOL)setKVInt32:(int32_t)value forKey:(NSString *)aKey;

- (BOOL)setKVInt64:(int64_t)value forKey:(NSString *)aKey;

- (BOOL)setKVFloat:(float)value forKey:(NSString *)aKey;

- (BOOL)setKVDouble:(double)value forKey:(NSString *)aKey;

- (BOOL)setKVBool:(BOOL)value forKey:(NSString *)aKey;

// get

- (nullable id)getKVObjectForKey:(NSString *)aKey ofClass:(nullable Class)clz unarchiveBlock:(nullable MLNKVCustomUnarchiveBlock)unarchiveBlock;

- (nullable id)getKVObjectForKey:(NSString *)aKey ofClass:(nullable Class)clz;

- (nullable NSData *)getKVDataForKey:(NSString *)aKey;
- (nullable NSData *)getKVDataForKey:(NSString *)aKey defaultValue:(nullable NSData *)defaultValue;

- (nullable NSString *)getKVStringForKey:(NSString *)aKey;
- (nullable NSString *)getKVStringForKey:(NSString *)aKey defaultValue:(nullable NSString *)defaultValue;

- (int32_t)getKVInt32ForKey:(NSString *)aKey;
- (int32_t)getKVInt32ForKey:(NSString *)aKey defaultValue:(int32_t)defaultValue;

- (int64_t)getKVInt64ForKey:(NSString *)aKey;
- (int64_t)getKVInt64ForKey:(NSString *)aKey defaultValue:(int64_t)defaultValue;

- (float)getKVFloatForKey:(NSString *)aKey;
- (float)getKVFloatForKey:(NSString *)aKey defaultValue:(float)defaultValue;

- (double)getKVDoubleForKey:(NSString *)aKey;
- (double)getKVDoubleForKey:(NSString *)aKey defaultValue:(double)defaultValue;

- (BOOL)getKVBoolForKey:(NSString *)aKey;
- (BOOL)getKVBoolForKey:(NSString *)aKey defaultValue:(BOOL)defaultValue;

// ...
- (MLNKVValueType)getValueType:(NSString *)aKey;
- (size_t)getValueSizeForKey:(NSString *)aKey;
- (BOOL)containsKey:(NSString *)aKey ;
- (size_t)count;
- (size_t)fileSize;
- (size_t)usedSize;
- (NSArray *)allKeys;

- (void)removeValueForKey:(NSString *)key;
- (void)removeValuesForKeys:(NSArray<NSString *> *)arrKeys;

- (void)clearAll;
- (void)trim;
- (void)clearMemoryCache;

- (void)sync;
- (void)async;

// unavailable
+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;
- (id)copy NS_UNAVAILABLE;
- (id)mutableCopy NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
