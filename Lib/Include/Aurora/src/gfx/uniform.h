#ifndef UNIFORM_H
#define UNIFORM_H

#include <string>
#include <vector>
#include <map>

#include "../math/math.h"

namespace Au{
namespace GFX{

template<typename T>
struct UniformStorage
{
    static std::vector<std::vector<T>> data;
    static std::map<std::string, unsigned int> name_index;
};

template<typename T>
std::vector<std::vector<T>> UniformStorage<T>::data;
template<typename T>
std::map<std::string, unsigned int> UniformStorage<T>::name_index;

class IUniformInterface
{
public:
    virtual ~IUniformInterface() {}
    virtual void Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) = 0;
};

template<typename T>
class UniformInterface : public IUniformInterface
{
public:
    void Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count);
};

template<>
inline void UniformInterface<float>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{ 
    glUniform1fv(location, count, (GLfloat*)&UniformStorage<float>::data[dataIndex][0]);
}
template<>
inline void UniformInterface<Math::Vec2f>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{ 
    glUniform2fv(
        location,
        count,
        (GLfloat*)&UniformStorage<Math::Vec2f>::data[dataIndex][0]
    ); 
}
template<>
inline void UniformInterface<Math::Vec3f>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{ 
    glUniform3fv(
        location,
        count,
        (GLfloat*)&UniformStorage<Math::Vec3f>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec4f>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform4fv(
        location,
        count,
        (GLfloat*)&UniformStorage<Math::Vec4f>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<int>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform1iv(
        location,
        count,
        (GLint*)&UniformStorage<int>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec2i>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform2iv(
        location,
        count,
        (GLint*)&UniformStorage<Math::Vec2i>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec3i>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform3iv(
        location,
        count,
        (GLint*)&UniformStorage<Math::Vec3i>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec4i>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform4iv(
        location,
        count,
        (GLint*)&UniformStorage<Math::Vec4i>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<unsigned int>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count)
{
    glUniform1uiv(
        location,
        count,
        (GLuint*)&UniformStorage<unsigned int>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec2ui>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform2uiv(
        location,
        count,
        (GLuint*)&UniformStorage<Math::Vec2ui>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec3ui>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform3uiv(
        location,
        count,
        (GLuint*)&UniformStorage<Math::Vec3ui>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Vec4ui>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniform4uiv(
        location,
        count,
        (GLuint*)&UniformStorage<Math::Vec4ui>::data[dataIndex][0]
    );
}
template<>
inline void UniformInterface<Math::Mat3f>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniformMatrix3fv(
        location,
        count,
        GL_FALSE,
        (GLfloat*)&UniformStorage<Math::Mat3f>::data[dataIndex][0][0]
    );
}
template<>
inline void UniformInterface<Math::Mat4f>::Upload(unsigned int location, unsigned int dataIndex, unsigned int offset, unsigned int count) 
{
    glUniformMatrix4fv(
        location, 
        count, 
        GL_FALSE, 
        (GLfloat*)&UniformStorage<Math::Mat4f>::data[dataIndex][0][0]
    );
}

// ======================================================

template<typename T>
class UniformInterfaceStorage
{
public:
    static UniformInterface<T> impl;
};

template<typename T>
UniformInterface<T> UniformInterfaceStorage<T>::impl;

// ======================================================

class Uniform
{
public:
    Uniform()
    : dataIndex(0), offset(0), count(1), impl(0) {}
    Uniform(unsigned dataIndex, unsigned offset, unsigned count, IUniformInterface* impl)
    : dataIndex(dataIndex), offset(offset), count(count), impl(impl) {}
    template<typename T>
    Uniform& operator=(const T& value);
    template<typename T>
    Uniform& operator=(const std::vector<T>& value);
    template<typename T>
    Uniform& Set(const T& value, unsigned id);
    Uniform operator[](int i)
    {
        return Uniform(dataIndex, i, 1, impl);
    }
    
    unsigned DataIndex() { return dataIndex; }
    
    void Upload(unsigned int location)
    {
        impl->Upload(location, dataIndex, offset, count);
    }
    
    bool operator<(const Uniform& other) const
    {
        return (impl + dataIndex) < (other.impl + other.dataIndex);
    }
private:
    unsigned dataIndex;
    unsigned offset;
    unsigned count;
    IUniformInterface* impl;
};

template<typename T>
Uniform& Uniform::operator=(const T& value)
{
    UniformStorage<T>::data[dataIndex][offset] = value;
    return *this;
}

template<typename T>
Uniform& Uniform::operator=(const std::vector<T>& value)
{
    for(unsigned i = 0; i < value.size() && i < count; ++i)
        UniformStorage<T>::data[dataIndex][i] = value[i];
    return *this;
}

template<typename T>
Uniform& Uniform::Set(const T& value, unsigned id)
{
    UniformStorage<T>::data[dataIndex][id] = value;
    return *this;
}

template<>
inline Uniform& Uniform::operator=(const Uniform& value)
{
    dataIndex = value.dataIndex;
    offset = value.offset;
    count = value.count;
    impl = value.impl;
}

template<typename T>
Uniform GetUniform(const std::string& name, unsigned int count = 1)
{
    std::map<std::string, unsigned int>::iterator it;
    it = UniformStorage<T>::name_index.find(name);
    if (it != UniformStorage<T>::name_index.end())
    {
        if(UniformStorage<T>::data[it->second].size() < count)
            UniformStorage<T>::data[it->second].resize(count);
        return Uniform(it->second, 0, count, &UniformInterfaceStorage<T>::impl);
    }
    else
    {
        unsigned int id = UniformStorage<T>::data.size();
        UniformStorage<T>::name_index.insert(std::make_pair(name, id));
        UniformStorage<T>::data.push_back(std::vector<T>(count));
        //names.push_back(name);
        return Uniform(id, 0, count, &UniformInterfaceStorage<T>::impl);
    }
}

}
}

#endif
