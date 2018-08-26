#include "fbx_read.h"
#include <aurora/deflate.h>
#include <fstream>

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
    const FbxWord len_len = word_len ? 4 : 1;
    if(FbxOffset(cursor, end) < len_len)
        return false;
    
    const FbxWord length = word_len ? FbxRead<FbxWord>(data, cursor, end) : FbxRead<FbxByte>(data, cursor, end);
    
    if(FbxOffset(cursor, end) < length)
        return false;
    
    std::string str(cursor, cursor + length);
    out = str;
    cursor += length;
    
    return true;
}

void FbxReadData(FbxProp& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end);
bool FbxReadBlock(FbxNode& node, const char* data, const char*& cursor, const char* end, FbxWord flags);

bool FbxReadMem(FbxScene& out_scene, const char* data, size_t size)
{
    if(!data) return false;
    if(size < 0x1b) return false;
    if(strncmp(data, "Kaydara FBX Binary", 18))
    {
        std::cout << "Invalid FBX header" << std::endl;
        return false;
    }

    const char* cursor = data + 0x15;
    const FbxWord flags = FbxRead<FbxWord>(data, cursor, data + size);
    const FbxByte padding_0 = FbxRead<FbxByte>(data, cursor, data + size);
    const FbxByte padding_1 = FbxRead<FbxByte>(data, cursor, data + size);

    while(cursor < data + size)
    {
        if(!FbxReadBlock(out_scene._getRootNode(), data, cursor, data + size, flags)) 
            break;
    }

    out_scene._finalize();

    return true;
}

bool FbxReadFile(FbxScene& out_scene, const std::string& filename)
{
    std::ifstream f(filename, std::ios::binary | std::ios::ate);
    if(!f.is_open())
    {
        std::cout << "Failed to open " << filename << std::endl;
        return false;
    }
    std::streamsize size = f.tellg();
    f.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(!f.read(buffer.data(), (unsigned int)size))
    {
        f.close();
        std::cout << "Failed to read " << filename << std::endl;
        return false;
    }

    FbxReadMem(out_scene, buffer.data(), buffer.size());

    f.close();
    return true;
}

void FbxReadData(FbxProp& prop, std::vector<char>& out, const char* data, const char*& cursor, const char* end)
{
    bool is_encoded = false;
    FbxWord uncomp_len;    
    if(FbxOffset(cursor, end) < 1){
        //std::cout << "out of bounds while reading data length" << std::endl;
        return;
    }
    FbxWord stride = 0; // For arrays
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
        const FbxWord length = FbxRead<FbxWord>(data, cursor, end);
        cursor += length;
        break;
    }
    case 'b': cursor = end; break;
    case 'f':
    case 'i':
    case 'd':
    case 'l': {
        const FbxWord length = FbxRead<FbxWord>(data, cursor, end);
        const FbxWord encoding = FbxRead<FbxWord>(data, cursor, end);
        const FbxWord comp_len = FbxRead<FbxWord>(data, cursor, end);
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

bool FbxReadBlock(FbxNode& node, const char* data, const char*& cursor, const char* end, FbxWord flags)
{
    const FbxWord end_offset = FbxRead<FbxWord>(data, cursor, end);
    
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
    const FbxWord prop_count = FbxRead<FbxWord>(data, cursor, end);
    const FbxWord prop_len = FbxRead<FbxWord>(data, cursor, end);
    
    std::string block_name;
    FbxReadString(block_name, data, cursor, end);
    //std::cout << "BLOCK: [" << block_name << "]" << std::endl;
    node.SetName(block_name);
    //node.SetPropCount(prop_count);
    
    const char* begin_cur = cursor;
    for(unsigned i = 0; i < prop_count; ++i)
    {
        FbxProp prop;
        std::vector<char> actual_data;
        FbxReadData(prop, actual_data, data, cursor, begin_cur + prop_len);
        node.AddProp(prop);
    }
    
    if(FbxOffset(begin_cur, cursor) != prop_len)
    {
        //std::cout << "Property length was not reached" << std::endl;
        return false;
    }
    
    const size_t sentinel_block_length = sizeof(FbxWord) * 3 + 1;
    
    if(FbxOffset(data, cursor) < end_offset)
    {
        if(end_offset - FbxOffset(data, cursor) < sentinel_block_length)
        {
            //std::cout << "Insufficient padding bytes at block end" << std::endl;
            return false;
        }
        
        while(FbxOffset(data, cursor) < end_offset - sentinel_block_length)
        {
            FbxNode new_node;
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