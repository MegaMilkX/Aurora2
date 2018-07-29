#include "fbx.h"

#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "../util/deflate.h"

namespace Au{
namespace Media{
namespace FBX{
    
uint32_t Offset(const char* begin, const char* cursor)
{
    return static_cast<unsigned int>(cursor - begin);
}

template<typename TYPE>
TYPE Read(const char* data, const char*& cursor, const char* end)
{
    if(Offset(cursor, end) < sizeof(TYPE))
        return 0;
    
    TYPE result = *reinterpret_cast<const TYPE*>(cursor);
    
    cursor += sizeof(TYPE);
    
    return result;
}

bool ReadString(std::string& out, const char* data, const char*& cursor, const char* end,
    bool word_len = false)
{
    const Word len_len = word_len ? 4 : 1;
    if(Offset(cursor, end) < len_len)
        return false;
    
    const Word length = word_len ? Read<Word>(data, cursor, end) : Read<Byte>(data, cursor, end);
    
    if(Offset(cursor, end) < length)
        return false;
    
    std::string str(cursor, cursor + length);
    out = str;
    cursor += length;
    
    return true;
}

// MEMBER FUNCTIONS ====================

void Reader::ConvertCoordSys(CoordSystem sys)
{
    settings.convAxes.right = AXIS_X;
    settings.convAxes.up = AXIS_Y;
    settings.convAxes.front = AXIS_Z;
    
    coordSys = sys;
}

Axis Reader::GetUpAxis()
{
    Node* upAxis = rootNode.Get("Properties70", 0).GetWhere(0, "UpAxis");
    int axis = (int)(*upAxis)[4].GetInt32();
    Axis up = FBXAxisToAxis(axis);
    Node* upAxisSign = rootNode.Get("Properties70", 0).GetWhere(0, "UpAxisSign");
    int sign = (int)(*upAxisSign)[4].GetInt32();
    if(sign != 1)
        FlipAxis(up);
    return up;
}

Axis Reader::GetFrontAxis()
{
    Node* frontAxis = rootNode.Get("Properties70", 0).GetWhere(0, "FrontAxis");
    int axis = (int)(*frontAxis)[4].GetInt32();
    Axis front = FBXAxisToAxis(axis);
    Node* frontAxisSign = rootNode.Get("Properties70", 0).GetWhere(0, "FrontAxisSign");
    int sign = -(int)(*frontAxisSign)[4].GetInt32();
    if(sign != 1)
        FlipAxis(front);
    return front;
}

Axis Reader::GetRightAxis()
{
    Node* coordAxis = rootNode.Get("Properties70", 0).GetWhere(0, "CoordAxis");
    int axis = (int)(*coordAxis)[4].GetInt32();
    Axis right = FBXAxisToAxis(axis);
    Node* coordAxisSign = rootNode.Get("Properties70", 0).GetWhere(0, "CoordAxisSign");
    int sign = (int)(*coordAxisSign)[4].GetInt32();
    if(sign != 1)
        FlipAxis(right);
    return right;
}

TIME_MODE Reader::GetTimeMode()
{
    Node* node = rootNode.Get("Properties70", 0).GetWhere(0, "TimeMode");
    int timeMode = (*node)[4].GetInt32();
    return (TIME_MODE)timeMode;
}

int Reader::GetTimeProtocol()
{
    Node* node = rootNode.Get("Properties70", 0).GetWhere(0, "TimeProtocol");
    int timeMode = (*node)[4].GetInt32();
    return timeMode;
}

double Reader::GetCustomFrameRate()
{
    Node* node = rootNode.Get("Properties70", 0).GetWhere(0, "CustomFrameRate");
    double fps = (*node)[4].GetDouble();
    return fps;
}

double Reader::GetFrameRate()
{
    TIME_MODE mode = GetTimeMode();
    double fps = 30.0;
    switch(mode)
    {
    case FRAMES_DEFAULT:
        break;
    case FRAMES_120:
        fps = 120.0;
        break;
    case FRAMES_100:
        fps = 100.0;
        break;
    case FRAMES_60:
        fps = 60.0;
        break;
    case FRAMES_50:
        fps = 50.0;
        break;
    case FRAMES_48:
        fps = 48.0;
        break;
    case FRAMES_30:
        fps = 30.0;
        break;
    case FRAMES_30_DROP:
        fps = 30.0;
        break;
    case FRAMES_NTSC_DROP:
        fps = 29.97;
        break;
    case FRAMES_NTSC_FULL:
        fps = 29.97;
        break;
    case FRAMES_PAL:
        fps = 25.0;
        break;
    case FRAMES_CINEMA:
        fps = 24.0;
        break;
    case FRAMES_1000m: // Should not be used for frame rate
        fps = 30.0;
        break;
    case FRAMES_CINEMA_ND:
        fps = 23.976;
        break;
    case FRAMES_CUSTOM:
        fps = GetCustomFrameRate();
        break;
    case FRAMES_96:
        fps = 96.0;
        break;
    case FRAMES_72:
        fps = 72.0;
        break;
    case FRAMES_59dot94:
        fps = 59.94;
        break;
    }
    
    return fps;
}

std::vector<AnimationStack>& Reader::GetAnimationStacks()
{
    if(animStacks.empty())
    {
        int animStackCount = rootNode.Count("AnimationStack");
        for(int i = 0; i < animStackCount; ++i)
        {
            AnimationStack animStack(rootNode, rootNode.Get("AnimationStack", i), settings);
            animStacks.push_back(animStack);
        }
    }
    return animStacks;
}

void Reader::FlipAxis(Axis& axis)
{
    if(axis == AXIS_X)
        axis = AXIS_MX;
    else if(axis == AXIS_Y)
        axis = AXIS_MY;
    else if(axis == AXIS_Z)
        axis = AXIS_MZ;
    else if(axis == AXIS_MX)
        axis = AXIS_X;
    else if(axis == AXIS_MY)
        axis = AXIS_Y;
    else if(axis == AXIS_MZ)
        axis = AXIS_Z;
}

Mesh* Reader::GetBoneDeformTarget(Node* bone)
{
    int64_t uid = (*bone)[0].GetInt64();
    Node* conn = 0;
    
    Node* deformer = 
        rootNode.GetConnectedParent("Deformer", uid, &conn);
    if(!deformer)
        return 0;
    
    Node* skinDeformer = 
        rootNode.GetConnectedParent("Deformer", (*deformer)[0].GetInt64(), &conn);
    if(!skinDeformer)
        return 0;
    
    Node* geometryNode = 
        rootNode.GetConnectedParent("Geometry", (*skinDeformer)[0].GetInt64(), &conn);
    if(!geometryNode)
        return 0;
    
    Node* modelNode =
        rootNode.GetConnectedParent("Model", (*geometryNode)[0].GetInt64(), &conn);
    if(!modelNode)
        return 0;
    
    std::string meshName = (*modelNode)[1].GetString();
    
    return GetMesh(meshName);
}

void Reader::_loadModels()
{
    unsigned modelCount = rootNode.Count("Model");
    for(unsigned i = 0; i < modelCount; ++i)
    {
        Node& modelNode = rootNode.Get("Model", i);
        
        Model model(&settings, &rootNode, &modelNode);
        models.push_back(model);
    }
}

unsigned Reader::ModelCount()
{
    return models.size();
}

Model* Reader::GetModel(unsigned id)
{
    if(id >= models.size())
        return 0;
    return &models[id];
}

Model* Reader::GetModelByUID(int64_t uid)
{
    for(unsigned i = 0; i < models.size(); ++i)
    {
        if(models[i].uid == uid)
            return &models[i];
    }
    return 0;
}

void Reader::_loadBones()
{    
    std::vector<Node> models = rootNode.GetAll("Model");
    
    for(unsigned i = 0; i < models.size(); ++i)
    {
        Node& model = models[i];
        std::string type = model[2].GetString();
        if(type != "LimbNode")
            continue;
        
        Mesh* meshTgt = GetBoneDeformTarget(&model);
        if(!meshTgt)
            continue;
        
        Bone bone(settings, rootNode, model, meshTgt);
        bones.push_back(bone);
    }
    
    std::sort(bones.begin(), bones.end());
    
    for(unsigned i = 0; i < bones.size(); ++i)
    {
        bones[i].Index(i);
    }
}

std::vector<Bone>& Reader::GetBones() 
{ 
    return bones; 
}

void Reader::ReadData(Prop& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end)
{
    bool is_encoded = false;
    Word uncomp_len;
    
    if(Offset(cursor, end) < 1)
    {
        //std::cout << "out of bounds while reading data length" << std::endl;
        return;
    }
    
    Word stride = 0; // For arrays
    const char type = *cursor;
    const char* sbegin = ++cursor;
    
    prop.Type(type);
    
    switch(type)
    {
    // 16 bit int
    case 'Y':
        cursor += 2;
        break;
    // 1 bit bool
    case 'C':
        cursor += 1;
        break;
    // 32 bit int
    case 'I':
        cursor += 4;
        break;
    case 'F':
        cursor += 4;
        break;
    // double
    case 'D':
        cursor += 8;
        break;
    // 64 bit int
    case 'L':
        cursor += 8;
        break;
    // Binary data
    case 'R':
    {
        const Word length = Read<Word>(data, cursor, end);
        cursor += length;
        break;
    }
    case 'b':
        cursor = end;
        break;
    case 'f':
    case 'i':
    case 'd':
    case 'l':
    {
        const Word length = Read<Word>(data, cursor, end);
        const Word encoding = Read<Word>(data, cursor, end);
        const Word comp_len = Read<Word>(data, cursor, end);
        //std::cout << "LEN: " << length << "|" << "ENC: " << encoding << "|" << "COMP_LEN: " << comp_len << std::endl;
        sbegin = cursor;
        
        switch(type)
        {
        case 'f':
            //std::cout << "float array, size: " << length << std::endl;
            stride = 4;
            break;
        case 'i':
            //std::cout << "int array, size: " << length << std::endl;
            stride = 4;
            break;
        case 'd':
            //std::cout << "double array, size: " << length << std::endl;
            stride = 8;
            break;
        case 'l':
            //std::cout << "long array, size: " << length << std::endl;
            stride = 8;
            break;
        }
        
        if(encoding == 0)
        {
            if(length * stride != comp_len)
            {
                //std::cout << "Failed to ReadData, calculated data stride differs from what the file claims" << std::endl;
                return;
            }
        }
        else if(encoding == 1)
        {
            //std::cout << "This shit is encoded" << std::endl;
            is_encoded = true;
        }
        else if(encoding != 1)
        {
            //std::cout << "ReadData failed, unknown encoding" << std::endl;
            return;
        }
        cursor += comp_len;
        uncomp_len = length * stride;
        //std::cout << "Uncomp len: " << uncomp_len << std::endl;
        break;
    }
    case 'S':
    {
        std::string str;
        ReadString(str, data, cursor, end, true);
        sbegin = cursor - str.size();
        //std::cout << "data str: " << str << std::endl;
        break;
    }
    default:
        //std::cout << "ReadData failed, unexpected type code: " << std::string(&type, 1) << std::endl;
        return;
    }
    
    if(cursor > end)
    {
        //std::cout << "ReadData failed, remaining size too small" << std::endl;
        return;
    }
    
    const char* send = cursor;
    out = std::vector<char>(sbegin, send);
    
    if(is_encoded)
    {
        out = Au::Inflate(out, uncomp_len);
    }
    prop.Data(out);
    //std::cout << "Data read: " << out.size() << std::endl;
}

bool Reader::ReadBlock(Node& node, const char* data, const char*& cursor, const char* end, Word flags)
{
    const Word end_offset = Read<Word>(data, cursor, end);
    
    if(end_offset == 0)
    {
        //std::cout << "end_offset is 0" << std::endl;
        return false;
    }
    if(end_offset > Offset(data, end))
    {
        //std::cout << "end_offset > Offset(data, end)" << std::endl;
        return false;
    }
    else if(end_offset < Offset(data, cursor))
    {
        //std::cout << "end_offset < Offset(data, end)" << std::endl;
        return false;
    }
    const Word prop_count = Read<Word>(data, cursor, end);
    const Word prop_len = Read<Word>(data, cursor, end);
    
    std::string block_name;
    ReadString(block_name, data, cursor, end);
    //std::cout << "BLOCK: [" << block_name << "]" << std::endl;
    node.Name(block_name);
    node.PropCount(prop_count);
    
    const char* begin_cur = cursor;
    for(unsigned i = 0; i < prop_count; ++i)
    {
        Prop prop;
        std::vector<char> actual_data;
        ReadData(prop, actual_data, data, cursor, begin_cur + prop_len);
        node.Add(prop);
    }
    
    if(Offset(begin_cur, cursor) != prop_len)
    {
        //std::cout << "Property length was not reached" << std::endl;
        return false;
    }
    
    const size_t sentinel_block_length = sizeof(Word) * 3 + 1;
    
    if(Offset(data, cursor) < end_offset)
    {
        if(end_offset - Offset(data, cursor) < sentinel_block_length)
        {
            //std::cout << "Insufficient padding bytes at block end" << std::endl;
            return false;
        }
        
        while(Offset(data, cursor) < end_offset - sentinel_block_length)
        {
            Node new_node;
            ReadBlock(new_node, data, cursor, data + end_offset - sentinel_block_length, flags);
            node.Add(new_node);
        }
        
        for(unsigned i = 0; i < sentinel_block_length; ++i)
        {
            if(cursor[i] != '\0')
            {
                //std::cout << "13 zero bytes expected" << std::endl;
                return 0;
            }
        }
        cursor += sentinel_block_length;
    }
    
    if(Offset(data, cursor) != end_offset)
    {
        //std::cout << "scope length not reached" << std::endl;
        return false;
    }
    
    return true;
}

std::vector<Node> Reader::GetConnectedChildren(const std::string& childName, Node& node)
{
    std::vector<Node> result;
    std::vector<Node*> connections = 
        rootNode.GetNodesWithProp("C", 2, node[0].GetInt64());
    for(unsigned i = 0; i < connections.size(); ++i)
    {
        Node* n = rootNode.GetNodeWithUID(childName, (*connections[i])[1].GetInt64());
        if(n)
            result.push_back(*n);
    }
    return result;
}

Axis Reader::FBXAxisToAxis(unsigned axis)
{ 
    if(axis == 0)
        return AXIS_X;
    else if(axis == 1)
        return AXIS_Z;
    else if(axis == 2)
        return AXIS_Y;
    else
        return AXIS_UNKNOWN;
}

bool Reader::ReadMemory(const char* data, unsigned size)
{
    {
        //Au::Timer t;
        //t.Start();
        if(!ReadFileFBX(data, size))
            return false;
        //std::cout << "ReadFileFBX: " << t.End() / 1000000.0f << std::endl;
    }
    settings.origAxes.right = GetRightAxis();
    settings.origAxes.up = GetUpAxis();
    settings.origAxes.front = GetFrontAxis();
    settings.convAxes = settings.origAxes;
    settings.Init(rootNode);
    
    {
        //Au::Timer t;
        //t.Start();
        int meshCount = rootNode.Count("Geometry");
        for(int i = 0; i < meshCount; ++i)
        {
            Node& geometry = rootNode.Get("Geometry", i);
            
            Mesh mesh(settings, rootNode, geometry);
            meshes.push_back(mesh);
        }
        //std::cout << "Geometry: " << t.End() / 1000000.0f << std::endl;
    }
    
    {
        //Au::Timer t;
        //t.Start();
        _loadModels();
        //std::cout << "_loadModels: " << t.End() / 1000000.0f << std::endl;
    }
    {
        //Au::Timer t;
        //t.Start();
        _loadBones();
        //std::cout << "_loadBones: " << t.End() / 1000000.0f << std::endl;
    }
    
    return true;
}

bool Reader::ReadFileFBX(const char* data, unsigned size)
{
    if(!data)
    {
        //std::cout << "data is null" << std::endl;
        return false;
    }
    if(size < 0x1b)
    {
        //std::cout << "size is to small" << std::endl;
        return false;
    }
    if(strncmp(data, "Kaydara FBX Binary", 18))
    {
        std::cout << "Invalid FBX header" << std::endl;
        return false;
    }
    //std::cout << "Found FBX header" << std::endl;
    
    const char* cursor = data + 0x15;
    
    const Word flags = Read<Word>(data, cursor, data + size);
    const Byte padding_0 = Read<Byte>(data, cursor, data + size);
    const Byte padding_1 = Read<Byte>(data, cursor, data + size);
    
    while(cursor < data + size)
    {
        if(!ReadBlock(rootNode, data, cursor, data + size, flags))
            break;
    }
    
    return true;
}

}
}
}