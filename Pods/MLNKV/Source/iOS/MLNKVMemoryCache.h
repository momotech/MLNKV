//
//  MLNKVMemoryCache.h
//  MLNKV
//
//  内存缓存层
//  FIFO + LRU + NSMapTable
//
//  Copyright © 2019 . All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MLNKVMemoryCache : NSObject
@property (copy, nullable) NSString *name;
@property (readonly) NSUInteger totalCount;
@property (readonly) NSUInteger totalMemorySize;

@property NSUInteger LRUCountLimit; // default 10000  if == 0 don't use LRU cache
@property NSUInteger FIFOCountLimit; // default 1000  if == 0 don't use FIFO cache
@property NSUInteger memorySizeLimit; // default 100M 100 << 20

@property (nonatomic) BOOL releaseOnMainThread; // default NO

@property BOOL shouldRemoveAllObjectsOnMemoryWarning; // default YES
@property BOOL shouldRemoveAllObjectsWhenEnteringBackground; // default YES

@property BOOL usedLRU; // default YES  use LRU cache
@property BOOL usedFIFO; // default YES use FIFO cache

#pragma mark - method
- (BOOL)containsObjectForKey:(NSString *)aKey;
- (nullable id)objectForKey:(NSString *)aKey;
- (void)setObject:(nullable id)object forKey:(NSString *)aKey;
- (void)setObject:(nullable id)object forKey:(NSString *)aKey withSize:(NSUInteger)size;
- (void)setWeakObj:(nullable id)weakObj forKey:(NSString *)aKey;
- (void)removeObjectForKey:(NSString *)aKey;
- (void)removeAllObjects;

@end

NS_ASSUME_NONNULL_END
