#ifndef AU_FBXNODE_H
#define AU_FBXNODE_H

#include <stdint.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../math/math.h"

namespace Au{
namespace Media{
namespace FBX{
    
class Prop
{
public:
    void Print(std::ostringstream& sstr, unsigned level = 0)
    {
        for(unsigned i = 0; i < level; ++i)
            sstr << "  ";
        int stride = 0;
        switch(type)
        {
        // 16 bit int
        case 'Y':
            sstr << *(int16_t*)(data.data());
            break;
        // 1 bit bool
        case 'C':
            sstr << "1 bit bool";
            break;
        // 32 bit int
        case 'I':
            sstr << *(int32_t*)(data.data());
            break;
        case 'F':
            sstr << *(float*)(data.data());
            break;
        // double
        case 'D':
            sstr << *(double*)(data.data());
            break;
        // 64 bit int
        case 'L':
            sstr << *(int64_t*)(data.data());
            break;
        // Binary data
        case 'R':
            sstr << "Just binary data";
            break;
        case 'b':
            break;
        case 'f':
            stride = 4;
            sstr << "Float array, size: " << data.size() / stride;
            break;
        case 'i':
            stride = 4;
            sstr << "Int32 array, size: " << data.size() / stride;
            break;
        case 'd':
            stride = 8;
            sstr << "Double array, size: " << data.size() / stride;
            break;
        case 'l':
            stride = 8;
            sstr << "Int64 array, size: " << data.size() / stride;
            break;
        case 'S':
            sstr << std::string(data.data(), data.data() + data.size());
            break;
        }
        
        sstr << std::endl;
    }
    void Type(char type)
    {
        this->type = type;
    }
    void ArraySize(unsigned arraySize)
    {
        this->arraySize = arraySize;
    }
    void Data(const std::vector<char>& data)
    {
        this->data = data;
    }
    
    std::string GetString()
    {
        unsigned strLen = 0;
        for(unsigned i = 0; i < data.size(); ++i)
        {
            if(data[i] == '\0')
                break;
            strLen++;
        }
        return std::string(data.data(), data.data() + strLen);
    }
    
    int64_t GetInt64()
    {
        if(data.size() != sizeof(int64_t))
            return 0;
        return *(int64_t*)(data.data());
    }
    
    int32_t GetInt32()
    {
        if(data.size() != sizeof(int32_t))
            return 0;
        return *(int32_t*)(data.data());
    }
    
    float GetFloat()
    {
        if(data.size() != sizeof(float))
            return 0.0;
        return *(float*)(data.data());
    }
    
    double GetDouble()
    {
        if(data.size() != sizeof(double))
            return 0.0;
        return *(double*)(data.data());
    }
    
    template<typename T>
    std::vector<T> GetArray()
    {
        std::vector<T> result;
        
        unsigned byteLength = data.size();
        unsigned arrayLength = 0;
        unsigned targetElementSize = sizeof(T);
        
        switch(type)
        {
        case 'f': case 'i': arrayLength = byteLength / 4; break;
        case 'd': case 'l': arrayLength = byteLength / 8; break;
        }
        
        if(arrayLength == 0)
            return result;
        
        switch(type)
        {
        case 'f':
            {
                float* dataptr = (float*)(data.data());
                for(unsigned i = 0; i < arrayLength; ++i)
                    result.push_back((T)(dataptr[i]));
            }
            break;
        case 'i':
            {
                int32_t* dataptr = (int32_t*)(data.data());
                for(unsigned i = 0; i < arrayLength; ++i)
                    result.push_back((T)(dataptr[i]));
            }
            break;
        case 'd':
            {
                double* dataptr = (double*)(data.data());
                for(unsigned i = 0; i < arrayLength; ++i)
                    result.push_back((T)(dataptr[i]));
            }
            break;
        case 'l':
            {
                int64_t* dataptr = (int64_t*)(data.data());
                for(unsigned i = 0; i < arrayLength; ++i)
                    result.push_back((T)(dataptr[i]));
            }
            break;
        }
        
        return result;
    }
private:
    char type;
    unsigned arraySize;
    std::vector<char> data;
};

class Node
{
public:    
    void Add(const Node& node)
    {
        children.push_back(node);
    }
    
    void Add(const Prop& prop)
    {
        props.push_back(prop);
    }
    
    void Name(const std::string& name)
    {
        this->name = name;
    }
    
