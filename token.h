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
    int priority;

    Token(string com, int pr = 0)
    {
        command = com;
        priority = pr;
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