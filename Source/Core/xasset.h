#ifndef XASSET_H
#define XASSET_H

#include <string>
#include <util/double_buffer.h>

class asset_base
{
public:
    asset_base() 
    {}
    asset_base(const std::string& name)
    {

    }

    void load(const std::string& name)
    {

    }

    void finalize()
    {

    }
private:
};

template<typename T>
class xasset : public asset_base
{

};

#endif
