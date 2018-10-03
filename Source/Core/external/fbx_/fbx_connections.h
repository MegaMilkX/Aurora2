#ifndef FBX_CONNECTIONS_2_H
#define FBX_CONNECTIONS_2_H

#include "fbx_connection.h"

namespace Fbx {

class ConnectionContainer
{
public:
    void Add(const Connection& conn) {
        connections.emplace_back(conn);
    }

    size_t CountChildren(CONNECTION_TYPE type, int64_t parent) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.parent_uid == parent && conn.type == type)
                ++c;
        }
        return c;
    }

    int64_t GetChild(CONNECTION_TYPE type, int64_t parent, size_t index) {
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

    size_t CountParents(CONNECTION_TYPE type, int64_t child) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.child_uid == child && conn.type == type)
                ++c;
        }
        return c;
    }

    int64_t GetParent(CONNECTION_TYPE type, int64_t child, size_t index) {
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

    Connection* GetChildConnection(CONNECTION_TYPE type, int64_t parent, size_t index) {
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

    Connection* GetParentConnection(CONNECTION_TYPE type, int64_t child, size_t index) {
        size_t c = 0;
        for(auto& conn : connections){
            if(conn.child_uid == child && conn.type == type) {
                if(index == c)
                    return &conn;
                ++c;
            }
        }
        return 0;
    }

    int64_t FindObjectToObjectParent(int64_t uid) {
        for(auto& conn : connections){
            if(conn.child_uid == uid && conn.type == OBJECT_OBJECT)
                return conn.parent_uid;
        }
        return -1;
    }
    int64_t FindObjectToObjectChild(int64_t uid) {
        for(auto& conn : connections){
            if(conn.parent_uid == uid && conn.type == OBJECT_OBJECT)
                return conn.child_uid;
        }
        return -1;
    }
private:
    std::vector<Connection> connections;
};

}

#endif
