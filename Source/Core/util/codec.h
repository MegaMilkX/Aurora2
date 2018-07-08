#ifndef CODEC_H
#define CODEC_H

#include <functional>
#include <map>
#include <vector>
#include <rttr/type>
#include <util/static_run.h>

typedef std::map<rttr::type, int> type_alloc_map_t;
typedef std::function<void(void*, size_t)> decoder_fn_t;
typedef std::function<void(void*, std::vector<unsigned char>&)> encoder_fn_t;

enum CODEC_BASIC_TYPES
{
    CT_CHAR,
    CT_INT16,
    CT_INT32,
    CT_INT64,
    CT_UCHAR,
    CT_UINT16,
    CT_UINT32,
    CT_UINT64,
    CT_FLOAT,
    CT_DOUBLE,
    CT_BOOL,
    CT_STDSTRING
};

class Codec
{
public:
    template<typename T>
    static void Decoder(decoder_fn_t fn)
    {

    }

    template<typename T>
    static void Encoder(encoder_fn_t fn)
    {

    }

    static std::vector<unsigned char> ToBytes(rttr::type t, void* data);
    static void* FromBytes(rttr::type& t, void* data);
private:
    std::map<rttr::type, encoder_fn_t> _encoders;
    std::map<rttr::type, decoder_fn_t> _decoders;
};

template<typename T>
void put(std::vector<unsigned char>& buffer, const T& data)
{
    buffer.insert(buffer.back(), (unsigned char*)data, (unsigned char*)data + sizeof(T));
}

STATIC_RUN(CodecDefaultTypes){
    Codec::Encoder<int>([](void* data, std::vector<unsigned char>& buffer){
        uint64_t hash = std::hash<std::string>()("int");
        put(buffer, hash);
        put(buffer, *(int*)data);
    });
    Codec::Decoder<int>([](void* data, size_t sz)->bool{
        uint64_t hash = std::hash<std::string>()("int");
        if(sz < sizeof(hash))
            return false;
        


        return true;
    });
}

#endif
