#ifndef FBX_CONNECTION_H
#define FBX_CONNECTION_H

#include "fbx_node.h"

enum FBX_CONNECTION_TYPE
{
    UNKNOWN,
    FBX_OBJECT_OBJECT,
    FBX_OBJECT_PROPERTY  
};

class FbxConnection
{
public:
    FbxConnection() {}
    FbxConnection(FbxNode& node)
    {
        std::string type_str = node.GetProperty(0).GetString();
        type = _typeStrToType(type_str);
        child_uid = node.GetProperty(1).GetInt64();
        parent_uid = node.GetProperty(2).GetInt64();
        
        if(node.PropCount() > 3)
        {
            name = node.GetProperty(3).GetString();
        }
    }

    FBX_CONNECTION_TYPE type;
    int64_t child_uid;
    int64_t parent_uid;    
    std::string name;
private:
    static FBX_CONNECTION_TYPE _typeStrToType(const std::string& type) {
        static std::map<std::string, FBX_CONNECTION_TYPE> map = {
            { "OO", FBX_OBJECT_OBJECT },
            { "OP", FBX_OBJECT_PROPERTY }
        };
        return map[type];
    }
};

#endif
