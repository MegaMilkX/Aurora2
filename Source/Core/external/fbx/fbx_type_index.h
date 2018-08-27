#ifndef FBX_TYPE_INDEX_H
#define FBX_TYPE_INDEX_H

typedef int FbxTypeIndex;

template<typename T>
struct FbxTypeInfo
{
    static FbxTypeIndex Index()
    {
        static FbxTypeIndex id = FbxTypeInfo_NewId();
        return id;
    }
private:
};

template<typename T>
FbxTypeIndex FbxGetTypeIndex(T value)
{
    return FbxTypeInfo<T>::Index();
}

inline FbxTypeIndex FbxTypeInfo_NewId()
{
    static FbxTypeIndex id;
    return ++id;
}

#endif
