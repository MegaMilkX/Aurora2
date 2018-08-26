#include "fbx_node.h"

#include <sstream>

void FbxNode::AddNode(const FbxNode& node){
    children[node.GetName()].emplace_back(node);
}
void FbxNode::AddProp(const FbxProp& prop){
    props.emplace_back(prop);
}
void FbxNode::SetName(const std::string& name){
    this->name = name;
}
const std::string& FbxNode::GetName() const{
    return name;
}
unsigned FbxNode::PropCount() const{
    return props.size();
}
FbxProp& FbxNode::GetProperty(unsigned i){
    return props[i];
}
unsigned FbxNode::ChildCount(const std::string& type) {
    return children[type].size();
}
FbxNode& FbxNode::GetNode(const std::string& type, unsigned i){
    return children[type][i];
}

void FbxNode::Print(std::ostringstream& sstr, unsigned level){
    for(unsigned i = 0; i < level; ++i)
        sstr << "  ";
    sstr << name << " | " << "Prop count: " << props.size() << std::endl;
    ++level;
    for(unsigned i = 0; i < props.size(); ++i)
        props[i].Print(sstr, level);
    for(auto& kv : children)
    {
        for(auto& n : kv.second)
        {
            n.Print(sstr, level);
        }
    }
}