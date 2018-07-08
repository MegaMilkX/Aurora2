#ifndef DOUBLE_BUFFER_H
#define DOUBLE_BUFFER_H

#include <iostream>
#include <string>

template<typename T>
class double_buffer
{
public:
    double_buffer()
    : front(new T()), back(new T())
    { std::cout << "double_buffer()" << std::endl; }
    double_buffer(const double_buffer& other)
    : front(new T(*other.front)), back(new T(*other.back))
    { std::cout << "double_buffer(const double_buffer& other)" << std::endl; }
    double_buffer(const T& value)
    : front(new T(value)), back(new T(value))
    { std::cout << "double_buffer(const T& value)" << std::endl; }
    ~double_buffer()
    {
        std::cout << "~double_buffer()" << std::endl;
        clear();
    }
    void operator=(const double_buffer& other)
    {
        clear();
        front = new T(*other.front);
        back = new T(*other.back);
    }
    void operator=(const T& value)
    {
        *back = value;
    }
    operator T()
    {
        return *front;
    }
    T& get()
    {
        return *front;
    }
    void swap()
    {
        T* tmp = front;
        front = back;
        back = tmp;
    }
private:
    void clear()
    {
        delete front;
        delete back;
    }
    T* front;
    T* back;
};

#endif
