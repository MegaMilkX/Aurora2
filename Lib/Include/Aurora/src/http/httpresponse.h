#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <string>
#include <map>

#define ROK 0

namespace Au{
namespace HTTP{

class Response
{
public:
    Response()
    : version("HTTP/1.1"),
    status("200"),
    message("OK")
    {}
    
    operator std::string() const
    {
        std::string response;
        response += version + " ";
        response += status + " ";
        response += message + "\r\n";
        
        std::map<std::string, std::string>::const_iterator it = headers.begin();
        for(it; it != headers.end(); ++it)
        {
            response += it->first + ": ";
            response += it->second + "\r\n";
        }
        
        response += "\r\n";
        response += data;
        
        return response;
    }
    
    void Version(const std::string& str) { version = str; }
    void Status(const std::string& str) { status = str; }
    void Message(const std::string& str) { message = str; }
    void Header(const std::string& header, const std::string& value) { headers[header] = value; }
    void Data(const std::string& data)
    {
        this->data = data;
        Header("Content-Length", std::to_string(data.size()));
    }
private:
    std::string version;
    std::string status;
    std::string message;
    std::string data;
    std::map<std::string, std::string> headers;
};

}
}

#endif
