//
//  MLNKVMemoryCache.m
//  MLNKV
//
//  Copyright © 2019. All rights reserved.
//

#import "MLNKVMemoryCache.h"
#import <UIKit/UIKit.h>
#import <pthread.h>

@interface MLNKVCacheNode : NSObject {
    @package
    __weak MLNKVCacheNode *_prev;
    __weak MLNKVCacheNode *_next;
    NSString *_key;
    id _value;
    NSUInteger _size;
}
@end

@implementation MLNKVCacheNode
@end

@interface MLNKVCache : NSObject {
    MLNKVCacheNode *_head;
    MLNKVCacheNode *_tail;
    
    NSMutableDictionary<NSString *, MLNKVCacheNode *> *_dict;
}

@property (nonatomic, assign) NSUInteger totalCount;
@property (nonatomic, assign) NSUInteger totalSize;

@property BOOL releaseOnMainThread;

- (MLNKVCacheNode *)nodeForKey:(NSString *)aKey;

- (void)insertNodeAtHead:(MLNKVCacheNode *)node;

- (void)bringNodeToHead:(MLNKVCacheNode *)node;

- (void)removeNode:(MLNKVCacheNode *)node;

- (MLNKVCacheNode *)removeTailNode;

- (void)removeAll;

@end

@implementation MLNKVCache

- (instancetype)init {
    if (self = [super init]) {
        _dict = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (NSUInteger)totalCount {
    return _dict.count;
}

- (MLNKVCacheNode *)nodeForKey:(NSString *)aKey {
    return [_dict objectForKey:aKey];
}

- (void)insertNodeAtHead:(MLNKVCacheNode *)node {
    if (!node) return;
    [_dict setObject:node forKey:node->_key];
    _totalSize += node->_size;
    if (_head) {
        node->_next = _head;
        _head->_prev = node;
        _head = node;
    } else {
        _head = _tail = node;
    }
}

- (void)bringNodeToHead:(MLNKVCacheNode *)node {
    if (!node) return;
    if (_head == node) return;
    if (_tail == node) {
        _tail = node->_prev;
        _tail->_next = nil;
    } else {
        node->_next->_prev = node->_prev;
        node->_prev->_next = node->_next;
    }
    node->_next = _head;
    node->_prev = nil;
    _head->_prev = node;
    _head = node;
}

- (void)removeNode:(MLNKVCacheNode *)node {
    if (!node) return;
    [_dict removeObjectForKey:node->_key];
    _totalSize -= node->_size;
    if (node->_next) node->_next->_prev = node->_prev;
    if (node->_prev) node->_prev->_next = node->_next;
    if (_head == node) _head = node->_next;
    if (_tail == node) _tail = node->_prev;
    
    if (_releaseOnMainThread && !pthread_main_np()) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [node class];
        });
    }
}

- (MLNKVCacheNode *)removeTailNode {
    if (!_tail) return nil;
    MLNKVCacheNode *tail = _tail;
    [_dict removeObjectForKey:tail->_key];
    _totalSize -= _tail->_size;
    if (_head == _tail) {
        _head = _tail = nil;
    } else {
        _tail = _tail->_prev;
        _tail->_next = nil;
    }
    if (_releaseOnMainThread && !pthread_main_np()) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [tail class];
        });
    }
    return tail;
}

- (void)removeAll {
    _totalSize = 0;
    _totalCount = 0;
    _head = nil;
    _tail = nil;
    if (_dict.count > 0 && _releaseOnMainThread && !pthread_main_np()) {
        NSArray *nodes = [_dict allValues];
        dispatch_async(dispatch_get_main_queue(), ^{
            [nodes class];
        });
    }
    [_dict removeAllObjects];
}

@end


@interface MLNKVMemoryCache ()

@property (nonatomic, strong) NSMapTable<NSString *, id> *weakMap;

@property (nonatomic, strong) MLNKVCache *LRUCache;

@property (nonatomic, strong) MLNKVCache *FIFOCache;

@end
@implementation MLNKVMemoryCache {
    pthread_mutex_t _lock;
}

- (NSString *)description {
    if (_name) return [NSString stringWithFormat:@"<%@: %p> (%@) [totalCount:%lu, memorySize:%lu]", self.class, self, _name,(unsigned long)(_LRUCache.totalCount + _FIFOCache.totalCount), (unsigned long)(_LRUCache.totalSize + _FIFOCache.totalSize)];
    else return [NSString stringWithFormat:@"<%@: %p> [totalCount:%lu, memorySize:%lu]", self.class, self, (unsigned long)(_LRUCache.totalCount + _FIFOCache.totalCount), (unsigned long)(_LRUCache.totalSize + _FIFOCache.totalSize)];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [_LRUCache removeAll];
    [_FIFOCache removeAll];
    pthread_mutex_destroy(&_lock);
}

- (instancetype)init {
    if (self = [super init]) {
        pthread_mutex_init(&_lock, NULL);
        _LRUCountLimit = 10000;
        _FIFOCountLimit = 1000;
        _memorySizeLimit = 100 << 20;
        _releaseOnMainThread = NO;
        _shouldRemoveAllObjectsOnMemoryWarning = YES;
        _shouldRemoveAllObjectsWhenEnteringBackground = YES;
        _usedLRU = YES;
        _usedFIFO = YES;
        
        _weakMap = [NSMapTable strongToWeakObjectsMapTable];
        _LRUCache = [[MLNKVCache alloc] init];
        _FIFOCache = [[MLNKVCache alloc] init];
        _LRUCache.releaseOnMainThread = _releaseOnMainThread;
        _FIFOCache.releaseOnMainThread = _releaseOnMainThread;
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidReceiveMemoryWarningNotification) name:UIApplicationDidReceiveMemoryWarningNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackgroundNotification) name:UIApplicationDidEnterBackgroundNotification object:nil];
    }
    return self;
}

