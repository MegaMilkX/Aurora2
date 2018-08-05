#include <iostream>
#include <string>

#include <windows.h>

#include "../../src/resource_compiler.h"
#include <fstream>

#include "../../../general/util.h"

#include <algorithm>

bool MakeResource(ResourceCompiler* compiler, const char* name, const char* type, const char* source_filename)
{
    if(std::string(type) != "Config")
    {
        return false;
    }

    compiler->SubmitCopy(
        source_filename,
        name
    );

    return true;
}