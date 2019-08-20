#include "timer.h"
#include <map>
#include <chrono>

namespace timer
{
    namespace
    {
    std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    std::chrono::time_point<std::chrono::high_resolution_clock> _end;
    }

    void start()
    {
        _start = std::chrono::high_resolution_clock::now();
    }

    void stop()
    {
        _end = std::chrono::high_resolution_clock::now();
    }

    u32 seconds()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(_end - _start).count();
    }

    u32 milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(_end - _start).count();
    }

    u32 microseconds()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>(_end - _start).count();
    }
}
