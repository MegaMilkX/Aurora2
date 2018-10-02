#include "fbx_prop.h"

#include <sstream>
#include <iostream>

void FbxProp::SetType(char t) { type = t; }

char FbxProp::GetType() { return type; }

void FbxProp::SetArrayLen(unsigned len){
    arraySize = len;
}
void FbxProp::SetData(const std::vector<char>& data){
    this->data = data;
}
std::string FbxProp::GetString(){
    if(type != 'S') return std::string();
    unsigned strLen = 0;
    for(unsigned i = 0; i < data.size(); ++i)
    {
        if(data[i] == '\0')
            break;
        strLen++;
    }
    return std::string(data.data(), data.data() + strLen);
}
int64_t FbxProp::GetInt64(){
    if(data.size() != sizeof(int64_t))
        return 0;
    return *(int64_t*)(data.data());
}
int32_t FbxProp::GetInt32(){
    if(data.size() != sizeof(int32_t))
        return 0;
    return *(int32_t*)(data.data());
}
int16_t FbxProp::GetInt16(){
    if(data.size() != sizeof(int16_t))
        return 0;
    return *(int16_t*)(data.data());
}
float FbxProp::GetFloat(){
    if(data.size() != sizeof(float))
        return 0.0;
    return *(float*)(data.data());
}
double FbxProp::GetDouble(){
    if(data.size() != sizeof(double))
        return 0.0;
    return *(double*)(data.data());
}

void FbxProp::Print(std::ostringstream& sstr, unsigned level){
    for(unsigned i = 0; i < level; ++i)
        sstr << "  ";
    int stride = 0;
    switch(type)
    {
    // 16 bit int
    case 'Y':
        sstr << "int16: " << *(int16_t*)(data.data());
        break;
    // 1 bit bool
    case 'C':
        sstr << "1 bit bool";
        break;
    // 32 bit int
    case 'I':
        sstr << "int32: " << *(int32_t*)(data.data());
        break;
    case 'F':
        sstr << "float: " << *(float*)(data.data());
        break;
    // double
    case 'D':
        sstr << "double: " << *(double*)(data.data());
        break;
    // 64 bit int
    case 'L':
        sstr << "int64: " << *(int64_t*)(data.data());
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
        sstr << "string: '" << std::string(data.data(), data.data() + data.size()) << "'";
        break;
    }
    
    sstr << std::endl;
}