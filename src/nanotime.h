#ifndef PPC_NANOTIME_H
#define PPC_NANOTIME_H

#include <stdint.h>
#include <time.h>

// Unsigned because I doubt this program will be executed before 1970.
// (Even after considering uninitialised or incorrectly set clocks.)
typedef uint64_t nanotime;

#define PPC_NANOSECONDS_IN_SECOND 1000000000ULL

nanotime
nanotime_from_timespec(const struct timespec *t) {
  return t->tv_sec * PPC_NANOSECONDS_IN_SECOND + t->tv_nsec;
}

double
nanotime_seconds(nanotime n) {
  return n / (double)PPC_NANOSECONDS_IN_SECOND;
}

#endif // ifndef PPC_NANOTIME_H
