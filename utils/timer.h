#ifndef TIMER_H
#define TIMER_H

#include <windows.h>

class Timer {
    LARGE_INTEGER start_time;
    LARGE_INTEGER frequency;

public:
    Timer();
    void start();\
    double stop(); // Returns milliseconds
};

#endif // TIMER_H