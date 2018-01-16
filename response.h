#include <string>
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
#include <iostream>

using namespace std;

class Response
{
  private:
    int code;
    string message;

  public:
    Response()
    {
        setCode(999);
    }
    Response(char *msg)
    {
        setCode(999);
        setMessage(string(msg));
    }
    string getMessage()
    {
        return message;
    }
    void setCode(int c)
    {
        if (code < 100 || code > 999)
            code = 999;
        code = c;
    }
    int getCode()
    {
        return code;
    }
    void setMessage(string m)
    {
        message = string(m);
    }
    int send(SSL* ssl)
    {
        char code[4];
        sprintf(code, "%i", getCode());
        string serializedString = string(code) + getMessage();
        const char *serialized = serializedString.c_str();
        size_t sizeRes = strlen(serialized);

        if (SSL_write(ssl, &sizeRes, sizeof(int)) < 0)
        {
            cerr << "Eroare la write() catre client.\n";
            return 0;
        }

        if (SSL_write(ssl, serialized, sizeRes) < 0)
        {
            cerr << "Eroare la write() catre client.\n";
            return 0;
        }
        return 1;
    }
};