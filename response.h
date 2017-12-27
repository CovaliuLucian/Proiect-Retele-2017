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
        code = 0;
    }
    Response(char* msg)
    {
        code = 0;
        setMessage(string(msg));
    }
    string getMessage()
    {
        return message;
    }
    void setCode(int c)
    {
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