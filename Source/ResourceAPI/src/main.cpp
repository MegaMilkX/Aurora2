#include "resource_registry.h"

class Text
{
public:
    bool Build(Resource* r)
    {
        data = std::string((char*)r->Data(), (char*)r->Data() + r->DataSize());
        return true;
    }

    void Print()
    {
        LOG(data);
    }
private:
    std::string data;
};

int main()
{
    ResourceRegistry registry;
    registry.Init();

    if(!registry.Get("General")->Load())
    {
        LOG("Failed to load General resource");
    }
    LOG("General loaded");

    return 0;
}

#undef MINIZ_HEADER_FILE_ONLY
#include "../../lib/miniz.c"