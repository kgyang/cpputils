#ifndef _LOG_HH_
#define _LOG_HH_

#include <stdio.h>
#include <string.h>
#include <string>
#include <time.h>
#include <sys/time.h>

#define SYS_LOG(fmt, ...)  \
do { \
    char buf[512]; \
    time_t now; \
    struct tm now_tm; \
    now = time(0); \
    gmtime_r(&now, &now_tm); \
    memset(buf, 0, sizeof(buf)); \
    strftime(buf, sizeof(buf), "%F %T ", &now_tm); \
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, \
             "%s: ", __FUNCTION__); \
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, \
             fmt, ##__VA_ARGS__); \
    CPPUTILS::Log::sysLog.addLog(buf); \
} while(0)

#define APP_LOG(fmt, ...)  \
do { \
    char buf[256]; \
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
    memset(buf, 0, sizeof(buf)); \
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, \
             "%02ld:%02ld:%02ld.%03ld,", iHour, iMin, iSec, now.tv_usec/1000); \
    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf) - 1, \
             fmt, ##__VA_ARGS__); \
    CPPUTILS::Log::appLog.addLog(buf); \
} while(0)

#define ERR_LOG(fmt, ...)  \
do { \
    SYS_LOG(fmt, ##__VA_ARGS__); \
} while(0)

namespace CPPUTILS
{

class Log
{
    public:
        Log(const char* fileName, int maxLogNumber);
        virtual ~Log();
        void addLog(const char* log);
        void clear();
        std::string read(size_t size);
        void setMaxLogNumber(int num) { maxlognumber_ = num; }
        int getMaxLogNumber(void) const { return maxlognumber_; }
        int getLogNumber(void) const { return lognumber_; }

        static Log sysLog;
        static Log appLog;

    private:
        const char* filename_;
        FILE *fp_;
        int maxlognumber_;
        int lognumber_;
};

}

#endif	// _LOG_HH_
