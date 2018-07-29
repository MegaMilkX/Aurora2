#include "deflate.h"
#include <iostream>
namespace Au{

std::vector<char> Inflate(const std::vector<char>& data, size_t size_guess)
{    
    int err;
    std::vector<char> result(size_guess);
    uLongf dest_size = result.size();
    uLongf source_size = data.size();

    //std::cout << "OUT: "<< dest_size << std::endl;
    //std::cout << "IN: "<< source_size << std::endl;
 
    z_stream zstream;
    zstream.opaque = Z_NULL;
    zstream.zalloc = Z_NULL;
    zstream.zfree = Z_NULL;
    zstream.data_type = Z_BINARY;
    
    if(inflateInit(&zstream) != Z_OK)
    {
        std::cout << "inflateInit failed" << std::endl;
        return result;
    }
    
    zstream.next_in = (Bytef*)(data.data());
    zstream.avail_in = source_size;
    zstream.avail_out = dest_size;
    zstream.next_out = (Bytef*)(result.data());
    
    err = inflate(&zstream, Z_FINISH);
    
    inflateEnd(&zstream);
    
    if(err == Z_MEM_ERROR)
    {
        std::cout << "Z_MEM_ERROR" << std::endl;
    }
    
    if(err == Z_BUF_ERROR)
    {
        std::cout << "Z_BUF_ERROR" << std::endl;
    }
    
    if(err == Z_DATA_ERROR)
    {
        std::cout << "Z_DATA_ERROR" << std::endl;
    }
    
    result.resize(dest_size);
    return result;
}

std::vector<char> Deflate(const std::vector<char>& data)
{
    return data;
}

}