#ifndef FBX_CONNECTION_2_H
#define FBX_CONNECTION_2_H

#include "fbx_object.h"
#include "fbx_log.h"
#include "fbx_macro.h"
#include <map>

namespace Fbx {

enum CONNECTION_TYPE
{
    CONN_UNKNOWN,
    OBJECT_OBJECT,
    OBJECT_PROPERTY  
};

class Connection : public BasicObject {
public:
    virtual bool Make(Node& node) {
        type = _typeStrToType(node.GetProperty(0).GetString());
        child_uid = node.GetProperty(1).GetInt64();
        parent_uid = node.GetProperty(2).GetInt64();
        if(node.PropCount() > 3) {
            name = node.GetProperty(3).GetString();
        }
        return true;
    }

    static bool IdentifyNode(Node& node) {
        if(node.GetName() != "C"
            || node.PropCount() < 3
            || !node.GetProperty(0).IsString()
            || !node.GetProperty(1).IsInt64()
            || !node.GetProperty(2).IsInt64()) 
            return false;
        return true;
    }

    CONNECTION_TYPE type;
    int64_t child_uid;
    int64_t parent_uid;    
    std::string name;
private:
    static CONNECTION_TYPE _typeStrToType(const std::string& type) {
        static std::map<std::string, CONNECTION_TYPE> map = {
            { "OO", OBJECT_OBJECT },
            { "OP", OBJECT_PROPERTY }
        };
        return map[type];
    }
};

}

#endif