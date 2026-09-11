#ifndef NC_VALIDATION_H
#define NC_VALIDATION_H
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
/* Enable before process launch with NC_VALIDATION=1. stderr preserves raw logs. */
static inline uint64_t ncv_now(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}
static inline void ncv_event(const char *event, uint64_t frame, unsigned channel,
                             int network, uint64_t latency, long detail) {
    const char *enabled = getenv("NC_VALIDATION");
    if (!enabled || enabled[0] != '1') return;
    fprintf(stderr, "{\"timestamp_us\":%llu,\"event\":\"%s\",\"frame_id\":%llu,"
        "\"channel\":%u,\"network_id\":%d,\"latency_us\":%llu,\"detail\":%ld}\n",
        (unsigned long long)ncv_now(), event, (unsigned long long)frame,
        channel, network, (unsigned long long)latency, detail);
}
#endif
