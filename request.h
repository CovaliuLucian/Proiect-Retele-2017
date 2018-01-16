#include <string>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;

class Request
{
  private:
    string request;
    bool succes;

  public:
    Request(string r)
    {
        request = std::move(r);
        succes = true;
    }
    Request(const char *r)
    {
        request = string(r);
        succes = true;
    }
    Request()
    {
    }
    string getRequest()
    {
        return request;
    }

    int send(SSL* ssl)
    {
        int status;
        string test = getRequest();
        const char *serialized = test.c_str();
        int len = (int)getRequest().length();

        status = SSL_write(ssl, &len, sizeof len);
        if (status < 0)
        {
            // error
        }

        status = SSL_write(ssl, serialized, len);
        if (status < 0)
        {
            // error
        }
        return status;
    }
    void setStatus(bool s)
    {
        succes = s;
    }
    bool getStatus()
    {
        return succes;
    }
};