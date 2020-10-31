#ifndef USER_DEBUG_H
#define USER_DEBUG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USER_DEBUG_LEVEL 2

#if (USER_DEBUG_LEVEL > 0)
#define  USER_EchoLog(...)     printf("ECHO: "); printf(__VA_ARGS__); printf("\r\n");
#define  USER_EchoLogStart()   printf("ECHO: ");
#define  USER_EchoLogStr(...)     printf(__VA_ARGS__);
#else
#define  USER_EchoLog(...)
#define  USER_EchoLogStart()
#define  USER_EchoLogStr(...)
#endif

#if (USER_DEBUG_LEVEL > 1)
#define  USER_ErrLog(...)     printf("ERROR: "); printf(__VA_ARGS__); printf("\r\n");
#define  USER_ErrLogStart()   printf("ERROR: ");
#define  USER_ErrLogStr(...)     printf(__VA_ARGS__);
#else
#define  USER_ErrLog(...)
#define  USER_ErrLogStart()
#define  USER_ErrLogStr(...)
#endif

#if (USER_DEBUG_LEVEL > 2)
#define  USER_DbgLog(...)     printf("DEBUG: "); printf(__VA_ARGS__); printf("\r\n");
#define  USER_DbgLogStart()   printf("DEBUG: ");
#define  USER_DbgLogStr(...)     printf(__VA_ARGS__);
#else
#define  USER_DbgLog(...)
#define  USER_DbgLogStart()
#define  USER_DbgLogStr(...)
#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif // USER_DEBUG_H

