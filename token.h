#ifndef Tokenh
#define Tokenh

#include <string>

using namespace std;

class Token
{
  protected:
    string type;

  public:
    string command;

    Token(string com)
    {
        command = com;
        type = "Token";
    }
    Token()
    {
    }
    virtual string getType()
    {
        return type;
    }
};

#endif