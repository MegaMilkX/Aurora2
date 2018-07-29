#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <map>
#include <iostream>
#include <stdlib.h>
#include "util.h"

namespace Au{
namespace HTTP{

class Request
{
public:    
    int FromString(const std::string& str)
    {
        std::string endStr = "\r\n\r\n";
        size_t endPos = str.find(endStr);
        if(endPos == std::string::npos)
            return -1;
        
        std::vector<std::string> lines = util::split(str, "\r\n");
        
        if(lines.size() == 0)
            return -2;
        
        std::vector<std::string> requestLine = util::split(lines[0], " ");
        
        if(requestLine.size() != 3)
            return -3;
        method = requestLine[0];
        uri = requestLine[1];
        path = requestLine[1];
        
        std::string::size_type pos = uri.find('?');
        if(pos != std::string::npos)
        {
            std::vector<std::string> prts = util::split(uri, "?");
            path = prts[0];
            if(method == "GET")
                ParseParams(prts[1]);
        }
        
        http = requestLine[2];
        
        for(unsigned i = 1; i < lines.size(); ++i)
        {
            std::string delim = ": ";
            size_t pos = lines[i].find(delim);
            if(pos == std::string::npos)
                continue;
            std::string headerName = lines[i].substr(0, pos);
            std::string headerContent = lines[i].substr(pos + delim.size(), std::string::npos);
            headers.insert(std::make_pair(headerName, headerContent));
        }
        
        unsigned int contentLen = Header<unsigned int>("Content-Length");
        if(str.size() < endPos + endStr.size() + contentLen)
            return -4;
        std::string content = str.substr(endPos + endStr.size(), endPos + endStr.size() + contentLen);
        
        if(method == "POST")
        {
            std::string contentType = Header<std::string>("Content-Type");
            if(contentType.find("application/x-www-form-urlencoded") != std::string::npos)
            {
                ParseParams(content);
            }
            else if(contentType.find("multipart/form-data") != std::string::npos)
            {
                ParseMultipartFormData(content);
            }
        }
        //std::cout << content << std::endl;
        return endPos + endStr.size() + contentLen;
    }
    
    void Print() const
    {
        std::cout << "Method: " << method << std::endl;
        std::cout << "URI: " << uri << std::endl;
        std::cout << "VER: " << http << std::endl;
        
        std::map<std::string, std::string>::const_iterator it = headers.begin();
        for(it; it != headers.end(); ++it)
        {
            std::cout << it->first << " " << it->second << std::endl;
        }
        
        it = params.begin();
        for(it; it != params.end(); ++it)
        {
            std::cout << it->first << " " << it->second << std::endl;
        }
    }
    
    std::string operator[](const std::string& key) const
    {
        std::map<std::string, std::string>::const_iterator it = params.find(key);
        if(it == params.end())
            return "";
        else
            return it->second;
    }
    
    template<typename T>
    T Header(const std::string& header)
    {
        long int num = strtol(headers[header].c_str(), 0, 0);
        return num;
    }
    
    template<>
    std::string Header(const std::string& header)
    {
        return headers[header];
    }
    
    std::map<std::string, std::string> Params() { return params; }
    std::string Method() const { return method; }
    std::string URI() const { return uri; }
    std::string Path() const { return path; }
    
    std::vector<char> FileData(const std::string& name) const
    {
        if(files.find(name) == files.end())
            return std::vector<char>();
        return files.at(name); 
    }
    
    std::string FileName(const std::string& name) const
    {
        if(fileNames.find(name) == fileNames.end())
            return "";
        return fileNames.at(name);
    }
    
private:
    void ParseParams(const std::string& str)
    {
        std::vector<std::string> p = util::split(str, "&");
        if(p.empty())
            p.push_back(str);
        
        for(unsigned i = 0; i < p.size(); ++i)
        {
            std::vector<std::string> kv = util::split(p[i], "=");
            if(kv.size() < 2)
                params.insert(std::make_pair(p[i], ""));
            else
                params.insert(std::make_pair(kv[0], kv[1]));
        }
    }
    
    void ParseMultipartFormData(const std::string& data)
    {
        std::map<std::string, std::string> map = ParseHeaderData(Header<std::string>("Content-Type"));
        
        std::string boundary = map["boundary"];
        
        std::string lastBoundary = std::string("\r\n--") + boundary + "--";
        std::string dataStr = data.substr(0, data.find(lastBoundary));
        
        std::vector<std::string> parts = util::split(dataStr, boundary);
        
        for(unsigned i = 0; i < parts.size(); ++i)
        {
            ParseFormDataPart(parts[i]);
        }
    }
    
    void ParseFormDataPart(const std::string& part)
    {
        size_t payloadPos = part.find("\r\n\r\n");
        if(payloadPos == std::string::npos)
            return;
        payloadPos += 4;
        
        std::vector<std::string> headers = util::split(part.substr(0, payloadPos-4), "\r\n");
        for(unsigned i = 0; i < headers.size(); ++i)
        {
            std::vector<std::string> kv = util::split(headers[i], ": ");
            if(kv.size() < 2)
                continue;
            
            if(kv[0] == "Content-Disposition")
            {
                std::map<std::string, std::string> map = ParseHeaderData(kv[1]);
                std::string name = map["name"];
                name.erase(name.find_last_of("\""), 1);
                name.erase(name.find_first_of("\""), 1);
                std::string payload = part.substr(payloadPos, std::string::npos);
                files[name] = std::vector<char>(payload.begin(), payload.end());
                std::string filename = map["filename"];
                filename.erase(filename.find_last_of("\""), 1);
                filename.erase(filename.find_first_of("\""), 1);
                fileNames[name] = filename;
                //std::cout << payload << std::endl;
                //std::cout << name << ": " << payload;
            }
        }
    }
    
    std::map<std::string, std::string> ParseHeaderData(const std::string& data)
    {
        std::map<std::string, std::string> values;
        if(data.empty())
            return values;
        
        std::vector<std::string> parts = util::split(data, ";");
        
        for(unsigned i = 0; i < parts.size(); ++i)
        {
            parts[i] = util::trim(parts[i]);
            
            std::vector<std::string> commaParts = util::split(parts[i], ",");
            if(commaParts.empty())
                commaParts.push_back(parts[i]);
            
            for(unsigned j = 0; j < commaParts.size(); ++j)
            {
                std::vector<std::string> kv = util::split(commaParts[j], "=");
                if(kv.empty())
                {
                    values[commaParts[j]] = "";
                }
                else
                {
                    values[kv[0]] = kv[1];
                }
            }
            
        }
        
        return values;
    }

    std::string method;
    std::string uri;
    std::string path;
    std::string http;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;
    std::map<std::string, std::vector<char>> files;
    std::map<std::string, std::string> fileNames;
};

}
}

#endif
