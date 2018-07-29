#ifndef _DATAHANDLE_H_
#define _DATAHANDLE_H_

#include <vector>
#include <string>

template<typename DATA>
class Handle
{
public:
    Handle() : handle(0){}
    DATA* operator->();
	DATA& operator*();
    void Release();

    static Handle<DATA> Acquire();
    static Handle<DATA> Acquire(DATA value);
    static void Replace(Handle<DATA> handle, const DATA& value);

    operator int()
    {
        return handle;
    }
private:
    Handle(unsigned int index) : index(index), magic(++Handle<DATA>::magic_next){}
    union
    {
        struct
        {
            unsigned int index : 16;
            unsigned int magic : 16;
        };
        unsigned int handle;
    };

    //
    static std::vector<DATA> data_storage;
    static std::vector<unsigned int> magic_storage;
    static std::vector<unsigned int> free_storage;
    static unsigned int magic_next;
};

template<typename DATA>
std::vector<DATA> Handle<DATA>::data_storage;
template<typename DATA>
std::vector<unsigned int> Handle<DATA>::magic_storage;
template<typename DATA>
std::vector<unsigned int> Handle<DATA>::free_storage;
template<typename DATA>
unsigned int Handle<DATA>::magic_next = 0;

template<typename DATA>
DATA* Handle<DATA>::operator->()
{
    return &(data_storage[index]);
}

template<typename DATA>
DATA& Handle<DATA>::operator*()
{
	return (data_storage[index]);
}

template<typename DATA>
void Handle<DATA>::Release()
{
    if (magic == 0)
        return;
    if (Handle<DATA>::magic_storage[index] != magic)
        return;
    Handle<DATA>::magic_storage[index] = 0;
    Handle<DATA>::free_storage.push_back(index);
    //Handle<DATA>::data_storage[index].Destroy();
    Handle<DATA>::data_storage[index] = DATA();
}

template<typename DATA>
Handle<DATA> Handle<DATA>::Acquire()
{
    Handle<DATA> resource;
    unsigned int index;
    if (free_storage.empty())
    {
        index = data_storage.size();
        data_storage.push_back(DATA());
        resource = Handle<DATA>(index);
        magic_storage.push_back(resource.magic);
    }
    else
    {
        index = free_storage.back();
        free_storage.pop_back();
        resource = Handle<DATA>(index);
        magic_storage[index] = resource.magic;
    }

    return resource;
}

template<typename DATA>
Handle<DATA> Handle<DATA>::Acquire(DATA value)
{
    Handle<DATA> resource;
    unsigned int index;
    if (free_storage.empty())
    {
        index = data_storage.size();
        data_storage.push_back(value);
        resource = Handle<DATA>(index);
        magic_storage.push_back(resource.magic);
    }
    else
    {
        index = free_storage.back();
        free_storage.pop_back();
        resource = Handle<DATA>(index);
        data_storage[index] = value;
        magic_storage[index] = resource.magic;
    }

    return resource;
}

template<typename DATA>
void Handle<DATA>::Replace(Handle<DATA> handle, const DATA& value)
{
    data_storage[handle.index] = value;
}

#endif