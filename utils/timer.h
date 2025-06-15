#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

class Timer {
    LARGE_INTEGER start_time;
    LARGE_INTEGER frequency;

public:
    Timer() {
        QueryPerformanceFrequency(&frequency);
    }

    void start() {
        QueryPerformanceCounter(&start_time);
    }

    double stop() {
        LARGE_INTEGER end_time;
        QueryPerformanceCounter(&end_time);
        return (end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
    }
};

#endif // TIMER_H