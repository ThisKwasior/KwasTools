#pragma once

#include <time.h>
#include <utime.h>
#include <sys/stat.h>

inline static const time_t du_get_file_time(const char* file_path)
{
    struct stat s;
    
    if(stat(file_path, &s) != 0)
        return 0;
    
    return s.st_mtime;
}

inline static void du_set_file_time(const char* file_path, const time_t t)
{
    struct stat s;
    struct utimbuf newtime;

    if(stat(file_path, &s) != 0)
        return;

    newtime.actime  = s.st_atime;   /* keep atime unchanged */
    newtime.modtime = t;            /* set mtime to current time */
    utime(file_path, &newtime);
}

inline static const time_t du_values_to_epoch(const int year, const int month,
                                              const int day, const int hour,
                                              const int min, const int sec)
{
    if(year < 1970) return 0;
    
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if(month == 0)
        tm.tm_mon = 0;
    else
        tm.tm_mon = (month%13) - 1;

    if(day == 0)
        tm.tm_mday = 1;
    else
        tm.tm_mday = day;

    tm.tm_year = year - 1900;
    tm.tm_hour  = hour;
    tm.tm_min   = min;
    tm.tm_sec   = sec;

    return mktime(&tm);
}