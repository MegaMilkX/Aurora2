#ifndef AU_LUA_VAL_H
#define AU_LUA_VAL_H

namespace Au{
    
template<typename T> struct bare_type { typedef T type; };
template<typename T> struct bare_type<const T> : bare_type<T> {};
template<typename T> struct bare_type<T&> : bare_type<T> {};
template<typename T> struct bare_type<T&&> : bare_type<T> {};
template<typename T> struct remove_ptr { typedef T type; };
template<typename T> struct remove_ptr<T*> : remove_ptr<T> {};

template<typename T>
void _deleteFn(void* data)
{
    delete ((T*)data);
}

class LuaValue
{
public:
    LuaValue()
    : _data(0), _delete(0) {}
    
    void Free()
    {
        if(_delete) _delete(_data);
        _delete = 0;
    }

    template<typename T>
    LuaValue& operator=(const T& value)
    {
        _data = new T(value);
        _delete = &_deleteFn<T>;
        return *this;
    }
    
    template<typename T>
    LuaValue& operator=(T* value)
    {
        _data = value;
        _delete = 0;
        return *this;
    }
    
    template<typename T>
    struct Getter
    {
        static T& Get(void* data)
        {
            return *(T*)data;
        }
    };
    
    template<typename T>
    struct Getter<T*>
    {
        static T*& Get(void*& data)
        {
            return (T*&)data;
        }
    };
    
    template<typename T>
    T& Get()
    { return Getter<bare_type<T>::type>::Get(_data); }
private:
    void (*_delete)(void*);
    void* _data;
};

}

#endif
