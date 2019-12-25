#ifdef __OBJC__
#import <UIKit/UIKit.h>
#else
#ifndef FOUNDATION_EXPORT
#if defined(__cplusplus)
#define FOUNDATION_EXPORT extern "C"
#else
#define FOUNDATION_EXPORT extern
#endif
#endif
#endif

#import "MLNKV.h"
#import "MLNKVMemoryCache.h"
#import "MLNKVBase.h"
#import "MLNKVLog.h"
#import "MLNKVType.h"
#import "MLNKVUtility.h"
#import "MLNKVValueInfo.hpp"

FOUNDATION_EXPORT double MLNKVVersionNumber;
FOUNDATION_EXPORT const unsigned char MLNKVVersionString[];