- (BOOL)containsObjectForKey:(NSString *)akey {
    if (!akey) return NO;
    pthread_mutex_lock(&_lock);
    BOOL contains = [_weakMap objectForKey:akey] || [_FIFOCache nodeForKey:akey] || [_LRUCache nodeForKey:akey];
    pthread_mutex_unlock(&_lock);
    return contains;
}

- (id)objectForKey:(NSString *)akey {
    if (!akey) return nil;
    pthread_mutex_lock(&_lock);
    MLNKVCacheNode *node = (_usedFIFO && _FIFOCountLimit > 0)? [_FIFOCache nodeForKey:akey]: nil;
    if (node) {
        [_FIFOCache removeNode:node];
        if (_usedLRU && _LRUCountLimit > 0) {
            [_LRUCache insertNodeAtHead:node];
        }
    }else {
        if (_usedLRU && _LRUCountLimit > 0) {
            node = [_LRUCache nodeForKey:akey];
            [_LRUCache bringNodeToHead:node];
        }
    }
    if (_usedLRU && _LRUCache.totalCount > _LRUCountLimit) {
        [_LRUCache removeTailNode];
    }
    id value = nil;
    if (node) {
        value = node->_value;
    }else {
        value = [_weakMap objectForKey:akey];
    }
    pthread_mutex_unlock(&_lock);
    return value;
}

- (void)setObject:(id)object forKey:(NSString *)akey {
    [self setObject:object forKey:akey withSize:0];
}

- (void)setObject:(id)object forKey:(NSString *)akey withSize:(NSUInteger)size {
    if (!akey) return;
    if (!object) {
        [self removeObjectForKey:akey];
        return;
    }
    pthread_mutex_lock(&_lock);
    [_weakMap setObject:object forKey:akey];
    
    MLNKVCacheNode *node = (_usedLRU && _LRUCountLimit > 0)? [_LRUCache nodeForKey:akey]: nil;
    if (node) {
        _LRUCache.totalSize = (_LRUCache.totalSize < node->_size)? 0: (_LRUCache.totalSize - node->_size);
        _LRUCache.totalSize += size;
        node->_value = object;
        node->_size = size;
        [_LRUCache bringNodeToHead:node];
    }else {
        if (_usedFIFO && _FIFOCountLimit > 0) {
            node = [_FIFOCache nodeForKey:akey];
            if (node) {
                _FIFOCache.totalSize = (_FIFOCache.totalSize < node->_size)? 0: (_FIFOCache.totalSize - node->_size);
                _FIFOCache.totalSize += size;
                node->_value = object;
                node->_size = size;
                [_FIFOCache bringNodeToHead:node];
            }else {
                node = [[MLNKVCacheNode alloc] init];
                node->_key = akey;
                node->_value = object;
                node->_size = size;
                [_FIFOCache insertNodeAtHead:node];
            }
            if (_FIFOCache.totalCount > _FIFOCountLimit) {
                [_FIFOCache removeTailNode];
            }
        }
    }
    
    while (_FIFOCache.totalSize + _LRUCache.totalSize > _memorySizeLimit) {
        if (![_LRUCache removeTailNode]) {
            if (![_FIFOCache removeTailNode]) {
                break;
            }
        }
    }
    pthread_mutex_unlock(&_lock);
}

- (void)setWeakObj:(id)weakObj forKey:(NSString *)aKey {
    if (!aKey) return;
    pthread_mutex_lock(&_lock);
    [_weakMap setObject:weakObj forKey:aKey];
    pthread_mutex_unlock(&_lock);
}

- (void)removeObjectForKey:(NSString *)akey {
    if (!akey) return;
    pthread_mutex_lock(&_lock);
    if (_usedFIFO && _FIFOCountLimit > 0) {
        [_FIFOCache removeNode:[_FIFOCache nodeForKey:akey]];
    }
    if (_usedLRU && _LRUCountLimit > 0) {
       [_LRUCache removeNode:[_LRUCache nodeForKey:akey]];
    }
    [_weakMap removeObjectForKey:akey];
    pthread_mutex_unlock(&_lock);
}

- (void)removeAllObjects {
    pthread_mutex_lock(&_lock);
    [_FIFOCache removeAll];
    [_LRUCache removeAll];
    [_weakMap removeAllObjects];
    pthread_mutex_unlock(&_lock);
}

#pragma mark - set

- (void)setReleaseOnMainThread:(BOOL)releaseOnMainThread {
    pthread_mutex_lock(&_lock);
    _releaseOnMainThread = releaseOnMainThread;
    _FIFOCache.releaseOnMainThread = releaseOnMainThread;
    _LRUCache.releaseOnMainThread = releaseOnMainThread;
    pthread_mutex_unlock(&_lock);
}

- (NSUInteger)totalCount {
    pthread_mutex_lock(&_lock);
    NSUInteger count = _FIFOCache.totalCount + _LRUCache.totalCount;
    pthread_mutex_unlock(&_lock);
    return count;
}

- (NSUInteger)totalMemorySize {
    pthread_mutex_lock(&_lock);
    NSUInteger size = _FIFOCache.totalSize + _LRUCache.totalSize;
    pthread_mutex_unlock(&_lock);
    return size;
}

#pragma mark - ---

- (void)applicationDidReceiveMemoryWarningNotification {
    if (_shouldRemoveAllObjectsOnMemoryWarning) {
        [self removeAllObjects];
    }
}

- (void)applicationDidEnterBackgroundNotification {
    if (_shouldRemoveAllObjectsWhenEnteringBackground) {
        [self removeAllObjects];
    }
}

@end
