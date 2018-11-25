#ifndef FBX_CONNECTIONS_H
#define FBX_CONNECTIONS_H

#include "fbx_connection.h"

class FbxConnections
{
public:
    void Add(const FbxConnection& conn) {
        connections.emplace_back(conn);
    }

    size_t CountChildren(FBX_CONNECTION_TYPE type, int64_t parent) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == type)
                ++c;
        }
        return c;
    }

    int64_t GetChild(FBX_CONNECTION_TYPE type, int64_t parent, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == type) {
                if(index == c)
                    return conn.child_uid;
                ++c;
            }
        }
        return -1;
    }

    size_t CountParents(FBX_CONNECTION_TYPE type, int64_t child) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.child_uid == child && conn.type == type)
                ++c;
        }
        return c;
    }

    int64_t GetParent(FBX_CONNECTION_TYPE type, int64_t child, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.child_uid == child && conn.type == type) {
                if(index == c)
                    return conn.parent_uid;
                ++c;
            }
        }
        return -1;
    }

    FbxConnection* GetChildConnection(FBX_CONNECTION_TYPE type, int64_t parent, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == type) {
                if(index == c)
                    return &conn;
                ++c;
            }
        }
        return 0;
    }

    int64_t FindObjectToObjectParent(int64_t uid) {
        for(auto& conn : connections){
            if(conn.child_uid == uid && conn.type == FBX_OBJECT_OBJECT)
                return conn.parent_uid;
        }
        return -1;
    }
    int64_t FindObjectToObjectChild(int64_t uid) {
        for(auto& conn : connections){
            if(conn.parent_uid == uid && conn.type == FBX_OBJECT_OBJECT)
                return conn.child_uid;
        }
        return -1;
    }
private:
    std::vector<FbxConnection> connections;
};

#endif
