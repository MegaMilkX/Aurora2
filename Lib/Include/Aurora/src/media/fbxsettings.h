#ifndef AU_FBX_SETTINGS_H
#define AU_FBX_SETTINGS_H

#include <iostream>
#include <string>

#include "fbxnode.h"

namespace Au{
namespace Media{
namespace FBX{
    
enum CoordSystem
{
    ORIGINAL,
    OPENGL,
    DIRECT3D
};

enum Axis
{
    AXIS_UNKNOWN = 3,
    AXIS_X = 0, AXIS_Y = 1, AXIS_Z = 2,
    AXIS_MX = -1, AXIS_MY = -2, AXIS_MZ = -3
};

inline bool IsSameAxisSign(Axis a, Axis b)
{
    return (((int)a < 0) == ((int)b < 0));
}

inline std::string AxisToStr(Axis axis)
{
    if(axis == AXIS_X)
        return "x";
    else if(axis == AXIS_Y)
        return "y";
    else if(axis == AXIS_Z)
        return "z";
    else if(axis == AXIS_MX)
        return "-x";
    else if(axis == AXIS_MY)
        return "-y";
    else if(axis == AXIS_MZ)
        return "-z";
    else
        return "unknown";
}

struct AxisSetup
{
    Axis right;
    Axis up;
    Axis front;
    
    void Print()
    {
        std::cout << "right: " << AxisToStr(right) << std::endl;
        std::cout << "up: " << AxisToStr(up) << std::endl;
        std::cout << "front: " << AxisToStr(front) << std::endl;
    }
};

struct Settings
{
    double scaleFactor;
    Au::Math::Mat4f scaleMat;
    AxisSetup origAxes;
    AxisSetup convAxes;
    
    Au::Math::Mat4f convMatrix;
    
    void Init(Node& rootNode)
    {
        Node* unitScaleFactorNode = 
            rootNode.Get("Properties70", 0).GetWhere(0, "UnitScaleFactor");
        scaleFactor = (*unitScaleFactorNode)[4].GetDouble() * 0.01;
        scaleMat = Au::Math::Mat3f((float)scaleFactor);
        BuildConversionMatrix(rootNode);
    }
    
    void BuildConversionMatrix(Node& rootNode)
    {
        Au::Math::Mat4f defaultMatrix(1.0f);
        convMatrix = Au::Math::Mat4f(1.0f);
        
        Node* axis = rootNode.Get("Properties70", 0).GetWhere(0, "UpAxis");
        if(axis)
        {
            int a = (int)(*axis)[4].GetInt32();
            convMatrix[1] = defaultMatrix[a];
        }
        Node* axisSign = rootNode.Get("Properties70", 0).GetWhere(0, "UpAxisSign");
        if(axisSign)
        {
            int sign = (int)(*axisSign)[4].GetInt32();
            if(sign != 1)
                convMatrix[1] = -convMatrix[1];
        }
        
        axis = rootNode.Get("Properties70", 0).GetWhere(0, "FrontAxis");
        if(axis)
        {
            int a = (int)(*axis)[4].GetInt32();
            convMatrix[2] = defaultMatrix[a];
        }
        axisSign = rootNode.Get("Properties70", 0).GetWhere(0, "FrontAxisSign");
        if(axisSign)
        {
            int sign = (int)(*axisSign)[4].GetInt32();
            if(sign != 1)
                convMatrix[2] = -convMatrix[2];
        }
        
        axis = rootNode.Get("Properties70", 0).GetWhere(0, "CoordAxis");
        if(axis)
        {
            int a = (int)(*axis)[4].GetInt32();
            convMatrix[0] = defaultMatrix[a];
        }
        axisSign = rootNode.Get("Properties70", 0).GetWhere(0, "CoordAxisSign");
        if(axisSign)
        {
            int sign = (int)(*axisSign)[4].GetInt32();
            if(sign != 1)
                convMatrix[0] = -convMatrix[0];
        }
        
        axis = rootNode.Get("Properties70", 0).GetWhere(0, "OriginalUpAxis");
        if(axis)
        {
            int a = (int)(*axis)[4].GetInt32();
            convMatrix[a] = convMatrix[1];
            convMatrix[1] = -defaultMatrix[a];
        }
        axisSign = rootNode.Get("Properties70", 0).GetWhere(0, "OriginalUpAxisSign");
        if(axisSign)
        {
            int sign = (int)(*axisSign)[4].GetInt32();
            if(sign != 1)
                convMatrix[1] = -convMatrix[1];
        }
    }
};
 
}
}
}

#endif
