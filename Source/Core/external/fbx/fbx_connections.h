#ifndef FBX_CONNECTIONS_H
#define FBX_CONNECTIONS_H

#include "fbx_connection.h"

class FbxConnections
{
public:
    void Add(const FbxConnection& conn) {
        connections.emplace_back(conn);
    }

    size_t CountChildrenOO(int64_t parent) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == "OO")
                ++c;
        }
        return c;
    }

    int64_t GetChildOO(int64_t parent, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == "OO") {
                if(index == c)
                    return conn.child_uid;
                ++c;
            }
        }
        return -1;
    }

    size_t CountChildrenOP(int64_t parent) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == "OP")
                ++c;
        }
        return c;
    }

    int64_t GetChildOP(int64_t parent, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == "OP") {
                if(index == c)
                    return conn.child_uid;
                ++c;
            }
        }
        return -1;
    }

    FbxConnection* GetChildOPConnection(int64_t parent, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == "OP") {
                if(index == c)
                    return &conn;
                ++c;
            }
        }
        return 0;
    }

    int64_t FindObjectToObjectParent(int64_t uid) {
        for(auto& conn : connections){
            if(conn.child_uid == uid && conn.type == "OO")
                return conn.parent_uid;
        }
        return -1;
    }
    int64_t FindObjectToObjectChild(int64_t uid) {
        for(auto& conn : connections){
            if(conn.parent_uid == uid && conn.type == "OO")
                return conn.child_uid;
        }
        return -1;
    }
private:
    std::vector<FbxConnection> connections;
};

#endif
