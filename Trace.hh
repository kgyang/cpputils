#ifndef _TRACE_H_
#define _TRACE_H_

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

typedef enum
{
  TRACE_LEVEL_NONE = 0,
  TRACE_LEVEL_ERROR,
  TRACE_LEVEL_WARNING,
  TRACE_LEVEL_DEBUG,
  TRACE_LEVEL_DETAIL,
} TraceLevel;

void trace_setLevel(TraceLevel level);

TraceLevel trace_getLevel(void);

#define TRACE_PRINT_DATE \
                do{ \
                    struct timeval now; \
                    long iHour, iMin, iSec; \
                    if (gettimeofday(&now , (struct timezone *)0) < 0) \
                    { \
                      now.tv_sec = 0; \
                      now.tv_usec = 0; \
                    } \
                    iHour = now.tv_sec % 86400; \
                    iSec = iHour % 60; \
                    iHour = (iHour - iSec) / 60; \
                    iMin = iHour % 60; \
                    iHour = (iHour - iMin) / 60;\
                    printf( "%02ld:%02ld:%02ld.%03ld", iHour, iMin, iSec, now.tv_usec/1000); \
                  } while(0)

#define  TRACE_PRINT( fmt, ...)  \
              do  { \
                    TRACE_PRINT_DATE; \
                    printf(": %s: ", __FUNCTION__); \
                    printf(fmt, ## __VA_ARGS__); \
                    printf("\n"); \
                  } while(0)


#define  TRACE_ERROR( fmt, ...) \
              do  { \
                    if (trace_getLevel() >= TRACE_LEVEL_ERROR) \
                    { \
                      printf("ERR - "); \
                      TRACE_PRINT(fmt, ## __VA_ARGS__); \
                      TRACE_LOG(fmt, ## __VA_ARGS__); \
                    } \
                   } while(0)

#define  TRACE_WARNING( fmt, ...) \
              do  { \
                    if (trace_getLevel() >= TRACE_LEVEL_WARNING) \
                    { \
                      printf("WRN - "); \
                      TRACE_PRINT(fmt, ## __VA_ARGS__); \
                      TRACE_LOG(fmt, ## __VA_ARGS__); \
                    } \
                  } while(0)

#define  TRACE_DEBUG( fmt, ...) \
                do  { \
                    if (trace_getLevel() >= TRACE_LEVEL_DEBUG) \
                    { \
                      printf("DBG - "); \
                      TRACE_PRINT(fmt, ## __VA_ARGS__); \
                    } \
                  } while(0)


#define  TRACE_DETAIL( fmt, ...) \
                do  { \
                    if (trace_getLevel() >= TRACE_LEVEL_DETAIL) \
                    { \
                      printf("DTL - "); \
                      TRACE_PRINT(fmt, ## __VA_ARGS__); \
                    } \
                  } while(0)

#endif
