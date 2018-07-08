#ifndef BINARY_DATA_H
#define BINARY_DATA_H

#include <vector>
#include <rttr/type>

class binary_data
{
public:
    binary_data()
    {
        _cursor = 0;
        _end = 0;
    }
    binary_data(unsigned char* data, size_t sz)
    : _data(data, data + sz)
    {
        _cursor = _data.data();
        _end = _data.data() + sz;
    }

    template<typename T>
    bool read(T& out)
    {
        if(offset(_cursor, _end) < sizeof(T))
            return false;
    
        out = *reinterpret_cast<const T*>(_cursor);
        
        _cursor += sizeof(T);
        
        return true;
    }
    template<typename T>
    void write(const T& data)
    {
        _data.insert(_data.end(), (unsigned char*)&data, (unsigned char*)&data + sizeof(T));
        _cursor = _data.data();
        _end = _data.data() + _data.size();
    }
    
    template<typename T>
    void write(const T* data, size_t count)
    {
        _data.insert(_data.end(), (unsigned char*)data, (unsigned char*)data + count * sizeof(T));
        _cursor = _data.data();
        _end = _data.data() + _data.size();
    }
    unsigned char* cursor() const { return _cursor; }
    unsigned char* ptr() { return _data.data(); }
    size_t size() const { return _data.size(); }
private:
    uint32_t offset(const unsigned char* begin, const unsigned char* cursor)
    {
        char* _begin = (char*)begin;
        char* _cursor = (char*)cursor;
        return static_cast<unsigned int>(_cursor - _begin);
    }
    unsigned char* _cursor;
    std::vector<unsigned char> _data;
    unsigned char* _end;
};

#endif
