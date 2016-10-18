#include "get_time.h"

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}

double getTimestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_usec + tv.tv_sec*1e6;
}


// High-resolution timer.
double getCurrentTimestamp() {

	// Use the high-resolution performance counter.

	static LARGE_INTEGER ticks_per_second = {};
	if (ticks_per_second.QuadPart == 0) {
		// First call - get the frequency.
		QueryPerformanceFrequency(&ticks_per_second);
	}

	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);

	double seconds = double(counter.QuadPart) / double(ticks_per_second.QuadPart);
	return seconds;

}