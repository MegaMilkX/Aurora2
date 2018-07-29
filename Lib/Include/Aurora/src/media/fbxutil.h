#ifndef AU_FBXUTIL_H
#define AU_FBXUTIL_H

#include "../math/math.h"

#include "fbxsettings.h"

namespace Au{
namespace Media{
namespace FBX{

inline bool IsSameAxisDirection(Axis a, Axis b)
{
    if(a == AXIS_X && b == AXIS_X)
        return true;
    else if(a == AXIS_Y && b == AXIS_Y)
        return true;
    else if(a == AXIS_Z && b == AXIS_Z)
        return true;
    return false;
}
    
inline bool IsSameAxis(Axis a, Axis b)
{
    if((a == AXIS_X || a == AXIS_MX) &&
       (b == AXIS_X || b == AXIS_MX))
        return true;
    else if((a == AXIS_Y || a == AXIS_MY) &&
       (b == AXIS_Y || b == AXIS_MY))
       return true;
    else if((a == AXIS_Z || a == AXIS_MZ) &&
       (b == AXIS_Z || b == AXIS_MZ))
       return true;
    else
        return false;
}
    
inline void GetConvertedAxisIndices(Settings& settings, unsigned (&indices)[3], float (&mults)[3])
{
    Axis origAxes[3] = { 
        settings.origAxes.right,
        settings.origAxes.up,
        settings.origAxes.front
    };
    Axis convAxes[3] = {
        settings.convAxes.right,
        settings.convAxes.up,
        settings.convAxes.front
    };
    
    for(unsigned i = 0; i < 3; ++i)
    {
        for(unsigned j = 0; j < 3; ++j)
        {
            if(IsSameAxis(origAxes[i], convAxes[j]))
            {
                indices[i] = j;
                if(!IsSameAxisDirection(origAxes[i], convAxes[j]))
                    mults[j] = -1.0f;
            }
        }
    }
}
    
inline void ConvertVector(Settings& settings, Au::Math::Vec3f& vec, bool dir = true)
{
    unsigned indices[3] = { 0, 0, 0 };
    float multipliers[3] = { 1.0f, 1.0f, 1.0f };
    
    GetConvertedAxisIndices(settings, indices, multipliers);
    
    float newVec[3];
    for(unsigned i = 0; i < 3; ++i)
        newVec[indices[i]] = vec[i] * (dir ? multipliers[i] : 1.0f);
    vec.x = newVec[0];
    vec.y = newVec[1];
    vec.z = newVec[2];
}

inline void ConvertMatrix(Settings& settings, Au::Math::Mat4f& m)
{
    unsigned indices[3];
    float multipliers[3] = { 1.0f, 1.0f, 1.0f };
    
    GetConvertedAxisIndices(settings, indices, multipliers);
    
    Au::Math::Vec4f newRotVec[3];
    for(unsigned i = 0; i < 3; ++i)
        newRotVec[indices[i]] = m[i];
    m[0] = newRotVec[0];
    m[1] = newRotVec[1];
    m[2] = newRotVec[2];
    
    // Position
    
    Au::Math::Vec3f pos = m[3];
    ConvertVector(settings, pos);
    m[3].x = pos.x;
    m[3].y = pos.y;
    m[3].z = pos.z;
}

}
}
}

#endif
