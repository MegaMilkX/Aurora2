#include "glbuffer.h"

namespace Au{
namespace GFX{

GLBuffer GLBuffer::Create(unsigned int target, unsigned int usage_hint)
{
    GLBuffer gb;
    gb.target = target;
    gb.usage = usage_hint;
    glGenBuffers(1, &gb.buffer);
    return gb;
}

void GLBuffer::Destroy()
{
	glDeleteBuffers(1, &buffer);
}

bool GLBuffer::Data(void* data, size_t sz)
{
    glBindBuffer(target, buffer);
    glBufferData(target, sz, data, usage);
    size = sz;
    return true;
}

std::vector<char> GLBuffer::Data()
{
    std::vector<char> result(size);
    if(!Valid())
        return result;
    glBindBuffer(target, buffer);
    glGetBufferSubData(target, 0, size, &(result[0]));
    return result;
}

bool GLBuffer::Valid()
{
    return buffer != 0;
}

void GLBuffer::Bind()
{
    glBindBuffer(target, buffer);
}

}
}
