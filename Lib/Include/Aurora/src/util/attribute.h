#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

#include "typeinfo.h"

#define AU_DEF_ATTRIB(NAME, ELEMTYPE, ELEMCOUNT) \
    class NAME{ \
    public: \
        operator unsigned int() const { return GetAttribIndex<NAME>(); } \
        operator AttribInfo() const \
        { \
            AttribInfo info; \
            info.attribIndex = GetAttribIndex<NAME>(); \
            info.typeIndex = TypeInfo<NAME>::Index(); \
            info.elemType = TypeInfo<ELEMTYPE>::Index(); \
            info.elemSize = sizeof(ELEMTYPE); \
            info.elemCount = ELEMCOUNT; \
            info.name = #NAME; \
            return info; \
        } \
        AttribFormat operator<<(const AttribInfo& other) \
        { \
            AttribFormat result; \
            result << (*this); \
            result << (other); \
            return result; \
        } \
        operator std::vector<AttribInfo>() const \
        { \
            std::vector<AttribInfo> result; \
            result.push_back(*this); \
            return result; \
        } \
        operator AttribFormat() const \
        { \
            AttribFormat result; \
            result << (*this); \
            return result; \
        } \
        ELEMTYPE& operator[](unsigned i) \
        { return data[i]; } \
        ELEMTYPE data[ELEMCOUNT]; \
    }

namespace Au{

inline unsigned int NextAttribIndex()
{
    static unsigned int index = 0;
    return index++;
}

template<typename A>
int GetAttribIndex()
{
    static unsigned int index = NextAttribIndex();
    return index;
}

// Global attribute index
typedef unsigned int AttribIndex;
// A number that tells you which one of the same attribute type
// of the same vertex format this is
typedef unsigned int AttribInstance;

struct AttribInfo
{
    AttribIndex attribIndex;
    typeindex typeIndex;
    std::string name;
    typeindex elemType;
    unsigned char elemSize;
    unsigned char elemCount;
    
    bool operator==(const AttribInfo& other) const
    {
        return typeIndex == other.typeIndex;
    }
    
    operator unsigned int() const
    {
        return attribIndex;
    }
    
    bool operator<(const AttribInfo& other) const
    {
        return typeIndex < other.typeIndex;
    }
    
    void Print() const
    {
        std::cout << name << ": " << std::endl;
        std::cout << "attribIndex: " << attribIndex << std::endl;
        std::cout << "typeIndex: " << typeIndex << std::endl;
        std::cout << "elemType: " << elemType << std::endl;
        std::cout << "elemSize: " << (int)elemSize << std::endl;
        std::cout << "elemCount: " << (int)elemCount << std::endl;
    }
};

class AttribFormat
{
public:
    AttribFormat() {}
    AttribFormat(const std::vector<AttribInfo>& fmt)
    : attrFmt(fmt.begin(), fmt.end()) {}

    bool operator==(const AttribFormat& other)
    {
        if(attrFmt.size() != other.attrFmt.size())
            return false;
        for(unsigned i = 0; i < attrFmt.size(); ++i)
        {
            if((unsigned int)attrFmt[i] != (unsigned int)other.attrFmt[i])
                return false;
        }
        
        return true;
    }
    
    bool operator!=(const AttribFormat& other)
    {
        return !operator==(other);
    }

    operator std::vector<AttribInfo>()
    {
        return attrFmt;
    }
    
    AttribFormat& operator<<(const AttribInfo& right)
    {
        attrFmt.push_back(right);
        std::sort(attrFmt.begin(), attrFmt.end());
        return *this;
    }
    
    void Print() const
    {
        for(unsigned i = 0; i < attrFmt.size(); ++i)
        {
            attrFmt.at(i).Print();
        }
    }
    
private:
    std::vector<AttribInfo> attrFmt;
};

AttribIndex GetGlobalAttribIndex(typeindex type, unsigned int instance);

AU_DEF_ATTRIB(Empty, float, 1);
    
AU_DEF_ATTRIB(Position, float, 3);

AU_DEF_ATTRIB(Normal, float, 3);
AU_DEF_ATTRIB(Tangent, float, 3);
AU_DEF_ATTRIB(Bitangent, float, 3);

AU_DEF_ATTRIB(UV, float, 2);
AU_DEF_ATTRIB(UVW, float, 3);

AU_DEF_ATTRIB(ColorRGBA, float, 4);
AU_DEF_ATTRIB(ColorRGB, float, 3);

AU_DEF_ATTRIB(BoneWeight4, float, 4);
AU_DEF_ATTRIB(BoneIndex4, float, 4);

}

#endif
