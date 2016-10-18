#ifndef __HET_TIME_H__
#define __HET_TIME_H__
#include <Winsock2.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 


int gettimeofday(struct timeval * tp, struct timezone * tzp);
double getTimestamp();
double getCurrentTimestamp();
#endif //__HET_TIME_H__