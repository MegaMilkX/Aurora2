#include "fbx_scene.h"
#include "fbx_macro.h"

#include <aurora/deflate.h>

namespace Fbx {

void Scene::Finalize(Node& node) {
    FBX_LOG("Finalizing fbx data structures...");

    Properties* props;
    if(props = node.ConvertChild<Properties>()) {
        properties = props;
    } else {
        FBX_LOGW("Scene properties not found");
    }

    for(size_t i = 0; i < node.ChildCount(); ++i) {
        Node& n = node.GetNode(i);
        if(n.GetName() != "C") continue;
        Connection* c = n.Convert<Connection>();
        connections.Add(*c);
    }

    for(size_t i = 0; i < node.ChildCount(); ++i) {
        Node& n = node.GetNode(i);
        
        objects.Create<AnimationCurve>(n)
        || objects.Create<AnimationCurveNode>(n)
        || objects.Create<DeformerCluster>(n)
        || objects.Create<DeformerBlendShape>(n)
        || objects.Create<NodeAttribute>(n)
        || objects.Create<Model>(n)
        || objects.Create<DeformerSkin>(n)
        || objects.Create<Geometry>(n)
        || objects.Create<AnimationStack>(n)
        || objects.Create<AnimationLayer>(n)
        || objects.Create<Texture>(n)
        || objects.Create<Material>(n)
        || objects.Create<Pose>(n);
    }

    for(size_t i = 0; i < objects.Count<Model>(); ++i) {
        if(!GetParent<Model>(OBJECT_OBJECT, objects.Get<Model>(i)->GetUid())) {
            rootModels.emplace_back(objects.Get<Model>(i));
        }
    }
    FBX_LOG("Root model count: " << rootModels.size());

    FBX_LOG("Done");
}

// ===============================
//
// ===============================

uint32_t FbxOffset(const char* begin, const char* cursor)
{
    return static_cast<unsigned int>(cursor - begin);
}

template<typename TYPE>
TYPE FbxRead(const char* data, const char*& cursor, const char* end)
{
    if(FbxOffset(cursor, end) < sizeof(TYPE))
        return 0;
    
    TYPE result = *reinterpret_cast<const TYPE*>(cursor);
    
    cursor += sizeof(TYPE);
    
    return result;
}

bool FbxReadString(std::string& out, const char* data, const char*& cursor, const char* end,
    bool word_len = false)
{
    const Word len_len = word_len ? 4 : 1;
    if(FbxOffset(cursor, end) < len_len)
        return false;
    
    const Word length = word_len ? FbxRead<Word>(data, cursor, end) : FbxRead<Byte>(data, cursor, end);
    
    if(FbxOffset(cursor, end) < length)
        return false;
    
    std::string str(cursor, cursor + length);
    out = str;
    cursor += length;
    
    return true;
}

void FbxReadData(NodeProperty& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end);
bool FbxReadBlock(Node& node, const char* data, const char*& cursor, const char* end, Word flags);

bool Scene::ReadMem(const char* data, size_t size)
{
    FBX_LOG("Reading fbx data...")
    if(!data) return false;
    if(size < 0x1b) return false;
    if(strncmp(data, "Kaydara FBX Binary", 18))
    {
        FBX_LOG("Invalid FBX header");
        return false;
    }

    const char* cursor = data + 0x15;
    const Word flags = FbxRead<Word>(data, cursor, data + size);
    const Byte padding_0 = FbxRead<Byte>(data, cursor, data + size);
    const Byte padding_1 = FbxRead<Byte>(data, cursor, data + size);

    while(cursor < data + size)
    {
        if(!FbxReadBlock(rootNode, data, cursor, data + size, flags)) 
            break;
    }

    Finalize(rootNode);
    FBX_LOG("Done")
    return true;
}

bool Scene::ReadFile(const std::string& filename)
{
    FBX_LOG("Reading fbx file '" << filename << "'...");
    std::ifstream f(filename, std::ios::binary | std::ios::ate);
    if(!f.is_open())
    {
        FBX_LOG("Failed to open " << filename);
        return false;
    }
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(!f.read(buffer.data(), (unsigned int)size))
    {
        f.close();
        FBX_LOG("Failed to read " << filename);
        return false;
    }

    ReadMem(buffer.data(), buffer.size());

    f.close();
    FBX_LOG("Done");
    return true;
}

void FbxReadData(NodeProperty& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end)
{
    bool is_encoded = false;
    Word uncomp_len;    
    if(FbxOffset(cursor, end) < 1){
        //std::cout << "out of bounds while reading data length" << std::endl;
        return;
    }
    Word stride = 0; // For arrays
    const char type = *cursor;
    const char* sbegin = ++cursor;
    
    prop.SetType(type);
    
    switch(type)
    {
    // 16 bit int
    case 'Y': cursor += 2; break;
    // 1 bit bool
    case 'C': cursor += 1; break;
    // 32 bit int
    case 'I': cursor += 4; break;
    case 'F': cursor += 4; break;
    // double
    case 'D': cursor += 8; break;
    // 64 bit int
    case 'L': cursor += 8; break;
    // Binary data
    case 'R': {
        const Word length = FbxRead<Word>(data, cursor, end);
        cursor += length;
        break;
    }
    case 'b': cursor = end; break;
    case 'f':
    case 'i':
    case 'd':
    case 'l': {
        const Word length = FbxRead<Word>(data, cursor, end);
        const Word encoding = FbxRead<Word>(data, cursor, end);
        const Word comp_len = FbxRead<Word>(data, cursor, end);
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
        FbxReadString(str, data, cursor, end, true);
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
    prop.SetData(out);
    //std::cout << "Data read: " << out.size() << std::endl;
}

bool FbxReadBlock(Node& node, const char* data, const char*& cursor, const char* end, Word flags)
{
    const Word end_offset = FbxRead<Word>(data, cursor, end);
    
    if(end_offset == 0)
    {
        //std::cout << "end_offset is 0" << std::endl;
        return false;
    }
    if(end_offset > FbxOffset(data, end))
    {
        //std::cout << "end_offset > Offset(data, end)" << std::endl;
        return false;
    }
    else if(end_offset < FbxOffset(data, cursor))
    {
        //std::cout << "end_offset < Offset(data, end)" << std::endl;
        return false;
    }
    const Word prop_count = FbxRead<Word>(data, cursor, end);
    const Word prop_len = FbxRead<Word>(data, cursor, end);
    
    std::string block_name;
    FbxReadString(block_name, data, cursor, end);
    //std::cout << "BLOCK: [" << block_name << "]" << std::endl;
    node.SetName(block_name);
    //node.SetPropCount(prop_count);
    
    const char* begin_cur = cursor;
    for(unsigned i = 0; i < prop_count; ++i)
    {
        NodeProperty prop;
        std::vector<char> actual_data;
        FbxReadData(prop, actual_data, data, cursor, begin_cur + prop_len);
        node.AddProp(prop);
    }
    
    if(FbxOffset(begin_cur, cursor) != prop_len)
    {
        //std::cout << "NodeProperty length was not reached" << std::endl;
        return false;
    }
    
    const size_t sentinel_block_length = sizeof(Word) * 3 + 1;
    
    if(FbxOffset(data, cursor) < end_offset)
    {
        if(end_offset - FbxOffset(data, cursor) < sentinel_block_length)
        {
            //std::cout << "Insufficient padding bytes at block end" << std::endl;
            return false;
        }
        
        while(FbxOffset(data, cursor) < end_offset - sentinel_block_length)
        {
            Node new_node;
            FbxReadBlock(new_node, data, cursor, data + end_offset - sentinel_block_length, flags);
            node.AddNode(new_node);
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
    
    if(FbxOffset(data, cursor) != end_offset)
    {
        //std::cout << "scope length not reached" << std::endl;
        return false;
    }
    
    return true;
}

} // Fbx
