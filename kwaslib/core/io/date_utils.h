#pragma once

#include <time.h>
#include <utime.h>
#include <string.h>
#include <sys/stat.h>

/*
    Returns the unix timestamp of the file path.
*/
inline static const time_t du_get_file_time(const char* file_path)
{
    struct stat s;
    
    if(stat(file_path, &s) != 0)
        return 0;
    
    return s.st_mtime;
}

/*
    Sets the modified date of the file to passed unix epoch.
*/
inline static void du_set_file_time(const char* file_path, const time_t t)
{
    struct stat s;
    struct utimbuf newtime;

    if(stat(file_path, &s) != 0)
        return;

    newtime.actime  = s.st_atime;   /* access time unchanged */
    newtime.modtime = t;
    utime(file_path, &newtime);
}

/*
    Converts values to unix epoch.
    Returns epoch as time_t.
*/
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

/*
    Converts the epoch to machine's local time and fills the passed arguments.
    Any argument can be NULL and won't be written to.
*/
inline static void du_epoch_to_values(const time_t epoch,
                                      int* year, int* month,
                                      int* day, int* hour,
                                      int* min, int* sec)
{
    /*struct tm* tm = gmtime(&epoch);*/
    struct tm* tm = localtime(&epoch);
    if(year)    (*year)  = tm->tm_year + 1900;
    if(month)   (*month) = tm->tm_mon + 1;
    if(day)     (*day)   = tm->tm_mday;
    if(hour)    (*hour)  = tm->tm_hour;
    if(min)     (*min)   = tm->tm_min;
    if(sec)     (*sec)   = tm->tm_sec;
}
