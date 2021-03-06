#include <string.h>

#include "base/LinuxWin.h"

int GetLastErrorCode()
{
#ifdef WIN32
    int errorCode = GetLastError();
#else
    errorCode = errno;
#endif

    return errorCode;
}

std::string GetErrorText(int errcode)
{
#ifdef WIN32
    LPVOID buffer;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    nullptr,
                    errcode,
                    0,
                    (LPTSTR)&buffer,
                    0,
                    nullptr);

    return (char *)buffer;
#else
    return (strerror(errcode));
#endif
}

std::string GetLastErrorText()
{
    int errcode = GetLastErrorCode();

#ifdef WIN32
    LPVOID buffer;
    ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    nullptr,
                    errcode,
                    0,
                    (LPTSTR)&buffer,
                    0,
                    nullptr);

    std::string s = (char *)buffer;
    LocalFree(buffer);

    return s;
#else
    return (strerror(errcode));
#endif
}

#ifdef WIN32
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);

    tm.tm_year  = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday  = wtm.wDay;
    tm.tm_hour  = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm.tm_isdst = -1;
    clock       = mktime(&tm);

    tp->tv_sec  = (long)clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;

    return (0);
}
#endif

int GetCurrThreadID()
{
#ifdef WIN32
    return GetCurrentThreadId();
#else
    return static_cast<pid_t>(::syscall(SYS_gettid));;
#endif
}
