#include "timer.h"

// Constructor: initialize the high-resolution timer frequency
Timer::Timer() {
    QueryPerformanceFrequency(&frequency);
}

// Start the timer by capturing the current counter value
void Timer::start() {
    QueryPerformanceCounter(&start_time);
}

// Stop the timer and return elapsed time in milliseconds
double Timer::stop() {
    LARGE_INTEGER end_time;
    QueryPerformanceCounter(&end_time);
    // Calculate elapsed time in milliseconds
    return (end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
}
