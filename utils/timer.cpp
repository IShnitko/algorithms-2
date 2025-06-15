#include "timer.h"

Timer::Timer() {
    QueryPerformanceFrequency(&frequency);
}

void Timer::start() {
    QueryPerformanceCounter(&start_time);
}

double Timer::stop() {
    LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    return (end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
}