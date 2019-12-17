//
//  MLNKV.m
//  MLNKV
//
//  Copyright © 2019 All rights reserved.
//

#import "MLNKV.h"
#import "MLNKVBase.h"
#import "MLNKVLog.h"
#import "MLNKVMemoryCache.h"

#define MLNKVSetKeyAssert() NSAssert(aKey && [aKey isKindOfClass:NSString.class], @"key must be not nil...");\
if (!aKey || ![aKey isKindOfClass:NSString.class]) return NO;

#define MLNKVGetKeyAssert(result) NSAssert(aKey && [aKey isKindOfClass:NSString.class], @"key must be not nil...");\
if (!aKey || ![aKey isKindOfClass:NSString.class]) return result;

@interface MLNKV ()

@property (nonatomic, copy) NSString *path;
@property (nonatomic, strong) MLNKVMemoryCache *memoryCache;

@end
@implementation MLNKV {
    MLNKVBase *_kvBase;
}

- (void)dealloc {
    if (_kvBase) {
        delete _kvBase;
        _kvBase = nullptr;
    }
}

+ (instancetype)defaultMLNKV {
    static dispatch_once_t onceToken;
    static MLNKV *mlnkv;
    dispatch_once(&onceToken, ^{
        mlnkv = [self mlnkvWithPath:[MLNKVDEFAULTPATH stringByAppendingPathComponent:@".defalut.kv"]];
    });
    return mlnkv;
}

+ (instancetype)mlnkvWithPath:(NSString *)path {
    return [[self alloc] initMLNKVWithPath:path];
}

- (instancetype)initMLNKVWithPath:(NSString *)path {
    NSParameterAssert(path);
    if (self = [super init]) {
        if (path && [path isKindOfClass:NSString.class]) {
            _path = path;
            _kvBase = new MLNKVBase(path.UTF8String);
            _memoryCache = [[MLNKVMemoryCache alloc] init];
            // 设置耗时
//            [[NSFileManager defaultManager] setAttributes:@{NSFileProtectionKey:NSFileProtectionNone} ofItemAtPath:_path error:NULL];
        }
    }
    return self;
}

#pragma mark - set

- (BOOL)setKVObject:(NSObject<NSCoding> *)value forKey:(NSString *)aKey archiveBlock:(MLNKVCustomArchiveBlock)archiveBlock {
    MLNKVSetKeyAssert();
    if (!value || [value isKindOfClass:NSData.class]) {
        return [self setKVData:(NSData *)value forKey:aKey];
    }else if ([value isKindOfClass:NSString.class]) {
        return [self setKVString:(NSString *)value forKey:aKey];
    }else if ([value isKindOfClass:NSNumber.class]) {
        NSNumber *numberValue = (NSNumber *)value;
        const char *valueType = numberValue.objCType;
        if (memcmp(valueType, @encode(int32_t), 1) == 0) { // i
            return [self setKVInt32:numberValue.intValue forKey:aKey];
        }else if (memcmp(valueType, @encode(int64_t), 1) == 0) { // q
            return [self setKVInt64:numberValue.longLongValue forKey:aKey];
        }else if (memcmp(valueType, @encode(double), 1) == 0) { // d
            return [self setKVDouble:numberValue.doubleValue forKey:aKey];
        }else if (memcmp(valueType, @encode(char), 1) == 0) { //c bool
            return [self setKVBool:numberValue.boolValue forKey:aKey];
        }else {
            NSAssert(NO, @"not implement check obj:%@ type:%s",numberValue ,numberValue.objCType);
        }
    }else { // 归档
        NSData *data = nil;
        if (archiveBlock) {
            data = archiveBlock(value);
        }else if (_customArchiveBlock) {
            data = _customArchiveBlock(value);
        }else {
            NSError *error;
            @try {
                if (@available(iOS 11.0, *)) {
                    data = [NSKeyedArchiver archivedDataWithRootObject:value requiringSecureCoding:NO error:&error];
                } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    data = [NSKeyedArchiver archivedDataWithRootObject:value];
#pragma clang diagnostic pop
                }
            } @catch (NSException *exception) {
                NSAssert(NO, @"archived error:%@", exception);
                error = [NSError errorWithDomain:exception.reason code:0 userInfo:exception.userInfo];
            }
            if (error) {
                MLNKVError("archived data error:%s", error.localizedDescription.UTF8String);
                return NO;
            }
        }
        if (_kvBase->setBytes(data.bytes, data.length, aKey.UTF8String)) {
            [_memoryCache setObject:value forKey:aKey];
            return YES;
        }
    }
    return NO;
}

- (BOOL)setKVObject:(NSObject<NSCoding> *)value forKey:(NSString *)aKey {
    return [self setKVObject:value forKey:aKey archiveBlock:NULL];
}

