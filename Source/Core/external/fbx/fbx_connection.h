#ifndef FBX_CONNECTION_H
#define FBX_CONNECTION_H

#include "fbx_node.h"

class FbxConnection
{
public:
    FbxConnection() {}
    FbxConnection(FbxNode& node)
    {
        type = node.GetProperty(0).GetString();
        child_uid = node.GetProperty(1).GetInt64();
        parent_uid = node.GetProperty(2).GetInt64();
        
        if(node.PropCount() > 3)
        {
            name = node.GetProperty(3).GetString();
        }
    }

    std::string type;
    int64_t child_uid;
    int64_t parent_uid;    
    std::string name;
};

#endif
