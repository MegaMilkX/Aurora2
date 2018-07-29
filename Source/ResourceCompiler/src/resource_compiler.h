#ifndef RESOURCE_COMPILER_H
#define RESOURCE_COMPILER_H

class ResourceCompiler
{
public:
    virtual bool SubmitMem(const char* name, void* data, size_t len) = 0;
    virtual bool SubmitFile(const char* name, const char* filename) = 0;
};

extern "C"{
    __declspec(dllexport) bool __cdecl MakeResource(ResourceCompiler*, const char*, const char*, const char*);
}

#endif
