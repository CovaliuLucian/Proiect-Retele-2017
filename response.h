#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    int send(int sd)
    {
        char code[4];
        sprintf(code, "%i", getCode());
        string serializedString = string(code) + getMessage();
        const char *serialized = serializedString.c_str();
        size_t sizeRes = strlen(serialized);

        if (write(sd, &sizeRes, sizeof(int)) < 0)
        {
            perror("[server] Eroare la write() catre client.\n");
            return 0;
        }

        if (write(sd, serialized, sizeRes) < 0)
        {
            perror("[server] Eroare la write() catre client.\n");
            return 0;
        }
        return 1;
    }
};