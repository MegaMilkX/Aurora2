#ifndef AU_TIMER_H
#define AU_TIMER_H

#include <windows.h>
#include <stdint.h>

namespace Au{
    
class Timer
{
public:
    Timer()
    {
        QueryPerformanceFrequency(&freq);
    }

    void Start()
    {
        QueryPerformanceCounter(&start);
    }
    
    int64_t End()
    {
        QueryPerformanceCounter(&end);
        elapsed.QuadPart = end.QuadPart - start.QuadPart;
        elapsed.QuadPart *= 1000000;
        elapsed.QuadPart /= freq.QuadPart;
        return elapsed.QuadPart;
    }
private:
    LARGE_INTEGER freq;
    LARGE_INTEGER start, end;
    LARGE_INTEGER elapsed;
};
    
}

#endif
