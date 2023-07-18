#include <stdint.h>
#include <time.h>

#include "system.h"

static uint64_t time_seconds, last_seconds;
double get_time(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    if (last_seconds < ts.tv_sec || (ts.tv_sec < 10000 && last_seconds > 10000)) {
        last_seconds = ts.tv_sec;
        time_seconds++;
    }
    return (double)time_seconds + (double)ts.tv_nsec / 1000000000.0;
}
