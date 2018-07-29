#ifndef HTTPREQUESTHANDLER_H
#define HTTPREQUESTHANDLER_H

#include "httprequest.h"
#include "httpresponse.h"

namespace Au{
namespace HTTP{

class RequestHandler
{
public:
    virtual ~RequestHandler() {}
    virtual void operator()(const Request& request, Response& response) = 0;
};

}
}

#endif
