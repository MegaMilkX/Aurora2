#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#define NO_MIN_MAX
#include <windows.h>

class critical_section
{
public:
    critical_section() { InitializeCriticalSection(&m_cs); }
    ~critical_section() { DeleteCriticalSection(&m_cs); }

    void lock() { EnterCriticalSection(&m_cs); }
    void unlock() { LeaveCriticalSection(&m_cs); }

private:
    CRITICAL_SECTION m_cs;
};

#endif