- (BOOL)setKVData:(NSData *)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    if (_kvBase->setBytes(value.bytes, value.length, aKey.UTF8String)) {
        [_memoryCache setObject:value forKey:aKey];
        return YES;
    }
    return NO;
}

- (BOOL)setKVString:(NSString *)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    if (!value || ![value isKindOfClass:NSString.class]) {
        return [self setKVObject:value forKey:aKey];
    }
    NSUInteger length = value.length;
    unichar buffer[length];
    [value getCharacters:buffer range:NSMakeRange(0, length)];
    return _kvBase->setBytes(buffer, length * sizeof(unichar), aKey.UTF8String);
}

- (BOOL)setKVInt32:(int32_t)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    return _kvBase->setInt32(value, aKey.UTF8String);
}

- (BOOL)setKVInt64:(int64_t)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    return _kvBase->setInt64(value, aKey.UTF8String);
}

- (BOOL)setKVFloat:(float)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    return _kvBase->setFloat(value, aKey.UTF8String);
}

- (BOOL)setKVDouble:(double)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    return _kvBase->setDouble(value, aKey.UTF8String);
}

- (BOOL)setKVBool:(BOOL)value forKey:(NSString *)aKey {
    MLNKVSetKeyAssert();
    return _kvBase->setBool(value, aKey.UTF8String);
}

#pragma mark - get

- (id)getKVObjectForKey:(NSString *)aKey ofClass:(Class)clz unarchiveBlock:(MLNKVCustomUnarchiveBlock)unarchiveBlock {
//    NSAssert(clz, @"clz can't be nil...");
//    if (!clz) return nil;
    MLNKVGetKeyAssert(nil);
    
    if (id obj = [_memoryCache objectForKey:aKey]) {
        return obj;
    }
    if ([clz isKindOfClass:NSData.class]) {
        return [self getKVDataForKey:aKey defaultValue:nil];
    }else if ([clz isKindOfClass:NSString.class]) {
        return [self getKVStringForKey:aKey defaultValue:nil];
    }else if ([clz isKindOfClass:NSNumber.class]) {
        NSAssert(NO, @"please use int/float/bool method, eg: getKVInt32ForKey: ...");
        return nil;
    }
    void *bytes;
    size_t size;
    if (!_kvBase->getBytesForKey(aKey.UTF8String, bytes, size)) return nil;
    
    id obj = nil;
    NSData *data = [NSData dataWithBytes:bytes length:size];
    if (unarchiveBlock) {
        obj = unarchiveBlock(data);
    }else if (_customUnarchiveBlock) {
        obj = _customUnarchiveBlock(data);
    }else {
        NSError *error;
        @try {
            if (@available(iOS 11.0, *)) {
                if (clz) {
                    obj = [NSKeyedUnarchiver unarchivedObjectOfClass:clz fromData:data error:&error];
                }else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                    obj = [NSKeyedUnarchiver unarchiveObjectWithData:data];
#pragma clang diagnostic pop
                }
            }else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                obj = [NSKeyedUnarchiver unarchiveObjectWithData:data];
#pragma clang diagnostic pop
            }
        } @catch (NSException *exception) {
            NSAssert(NO, @"unarchived error:%@", exception);
            error = [NSError errorWithDomain:exception.reason code:0 userInfo:exception.userInfo];
        }
        if (error) {
            MLNKVError("unarchived data error:%s", error.localizedDescription.UTF8String);
            return nil;
        }
    }
    if (clz == nil || (clz && [clz isKindOfClass:[obj class]])) {
        [_memoryCache setObject:obj forKey:aKey];
        return obj;
    }
    NSAssert(obj, @"unarchived error obj:%@  expected class:%@", obj, clz);
    return nil;
}

- (id)getKVObjectForKey:(NSString *)aKey ofClass:(Class)clz {
    return [self getKVObjectForKey:aKey ofClass:clz unarchiveBlock:NULL];
}

- (NSData *)getKVDataForKey:(NSString *)aKey {
    return [self getKVDataForKey:aKey defaultValue:nil];
}

- (NSData *)getKVDataForKey:(NSString *)aKey defaultValue:(NSData *)defaultValue {
    MLNKVGetKeyAssert(nil);
    if (NSData *obj = [_memoryCache objectForKey:aKey]) {
        return [obj isKindOfClass:NSData.class]? obj: defaultValue;
    }
    void *bytes;
    size_t size;
    if (_kvBase->getBytesForKey(aKey.UTF8String, bytes, size)) {
        NSData *data = [NSData dataWithBytes:bytes length:size];
        [_memoryCache setObject:data forKey:aKey];
        return data;
    }
    return defaultValue;
}

