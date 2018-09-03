#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <shlwapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iterator>

#include "ext/json.hpp"
using json = nlohmann::json;

template<typename Out>
inline void split(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

inline std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

inline bool replace(std::string& str, const std::string& from, const std::string& to) 
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

inline void replace_all(std::string& str, const std::string& from, const std::string& to)
{
    while(replace(str, from, to));
}

inline std::string get_string(json& j, const std::string& def)
{
    if(!j.is_string())
    {
        return def;
    }
    return j.get<std::string>();
}

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

inline std::string cut_dirpath(const std::string& path)
{
    return std::string(
        path.begin(), 
        path.begin() + path.find_last_of("\\")
    );
}

inline std::string get_module_dir()
{
    std::string filename;
    char buf[512];
    GetModuleFileNameA(this_module_handle(), buf, 512);
    filename = buf;
    filename = cut_dirpath(filename);
    return filename;
}

inline bool copy_file(const std::string& from, const std::string& to)
{
    if(CopyFileA(
        from.c_str(),
        to.c_str(),
        false
    ) == FALSE)
    {
        return false;
    }
    return true;
}

inline void find_files(const std::string& dir, const std::string& filter, std::vector<std::string>& out) {
    std::string full_filter = dir + "\\*";
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(full_filter.c_str(), &data);

    char buf[260];
    DWORD len = GetFullPathNameA(full_filter.c_str(), 260, buf, 0);
    std::string dirpath(buf, len);

    if ( hFind != INVALID_HANDLE_VALUE ) 
    {
        do 
        {
            if(std::string(data.cFileName) == "." || 
                std::string(data.cFileName) == "..")
            {
                continue;
            }

            if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                find_files(dir + "\\" + std::string(data.cFileName), filter, out);
                continue;
            }

            if(PathMatchSpecA(data.cFileName, filter.c_str())) {
                out.push_back(dir + "\\" + std::string(data.cFileName));
            }
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    }
}

inline std::vector<std::string> find_all_files(const std::string& dir, const std::string& filter)
{
    std::vector<std::string> names;
    find_files(dir, filter, names);
    return names;
}

#define MKSTR(LINE) \
static_cast<std::ostringstream&>(std::ostringstream() << LINE).str()

#define LOG(LINE) std::cout << MKSTR(LINE) << std::endl;
#define LOG_WARN(LINE) LOG("WARNING: " << LINE)
#define LOG_ERR(LINE) LOG("ERROR: " << LINE)
#define LOG_DBG(LINE) LOG("DEBUG: " << LINE)

#endif
