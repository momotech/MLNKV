//
//  MILKVLog.c
//  MLNKV
//
//  Copyright © 2019 . All rights reserved.
//

#include "MLNKVLog.h"
#include <string>
#include <iostream>
#include <sys/time.h>


static MLNKVLogCallbackHandler *kMLNKVLogCallbackHandler;
void MLNKVSetLogCallbackHandler(MLNKVLogCallbackHandler *callbackHandler) {
    kMLNKVLogCallbackHandler = callbackHandler;
}
MLNKVLogCallbackHandler *MLNKVGetLogCallbackHandler(void) {
    return kMLNKVLogCallbackHandler;
}

void MLNKVLogWithLevel(MLNLogLevel level, const char *file, const char *func, int line, const char *format, ...) {
    
    if (kMLNKVLogCallbackHandler) {
        va_list args;
        va_start(args, format);
        kMLNKVLogCallbackHandler(level, file, func, line, format, args);
        va_end(args);
        return;
    }
    
    std::string message;
    char buffer[16];

    va_list args;
    va_start(args, format);
    auto length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (length < 0) { // something wrong
        message = {};
    } else if (length < sizeof(buffer)) {
        message = std::string(buffer, static_cast<unsigned long>(length));
    } else {
        message.resize(static_cast<unsigned long>(length), '\0');
        va_start(args, format);
        vsnprintf(const_cast<char *>(message.data()), static_cast<size_t>(length) + 1,
                       format, args);
        va_end(args);
    }
    
    time_t t;
    struct tm *timeinfo;
    time(&t);
    timeinfo = localtime(&t);
    
    char timeStr[128];
    strftime(timeStr, sizeof(timeStr), "%F %T", timeinfo);
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    
    printf("%s.%d <%s:%s::%d> [%c] %s \n", timeStr, (int)tv.tv_usec, file, func, line, (level == MLNLogDebug)?'D':'E', message.c_str());
}
