#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include "ext/simpleini/simpleini.h"
#include "ext/json.hpp"

using json = nlohmann::json;

inline std::vector<std::string> get_from_json_array(json j)
{
    std::vector<std::string> r;
    if(!j.is_array())
    {
        std::cout << "json is not an array";
        return r;
    }
    for(unsigned i = 0; i < j.size(); ++i)
    {
        if(!j[i].is_string())
        {
            continue;
        }
        
        r.push_back(j[i].get<std::string>());
    }
    return r;
}

inline HMODULE this_module_handle()
{
  HMODULE h = NULL;
  GetModuleHandleExW(
    GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
    reinterpret_cast<LPCWSTR>(&this_module_handle),
    &h
  );
  return h;
}

std::string cut_dirpath(const std::string& path)
{
    return std::string(
        path.begin(), 
        path.begin() + path.find_last_of("\\")
    );
}

std::string get_module_dir()
{
    std::string filename;
    char buf[512];
    GetModuleFileNameA(this_module_handle(), buf, 512);
    filename = buf;
    filename = cut_dirpath(filename);
    return filename;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        std::cout << "no project path specified" << std::endl;
        return 0;
    }
    
    std::string project_filename = argv[1];
    
    
    CSimpleIniA project_ini;
    project_ini.SetUnicode();
    project_ini.LoadFile(project_filename.c_str());
    std::string project_name = "UntitledProject";
    std::string build_dir = "build";
    std::string source_dir = "src";
    project_name = project_ini.GetValue("", "project_name", "UntitledProject");
    build_dir = project_ini.GetValue("", "build_path", "build");
    source_dir = project_ini.GetValue("", "source_path", "");
    std::cout << "Building " << project_name << std::endl;
    
    std::cout << get_module_dir() << std::endl;
    
    std::ifstream json_file(get_module_dir() + "\\config.json");
    json j;
    try
    {
        json_file >> j;
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what();
        return 0;
    }
    if(!j.is_object())
    {
        std::cout << "config json is not object";
        return 0;
    }
    std::vector<std::string> include_paths = get_from_json_array(j["include_paths"]);
    std::vector<std::string> library_paths = get_from_json_array(j["library_paths"]);
    std::vector<std::string> libraries = get_from_json_array(j["libraries"]);
    
    std::string include_paths_s;
    std::string library_paths_s;
    std::string libraries_s;
    for(auto v : include_paths)
    {
        if(v.find_last_of(":") == v.npos)
        {
            v = get_module_dir() + "\\" + v;
        }
        include_paths_s += "/I \"" + v + "\" ";
    }
    for(auto v : library_paths)
    {
        if(v.find_last_of(":") == v.npos)
        {
            v = get_module_dir() + "\\" + v;
        }        
        library_paths_s += "/LIBPATH:\"" + v + "\" ";
    }
    for(auto v : libraries) 
        libraries_s += "\"" + v + "\" ";
    
    std::cout << include_paths_s << std::endl;
    std::cout << library_paths_s << std::endl;
    std::cout << libraries_s << std::endl;
    
    //std::cout << j.dump() << std::endl;
    
    std::string source_path = "\"" + get_module_dir() + "\\..\\..\\Source\\Core\\\" ";
    source_path += 
       "\"" + cut_dirpath(project_filename) + "\\" + source_dir + "\\\"";
    std::cout << source_path;
    _putenv_s("INCLUDE_PATHS", include_paths_s.c_str());
    _putenv_s("LIB_PATHS", library_paths_s.c_str());
    _putenv_s("LIBRARIES", libraries_s.c_str());
    
    _putenv_s("EXENAME", project_name.c_str());
    _putenv_s("OUTDIR", (cut_dirpath(project_filename) + "\\" + build_dir + "\\").c_str());
    _putenv_s("SOURCE_DIRS", source_path.c_str());
    
    if(system(("call \"" + get_module_dir() + "\\build\"").c_str()) != 0)
    {
        std::cout << "Failed to execute vcvarsall script" << std::endl;
    }
    
    std::getchar();
    return 0;
}