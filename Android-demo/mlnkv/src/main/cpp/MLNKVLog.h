//
//  MLNKVLog_h.h
//  MLNKV
//
//  Copyright © 2019. All rights reserved.
//

#ifndef MLNKVLog_h
#define MLNKVLog_h

#include <stdio.h>
#include <errno.h>

typedef enum {
    MLNLogDebug = 0,
    MLNLogError = 1,
} MLNLogLevel;


//#define MLNDebug

#ifdef MLNDebug

#define __mlnkv_filename__ (strrchr(__FILE__, '/') + 1)

#define MLNKVError(format, ...) MLNKVLogWithLevel(MLNLogError, __mlnkv_filename__, __func__, __LINE__, format, ##__VA_ARGS__)
#define MLNKVLog(format, ...) MLNKVLogWithLevel(MLNLogDebug, __mlnkv_filename__, __func__, __LINE__, format, ##__VA_ARGS__)

void MLNKVLogWithLevel(MLNLogLevel level, const char *file, const char *func, int line, const char *format, ...);

#else

#define MLNKVError(format, ...) 
#define MLNKVLog(format, ...)

#endif




#endif /* MLNKVLog_h */
