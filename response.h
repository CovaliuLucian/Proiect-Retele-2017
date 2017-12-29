#include <string>

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
    Response(char* msg)
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
        if(code < 100 || code > 999)
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
};