#include "bedrock.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
double bedrock_time(void) {
  static int initialized = 0;
  static uint64_t freq, start;
  uint64_t curr;

  if (!initialized) {
    initialized = 1;
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    QueryPerformanceCounter((LARGE_INTEGER *)&start);
  }
  QueryPerformanceCounter((LARGE_INTEGER *)&curr);

  return (double)(curr - start) / freq;
}
#else
#include <time.h>
double bedrock_time(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (float)ts.tv_sec + ((float)ts.tv_nsec / 1000000000.0f);
}
#endif
