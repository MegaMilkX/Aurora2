#ifndef SCOPED_TIMER_H
#define SCOPED_TIMER_H

#include <iostream>
#include <string>
#include <aurora/timer.h>

class ScopedTimer
{
public:
    ScopedTimer()
    : ScopedTimer("Noname") {}
    ScopedTimer(const std::string name)
    : name(name)
    {
        timer.Start();
    }
    ~ScopedTimer()
    {
        std::cout << name <<  ": " << timer.End() / 1000000.0f << std::endl;
    }
private:
    std::string name;
    Au::Timer timer;
};

#endif
