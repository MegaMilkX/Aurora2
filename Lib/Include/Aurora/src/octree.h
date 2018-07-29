#ifndef OCTREE_H
#define OCTREE_H

#include <vector>

#include "math/math.h"

namespace Au{

struct OctObject
{
    
};

class OctCell
{
public:
    OctCell()
    : children() {}
    bool Insert(const OctObject& object)
    {
        if(Math::Intersects(aabb, object.aabb))
        {
            
        }
        else
            return false;
    }
private:
    void _createChildren()
    {
        for(unsigned int i = 0; i < 8; ++i)
            children[i] = new OctCell();
    }
    void _deleteChildren()
    {
        for(unsigned int i = 0; i < 8; ++i)
            delete children[i];
    }
    Math::AABB3f aabb;
    std::vector<OctObject> objects;
    OctCell* children[8];
};

class Octree
{
public:
    Octree()
    : rootCell(0) {}
    void Insert(const OctObject& object)
    {
        while(!rootCell->Insert(object))
            Expand(object);
    }
    void Expand(const OctObject& object)
    {
        
    }
private:
    OctCell* rootCell;
};

}

#endif
