#ifndef BINARY_CODEC_H
#define BINARY_CODEC_H

#include <fstream>
#include <rttr/type>
#include "binary_data.h"

namespace binary
{

enum BINARY_BASIC_TYPES
{
    CHAR,
    INT16,
    INT32,
    INT64,
    UCHAR,
    UINT16,
    UINT32,
    UINT64,
    FLOAT,
    DOUBLE,
    BOOL,
    TYPE_REGISTRY,
    CHUNK_REGISTRY,
    END = 0xFFFFFFFF
};

#pragma pack(push, 1)
struct header
{
    char id_string[11] = { '\211', 'A', 'u', 'r', 'o', 'r', 'a', '\r', '\n', '\032', '\n' };
    char versionMajor = 1;
    char versionMinor = 0;
    uint64_t offsetToData = sizeof(header);
};
#pragma pack(pop)

#pragma pack(push, 1)
struct chunk_header
{
    uint64_t size; // w/o the chunk header size
    uint32_t type;
};
#pragma pack(pop)

#define ADD_SIMPLE_ENCODER(TYPE, TYPE_NUM) add_encoder<TYPE>([](binary_data& dest, void* data, size_t count){ \
    dest.write(chunk_header{ sizeof(TYPE) * count, (uint32_t) TYPE_NUM }); \
    dest.write((TYPE*)data, count); \
});

class binary_codec
{
public:
    typedef std::function<void(binary_data&, void*, size_t)> encoder_fn_t;
    typedef std::function<void(binary_data&, void*, size_t)> decoder_fn_t;

    binary_codec()
    {
        ADD_SIMPLE_ENCODER(char, CHAR);
        ADD_SIMPLE_ENCODER(int16_t, INT16);
        ADD_SIMPLE_ENCODER(int32_t, INT32);
        ADD_SIMPLE_ENCODER(int64_t, INT64);
        ADD_SIMPLE_ENCODER(unsigned char, UCHAR);
        ADD_SIMPLE_ENCODER(uint16_t, UINT16);
        ADD_SIMPLE_ENCODER(uint32_t, UINT32);
        ADD_SIMPLE_ENCODER(uint64_t, UINT64);
        ADD_SIMPLE_ENCODER(float, FLOAT);
        ADD_SIMPLE_ENCODER(double, DOUBLE);
        ADD_SIMPLE_ENCODER(bool, BOOL);
    }

    template<typename T>
    void add_encoder(encoder_fn_t fn)
    {
        encoders[rttr::type::get<T>()] = fn;
    }

    template<typename T>
    bool encode(binary_data& dest, const T& value)
    {
        return encode(dest, &value, 1);
    }

    template<typename T>
    bool encode(binary_data& dest, T* data, size_t count)
    {
        auto it = encoders.find(rttr::type::get<T>());
        if(it == encoders.end())
            return false;
        encoder_fn_t encoder = it->second;
        encoder(dest, (void*)data, count);
        return true;
    }

    template<typename T>
    void add_chunk(const T& data, const std::string& name = "")
    {
        chunk_header header;
        header.type = rttr::type::get<T>().get_id();
        header.size = sizeof(T);
        binary_data bdata;
        bdata.write(data);
        chunks.push_back(std::make_pair(header, bdata));

        _type_registry[rttr::type::get<T>().get_name().to_string()] = rttr::type::get<T>().get_id();

        if(name.empty()) return;

        _chunk_registry[name] = 0;
    }

    template<typename T>
    void write_map(BINARY_BASIC_TYPES type, const std::map<std::string, T>& map)
    {
        chunk_header header = { 0, (uint32_t)type };
        
        binary_data map_data;
        for(auto kv : map)
        {
            map_data.write(kv.second);
            map_data.write((uint32_t)kv.first.size());
            map_data.write(kv.first.data(), kv.first.size());
            /*
            encode(map_data, kv.first.data(), kv.first.size());
            encode(map_data, kv.second);
            */
        }
        header.size = map_data.size();
        data.write(header);
        data.write(map_data.ptr(), map_data.size());
    }

    void add_chunk(uint32_t type, void* data, size_t size)
    {

    }

    bool save(const std::string& filename)
    {
        data.write(_header);
        write_map(TYPE_REGISTRY, _type_registry);
        write_map(CHUNK_REGISTRY, _chunk_registry);

        data.write(chunk_header{0, (uint32_t)END});

        std::ofstream file(filename, std::ios::binary | std::ios::ate);
        if(!file.is_open())
            return false;
        file.write((char*)data.ptr(), data.size());
        file.close();
        return true;
    }
private:
    std::map<rttr::type, encoder_fn_t> encoders;
    std::map<rttr::type, decoder_fn_t> decoders;

    header _header;
    std::map<std::string, uint32_t> _type_registry;
    std::map<std::string, uint64_t> _chunk_registry;
    std::vector<std::pair<chunk_header, binary_data>> chunks;

    binary_data data;
};

}

#endif