- (NSString *)getKVStringForKey:(NSString *)aKey {
    return [self getKVStringForKey:aKey defaultValue:nil];
}

- (NSString *)getKVStringForKey:(NSString *)aKey defaultValue:(NSString *)defaultValue {
    MLNKVGetKeyAssert(nil);
    void *value;
    size_t size;
    if (_kvBase->getBytesForKey(aKey.UTF8String, value, size)) {
        NSString *valueStr = [[NSString alloc] initWithCharacters:(unichar *)value length:size/sizeof(unichar)];
        return valueStr;
    }
    return defaultValue;
}

- (int32_t)getKVInt32ForKey:(NSString *)aKey {
    return [self getKVInt32ForKey:aKey defaultValue:0];
}

- (int32_t)getKVInt32ForKey:(NSString *)aKey defaultValue:(int32_t)defaultValue {
    MLNKVGetKeyAssert(0);
    return _kvBase->getInt32ForKey(aKey.UTF8String, defaultValue);
}

- (int64_t)getKVInt64ForKey:(NSString *)aKey {
    return [self getKVInt64ForKey:aKey defaultValue:0];
}

- (int64_t)getKVInt64ForKey:(NSString *)aKey defaultValue:(int64_t)defaultValue {
    MLNKVGetKeyAssert(0);
    return _kvBase->getInt64ForKey(aKey.UTF8String, defaultValue);
}

- (float)getKVFloatForKey:(NSString *)aKey {
    return [self getKVFloatForKey:aKey defaultValue:0];
}

- (float)getKVFloatForKey:(NSString *)aKey defaultValue:(float)defaultValue {
    MLNKVGetKeyAssert(0);
    return _kvBase->getFloatForKey(aKey.UTF8String, defaultValue);
}

- (double)getKVDoubleForKey:(NSString *)aKey {
    return [self getKVDoubleForKey:aKey defaultValue:0];
}

- (double)getKVDoubleForKey:(NSString *)aKey defaultValue:(double)defaultValue {
    MLNKVGetKeyAssert(0);
    return _kvBase->getDoubleForKey(aKey.UTF8String, defaultValue);
}

- (BOOL)getKVBoolForKey:(NSString *)aKey {
    return [self getKVBoolForKey:aKey defaultValue:NO];
}

- (BOOL)getKVBoolForKey:(NSString *)aKey defaultValue:(BOOL)defaultValue {
    MLNKVGetKeyAssert(NO);
    return _kvBase->getBoolForKey(aKey.UTF8String, defaultValue);
}

#pragma mark - ...

- (MLNKVValueType)getValueType:(NSString *)aKey {
    MLNKVGetKeyAssert(MLNKVValueType_None);
    MLNKVValueType valueType;
    _kvBase->getValueSizeForKey(aKey.UTF8String, valueType);
    return valueType;
}

- (size_t)getValueSizeForKey:(NSString *)aKey {
    MLNKVGetKeyAssert(0);
    MLNKVValueType valueType;
    return _kvBase->getValueSizeForKey(aKey.UTF8String, valueType);
}

- (BOOL)containsKey:(NSString *)aKey {
    MLNKVGetKeyAssert(NO);
    return _kvBase->containsKey(aKey.UTF8String);
}

- (size_t)count {
    return _kvBase->count();
}

- (size_t)fileSize {
    return _kvBase->getFileSize();
}

- (size_t)usedSize {
    return _kvBase->totalUsedSize();
}
- (NSArray *)allKeys {
    std::vector<std::string> keys = _kvBase->allKeys();
    NSMutableArray *arrKeys = [NSMutableArray arrayWithCapacity:keys.size()];
    for (auto k : keys) {
        NSString *key = [[NSString alloc] initWithUTF8String:k.c_str()];
        if (key) {
            [arrKeys addObject:key];
        }
    }
    return arrKeys;
}

- (void)removeValueForKey:(NSString *)aKey {
    MLNKVGetKeyAssert();
    _kvBase->remove(aKey.UTF8String);
    [_memoryCache removeObjectForKey:aKey];
}

- (void)removeValuesForKeys:(NSArray<NSString *> *)arrKeys {
    for (NSString *key in arrKeys) {
        if ([key isKindOfClass:NSString.class]) {
            _kvBase->remove(key.UTF8String);
            [_memoryCache removeObjectForKey:key];
        }
    }
}

- (void)clearAll {
    _kvBase->clearAll();
    [_memoryCache removeAllObjects];
}

- (void)trim {
    _kvBase->trim();
}

- (void)clearMemoryCache {
    [_memoryCache removeAllObjects];
}

- (void)sync {
    _kvBase->sync(true);
}

- (void)async {
    _kvBase->sync(false);
}

@end