    std::string Name()
    {
        return name;
    }
    
    void PropCount(unsigned count)
    {
        propCount = count;
    }
    
    unsigned ChildCount()
    {
        return children.size();
    }
    
    Node& Get(unsigned index)
    {
        if(index >= children.size())
            children.resize(index + 1);
        
        return children[index];
    }
    
    Node& Get(const std::string& name, unsigned instance = 0)
    {
        unsigned instanceCounter = 0;
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].name == name && instance == instanceCounter++)
            {
                return children[i];
            }
        }
        
        Node node;
        node.Name(name);
        children.push_back(node);
        return children[children.size()-1];
    }
    
    std::vector<Node*> GetConnectedChildren(
        const std::string& childName, 
        int64_t parentUID,
        std::vector<Node*>& conns = std::vector<Node*>())
    {
        std::vector<Node*> result;
        std::vector<Node*> connections = 
            GetNodesWithProp("C", 2, parentUID);
        for(unsigned i = 0; i < connections.size(); ++i)
        {
            Node* n = GetNodeWithUID(childName, (*connections[i])[1].GetInt64());
            if(n)
            {
                conns.push_back(connections[i]);
                result.push_back(n);
            }
        }
        return result;
    }
    
    Node* GetConnectedParent(const std::string& parentName, int64_t childUID, Node** conn)
    {
        std::vector<Node*> connections = 
            GetNodesWithProp("C", 1, childUID);
        for(unsigned i = 0; i < connections.size(); ++i)
        {
            Node* n = GetNodeWithUID(parentName, (*connections[i])[2].GetInt64());
            if(n)
            {
                if(conn)
                    (*conn) = connections[i];
                return n;
            }
        }
        return 0;
    }
    
    std::vector<Node*> GetNodesWithProp(const std::string& name, unsigned propId, const std::string& value)
    {
        std::vector<Node*> result;
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].name == name)
                if(children[i][propId].GetString() == value)
                    result.push_back(&children[i]);
        }
        return result;
    }
    
    std::vector<Node*> GetNodesWithProp(const std::string& name, unsigned propId, int64_t value)
    {
        std::vector<Node*> result;
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].name == name)
                if(children[i][propId].GetInt64() == value)
                    result.push_back(&children[i]);
        }
        return result;
    }
    
    Node* GetNodeWithUID(const std::string& name, int64_t uid)
    {
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].name == name)
                if(children[i][0].GetInt64() == uid)
                    return &children[i];
        }
        return 0;
    }
    
    Node* GetWhere(const std::string& name, unsigned propId, const std::string& value)
    {
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].Name() == name)
                if(children[i][propId].GetString() == value)
                    return &children[i];
        }
        return 0;
    }
    
    Node* GetWhere(const std::string& name, unsigned propId, int64_t value)
    {
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].Name() == name)
                if(children[i][propId].GetInt64() == value)
                    return &children[i];
        }
        return 0;
    }
    
    Node* GetWhere(unsigned propId, const std::string& value)
    {
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i][propId].GetString() == value)
            {
                return &children[i];
            }
        }
        return 0;
    }
    
    std::vector<Node> GetAll(const std::string& name)
    {
        std::vector<Node> result;
        for(unsigned i = 0; i < children.size(); ++i)
        {
            if(children[i].name == name)
                result.push_back(children[i]);
        }
        return result;
    }
    
    int Count(const std::string& name)
    {
        int result = 0;
        for(unsigned i = 0; i < children.size(); ++i)
            if(children[i].name == name)
                result++;
        return result;
    }
    
    Prop& operator[](unsigned index)
    {
        if(index >= props.size())
            props.resize(index+1);
        return props[index];
    }
    
    Prop& GetProp(unsigned index)
    {
        if(index >= props.size())
            props.resize(index+1);
        return props[index];
    }
    
    void Print(std::ostringstream& sstr, unsigned level = 0)
    {
        for(unsigned i = 0; i < level; ++i)
            sstr << "  ";
        sstr << name << " | " << "Prop count: " << propCount << std::endl;
        ++level;
        for(unsigned i = 0; i < props.size(); ++i)
            props[i].Print(sstr, level);
        for(unsigned i = 0; i < children.size(); ++i)
            children[i].Print(sstr, level);
    }
    
private:
    std::string name;
    unsigned propCount;
    std::vector<Prop> props;
    std::vector<Node> children;
};
    
}}}

#endif
