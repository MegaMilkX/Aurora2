#ifndef FBX_TYPE_INDEX_2_H
#define FBX_TYPE_INDEX_2_H

namespace Fbx {

typedef int TypeIndex;

template<typename T>
struct TypeInfo
{
    static TypeIndex Index()
    {
        static TypeIndex id = TypeInfo_NewId();
        return id;
    }
private:
};

template<typename T>
TypeIndex FbxGetTypeIndex(T value)
{
    return TypeInfo<T>::Index();
}

inline TypeIndex TypeInfo_NewId()
{
    static TypeIndex id;
    return ++id;
}

}

#endif
