#ifndef FBX_NODE_2_H
#define FBX_NODE_2_H

#include "fbx_prop.h"
#include "fbx_type_index.h"
#include <map>
#include <memory>

namespace Fbx {

class BasicObject;
class Object;
class Node
{
public:
    ~Node();
    void AddNode(const Node& node);
    void AddProp(const NodeProperty& prop);
    void SetName(const std::string& name);
    const std::string& GetName() const;
    unsigned PropCount() const;
    NodeProperty& GetProperty(unsigned i);
    size_t ChildCount();
    Node& GetNode(unsigned i);
    Node* FindNode(const std::string& name);
    Node* FindNode(const std::string& name, unsigned i);
    template<typename T>
    T* Convert() {
        if(object_cache[TypeInfo<T>::Index()]) {
            return (T*)object_cache[TypeInfo<T>::Index()];
        }
        if(!T::IdentifyNode(*this)) {
            return 0;
        }
        T* ptr = new T();
        SetUidAndName(*this, ptr);
        if(!ptr->Make(*this)) {
            delete ptr;
            return 0;
        }
        object_cache[TypeInfo<T>::Index()] = ptr;
        return ptr;
    }
    template<typename T>
    T* ConvertChild() {
        return ConvertChild<T>(0);
    }
    template<typename T>
    size_t CountConvertable() {
        size_t counter = 0;
        for(size_t j = 0; j < ChildCount(); ++j) {
            Node& n = GetNode(j);
            if(T::IdentifyNode(n)) {
                counter++;
            }
        }
        return counter;
    }
    template<typename T>
    T* ConvertChild(unsigned i) {
        unsigned counter = 0;
        for(size_t j = 0; j < ChildCount(); ++j) {
            Node& n = GetNode(j);
            if(T::IdentifyNode(n)) {
                if(counter == i) {
                    return n.Convert<T>();
                }
                counter++;
            }
        }
        return 0;
    }

    void Print(std::ostringstream& sstr, unsigned level = 0);
private:
    std::string name;
    std::vector<NodeProperty> props;
    std::vector<Node> children;
    std::map<TypeIndex, BasicObject*> object_cache;

    void SetUidAndName(Node& node, Object* o);
    void SetUidAndName(Node& node, BasicObject* o);
};

}

#endif
