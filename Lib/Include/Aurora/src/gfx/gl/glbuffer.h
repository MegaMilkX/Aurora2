#ifndef GLBUFFER_H
#define GLBUFFER_H

#include <vector>

#include "glextutil.h"

namespace Au{
namespace GFX{

class GLBuffer
{
public:
    GLBuffer() : size(0), buffer(0), target(0), usage(0) {}
    static GLBuffer Create(unsigned int target, unsigned int usage_hint);
	void Destroy();
    bool Data(void* data, size_t sz);
    std::vector<char> Data();
    bool Valid();
    void Bind();
private:
    size_t size;
    unsigned int buffer;
    unsigned int target;
    unsigned int usage;
};

}
}

#endif