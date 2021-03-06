#include "HttpContext.h"
#include "Buffer.h"

namespace MuduoPlus
{

    bool HttpContext::processRequestLine(const char* begin, const char* end)
    {
        /*  POST /p/pcdn/i.php?v=35068810873 HTTP/1.0\r\n */
        bool succeed = false;
        const char* start = begin;
        const char* space = std::find(start, end, ' ');

        if(space != end && request_.setMethod(start, space))
        {
            start = space + 1;
            space = std::find(start, end, ' ');

            if(space != end)
            {
                const char* question = std::find(start, space, '?');

                if(question != space)
                {
                    request_.setPath(start, question);
                    request_.setQuery(question, space);
                }
                else
                {
                    request_.setPath(start, space);
                }

                start = space + 1;
                succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");

                if(succeed)
                {
                    if(*(end - 1) == '1')
                    {
                        request_.setVersion(HttpRequest::kHttp11);
                    }
                    else if(*(end - 1) == '0')
                    {
                        request_.setVersion(HttpRequest::kHttp10);
                    }
                    else
                    {
                        succeed = false;
                    }
                }
            }
        }

        return succeed;
    }

// return false if any error
    bool HttpContext::parseRequest(Buffer* buf, Timestamp receiveTime)
    {
        bool ok = true;
        bool hasMore = true;

        while(hasMore)
        {
            if(state_ == kExpectRequestLine)
            {
                const char* crlf = buf->findCRLF();

                if(crlf)
                {
                    ok = processRequestLine(buf->peek(), crlf);

                    if(ok)
                    {
                        request_.setReceiveTime(receiveTime);
                        buf->retrieveUntil(crlf + 2);
                        state_ = kExpectHeaders;
                    }
                    else
                    {
                        hasMore = false;
                    }
                }
                else
                {
                    hasMore = false;
                }
            }
            else if(state_ == kExpectHeaders)
            {
                /* Host:116.211.115.50\r\n
                 * User-Agent:ikuacc\r\n
                 */
                const char* crlf = buf->findCRLF();

                if(crlf)
                {
                    const char* colon = std::find(buf->peek(), crlf, ':');

                    if(colon != crlf)
                    {
                        request_.addHeader(buf->peek(), colon, crlf);
                    }
                    else
                    {
                        // empty line, end of header
                        // FIXME:
                        state_ = kGotAll;
                        hasMore = false;
                    }

                    buf->retrieveUntil(crlf + 2);
                }
                else
                {
                    hasMore = false;
                }
            }
            else if(state_ == kExpectBody)
            {
                // FIXME:
            }
        }

        return ok;
    }
}