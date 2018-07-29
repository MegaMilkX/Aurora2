#ifndef HTTPUTIL_H
#define HTTPUTIL_H

#include <string>
#include <vector>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

namespace Au{
namespace HTTP{
namespace util
{
    
inline std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> result;
    std::string s = str;
    
    size_t start = 0U;
    size_t end = s.find(delim);
    while(end != std::string::npos)
    {
        result.push_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);
        if(end == std::string::npos)
            result.push_back(s.substr(start, end));
    }
    
    return result;
}

inline std::string replace_char(const std::string& str, char from, char to)
{
    std::string result = str;
    for(unsigned i = 0; i < result.size(); ++i)
    {
        if(result[i] == from)
            result[i] = to;
    }
    return result;
}

inline std::string FileNameExtension(const std::string& fileName)
{
    size_t pos = fileName.find_last_of('.');
    return fileName.substr(pos + 1, std::string::npos);
}

inline std::string urlDecode(std::string &SRC)
{
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) 
    {
        if (int(SRC[i])==37) 
        {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } 
        else if (SRC[i]=='+')
        {
            ret+=' ';
        }
        else 
        {
            ret+=SRC[i];
        }
    }
    return (ret);
}

// trim from start
inline std::string &ltrim(std::string &s) 
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
inline std::string &rtrim(std::string &s) 
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
inline std::string &trim(std::string &s) 
{
    return ltrim(rtrim(s));
}

}
}
}

#endif
