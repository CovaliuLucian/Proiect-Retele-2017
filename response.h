#include <string>

using namespace std;

class Response
{
  private:
    int code;
    string message;

  public:
    string getMessage()
    {
        return message;
    }
    void setCode(int c)
    {
        code = c;
    }
    void setMessage(string m)
    {
        message = m;
    }
};