#include <sys/stat.h>
#include "Log.hh"

#define SYS_LOG_NUMBER 10000
#define APP_LOG_NUMBER 10000

namespace CPPUTILS
{

Log Log::sysLog("/tmp/sys.log", SYS_LOG_NUMBER);
Log Log::appLog("/tmp/app.log", APP_LOG_NUMBER);

Log::Log(const char* fileName, int maxLogNumber)
    :filename_(fileName), maxlognumber_(maxLogNumber), lognumber_(0)
{
    // open log file
    fp_ = fopen(filename_, "w+");
    if (fp_)
    {
        // if not doing so, ftell() will return 0 in case of no fwrite
        fseek(fp_, 0, SEEK_END);

        // to support mutiple users running ptp simulator on same machine
        chmod(filename_, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    }
}

Log::~Log()
{
    if (fp_) fclose(fp_);
}

void Log::addLog(const char* log)
{
    if (fp_ && (lognumber_ < maxlognumber_))
    {
        size_t logSize = strlen(log);
        if (logSize > 0)
        {
           if (log[logSize - 1] == '\n')
               fprintf(fp_, "%s", log);
           else
               fprintf(fp_, "%s\n", log);
           fflush(fp_);
           lognumber_++;
        }
    }
}

void Log::clear()
{
    if (fp_)
    {
        /* clr log file */
        fclose(fp_);
        fp_ = fopen(filename_, "w+");
    }
    lognumber_ = 0;
}

std::string Log::read(size_t size)
{
    std::string data;

    if (fp_)
    {
        fclose(fp_);
    }

    fp_ = fopen(filename_, "a+");

    if (!fp_) return data;

    fseek(fp_, 0, SEEK_END);
    size_t fsize = ftell(fp_);
    size = (fsize > size) ? size : fsize;

    fseek(fp_, fsize - size, SEEK_SET);

    data.resize(size);
    fread(&data[0], 1, size, fp_);

    return data;
}

}

