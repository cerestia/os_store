#ifndef ONIX_TYPE_H
#define ONIX_TYPE_H

#include<onix/types.h>

typedef struct tm{
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
} tm;

void time_read_bcd(tm* time);
void time_read(tm* time);
time_t mktime(tm* time);

#endif